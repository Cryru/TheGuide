/* Copyright 2007-08 Mahadevan R
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stdafx.h"
#include "Guide.h"
#include "SelectNodeDialog.h"
#include <libguide/guide.h>
#include ".\hyperlinkdialog.h"

// Helper function to parse a piece of RTF that holds the link and text
// for a hyperlink. The fragement is got from getting the text range
// corresponding to the selection.
static void parse(const CString& sel, CString& text, CString& url)
{
	int ofs = 0, pos, len = sel.GetLength();

	// Clear text and url. After we're through, the text will
	// contain all the text (even if we've selected multiple
	// links) and the URL will contain the first URL.
	text.Empty();
	url.Empty();

	// loop over 'sel', looking for HYPERLINKs.
	while (ofs < len && (pos = sel.Find(L"HYPERLINK \"", ofs)) >= 0)
	{
		// add to text, what is before the HYPERLINK.
		text += sel.Mid(ofs, pos-ofs);

		pos += 11;
		int pos2 = sel.Find(L'"', pos); // find the second (end) quote

		// we take only the first URL if there are multiple links
		// selected (i.e., the selection spans over link-nonlink-link
		// text)
		if (url.IsEmpty())
		{
			// the url is the portion between the quotes
			url = sel.Mid(pos, pos2-pos);
		}

		++pos2;		// skip over the closing "
		if (sel[pos2] == L' ')
			++pos2;	// and also over a space just after, if present
		ofs = pos2; // restart search from here
	}

	// add to text, what is after the final link.
	text += sel.Mid(ofs);

	// replace all control chars in text with space.
	for (int i=0; i<text.GetLength(); ++i)
		if (text[i] < 32)
			text.SetAt(i, 32);
}

// Callback function for SHBrowseForFolder, used below. It sets the
// initial dir, as well as disables the OK button for all non-FS 
// objects.
static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, 
	LPARAM pData) 
{
	switch(uMsg) 
	{
		case BFFM_INITIALIZED:
			// set initial dir
			if (pData)
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;

		case BFFM_SELCHANGED:
			// check if the selected dir actually exists (is not "Control Panel" etc)
			TCHAR szDir[MAX_PATH];
			BOOL dirExists;
			dirExists = SHGetPathFromIDList((LPITEMIDLIST) lp, szDir);
			// enable/disable the OK button accordingly
			SendMessage(hwnd, BFFM_ENABLEOK, 0, dirExists);
		break;
	}

	return 0;
}

// given "C:\path\to\file.txt", returns "C:\path\to".
static CString GetContainingPath(const CString& path)
{
	int pos = path.ReverseFind(L'\\');
	if (pos != -1)
		return path.Left(pos);
	return path;
}

// Helper function to browse for a folder. Initial dir
// can be passed. The final dir is also passed back
// in the same variable.
static bool BrowseForFolder(CString& path_inout)
{
	// resource strings:
	//	IDS_SELDIR	"Select directory to link to:"
	CString selDir = RCSTR(IDS_SELDIR);

	// prepare a browse info
    BROWSEINFO bi = { 0 };
	bi.lpszTitle = (LPCTSTR)selDir;
	bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS;
	bi.lpfn = BrowseCallbackProc;

	// set initial path
	CString szDir;
	DWORD attr = ::GetFileAttributes(path_inout);
	if (attr == INVALID_FILE_ATTRIBUTES)
	{
		// path may not exist. don't try to locate it.
		bi.lParam = NULL;
	}
	else if ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		// path is a file.
		szDir = GetContainingPath(path_inout);
		bi.lParam = (LPARAM)(LPCTSTR)szDir;
	}
	else
	{
		// path is a dir.
		bi.lParam = (LPARAM)(LPCTSTR)path_inout;
	}

	// actually browse
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl == 0)
		return false;

    // get the name of the folder
	TCHAR path[MAX_PATH] = {0};
    if (SHGetPathFromIDList(pidl, path))
        path_inout = path;

    // free memory used
	CoTaskMemFree(pidl);

	// return true if we filled the out parameter
	return path[0] != 0;
}


//-----------------------------------------------------------------------------
// CHyperLinkDialog implementation


IMPLEMENT_DYNAMIC(CHyperLinkDialog, CDialog)

CHyperLinkDialog::CHyperLinkDialog(const CString& sel, struct guide_t *guide,
	struct tree_node_t *node, CWnd* pParent)
	: CDialog(IDD_HYPERLINK, pParent)
	, m_pGuide(guide)
	, m_pNode(node)
	, m_nNodeUID(0)
	, m_cText(_T(""))
	, m_cLinkType(0)
	, m_bClearLink(false)
{
	CString link;
	parse(sel, m_cText, link);
	SetLink(link);
}

CHyperLinkDialog::~CHyperLinkDialog()
{
	// empty
}

void CHyperLinkDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TEXT, m_cText);
	DDX_Text(pDX, IDC_NODETEXT, m_cNode);
	DDX_Text(pDX, IDC_FILEDIRTEXT, m_cFileDir);
	DDX_Text(pDX, IDC_EMAILTEXT, m_cEmail);
	DDX_Text(pDX, IDC_URLTEXT, m_cURL);
	DDX_Radio(pDX, IDC_NODE, m_cLinkType);
}

BEGIN_MESSAGE_MAP(CHyperLinkDialog, CDialog)
	ON_BN_CLICKED(IDC_NODE, EnableAppropriately)
	ON_BN_CLICKED(IDC_FILEDIR, EnableAppropriately)
	ON_BN_CLICKED(IDC_EMAIL, EnableAppropriately)
	ON_BN_CLICKED(IDC_URL, EnableAppropriately)
	ON_BN_CLICKED(IDC_CLEARLINK, OnBnClickedClearlink)
	ON_BN_CLICKED(IDC_UPDATELINK, OnBnClickedUpdatelink)
	ON_BN_CLICKED(IDC_CHOOSEDIR, OnBnClickedChoosedir)
	ON_BN_CLICKED(IDC_CHOOSEFILE, OnBnClickedChoosefile)
	ON_BN_CLICKED(IDC_CHOOSENODE, OnBnClickedChoosenode)
END_MESSAGE_MAP()

BOOL CHyperLinkDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// update UI (DDX vars were set in ctor)
	UpdateData(FALSE);

	// enable/disable
	EnableAppropriately();

	return TRUE;
}

void CHyperLinkDialog::EnableAppropriately()
{
	UpdateData();

	static const UINT theControls[] = {
		IDC_NODETEXT, IDC_FILEDIRTEXT, IDC_EMAILTEXT, IDC_URLTEXT
	};

	static const int theMatrix[][4] = {
		{ 1, 0, 0, 0 },	// 0: node
		{ 0, 1, 0, 0 },	// 1: file/dir
		{ 0, 0, 1, 0 },	// 2: email
		{ 0, 0, 0, 1 },	// 3: url
	};

	for (int i=0; i<sizeof(theControls)/sizeof(*theControls); ++i)
	{
		UINT id = theControls[i];
		GetDlgItem(id)->EnableWindow(theMatrix[m_cLinkType][i]);
	}
}

// sets the DDX vars according to the given link
void CHyperLinkDialog::SetLink(const CString& link)
{
	LPCTSTR szLink = link;

	if (_tcsncmp(szLink, L"node://", 7) == 0)
	{
		m_cLinkType = 0;

		// find last '/'
		int pos = link.ReverseFind(L'/');
		ASSERT(pos != -1);
		if (pos == -1)
		{
			m_cLinkType = 3;
			m_cURL = _T("");
			return;
		}

		// take the number after that
		m_nNodeUID = (unsigned int)_wtoi(szLink + pos + 1);

		// get the node from the uid
		struct tree_node_t *node = guide_get_node_by_uid(m_pGuide, m_nNodeUID);

		// set the text
		struct guide_nodedata_t *data = (struct guide_nodedata_t *)
			tree_get_data(node);
		ASSERT(m_nNodeUID == data->uid);
		m_cNode = data->title;
	}
	else if (_tcsncmp(szLink, L"file:///", 8) == 0)
	{
		m_cLinkType = 1;
		m_cFileDir = szLink + 8;
	}
	else if (_tcsncmp(szLink, L"mailto:", 7) == 0)
	{
		m_cLinkType = 2;
		m_cEmail = szLink + 7;
	}
	else
	{
		m_cLinkType = 3;
		m_cURL = link;
	}
}

void CHyperLinkDialog::OnBnClickedClearlink()
{
	UpdateData();

	if (m_cText.IsEmpty())
	{
		AfxMessageBox(IDS_NOLINKTEXT, MB_ICONSTOP);
		GetDlgItem(IDC_TEXT)->SetFocus();
		return;
	}

	m_bClearLink = true; // indicate user wants to clear the link
	OnOK(); // close dialog
}

void CHyperLinkDialog::OnBnClickedUpdatelink()
{
	UpdateData();

	// check various error conditions

	if (m_cText.IsEmpty())
	{
		AfxMessageBox(IDS_NOLINKTEXT, MB_ICONSTOP);
		GetDlgItem(IDC_TEXT)->SetFocus();
		return;
	}
	if (m_cLinkType == 0 && m_nNodeUID == 0)
	{
		AfxMessageBox(IDS_NONODE, MB_ICONSTOP);
		return;
	}
	if (m_cLinkType == 1 && m_cFileDir.IsEmpty())
	{
		AfxMessageBox(IDS_NOFILEDIR, MB_ICONSTOP);
		return;
	}
	if (m_cLinkType == 2 && m_cEmail.IsEmpty())
	{
		AfxMessageBox(IDS_NOEMAIL, MB_ICONSTOP);
		GetDlgItem(IDC_EMAILTEXT)->SetFocus();
		return;
	}
	if (m_cLinkType == 3 && m_cURL.IsEmpty())
	{
		AfxMessageBox(IDS_NOURL, MB_ICONSTOP);
		GetDlgItem(IDC_URLTEXT)->SetFocus();
		return;
	}
	if (m_cLinkType == 2 && m_cEmail.Find(L'"') != -1)
	{
		AfxMessageBox(IDS_NODQINEMAIL, MB_ICONSTOP);
		GetDlgItem(IDC_EMAILTEXT)->SetFocus();
		return;
	}
	if (m_cLinkType == 3 && m_cURL.Find(L'"') != -1)
	{
		AfxMessageBox(IDS_NODQINURL, MB_ICONSTOP);
		GetDlgItem(IDC_URLTEXT)->SetFocus();
		return;
	}

	OnOK();
}

// Return a string, representing the link that the user chose
CString CHyperLinkDialog::GetLink()
{
	CString out;

	switch (m_cLinkType)
	{
	case 0: out.Format(L"node://./%u", m_nNodeUID);	break;
	case 1: out.Format(L"file:///%s", m_cFileDir);	break;
	case 2: out.Format(L"mailto:%s", m_cEmail);		break;
	case 3: out = m_cURL;							break;
	}

	// There appears to be a strange bug with the rich edit control, in that if
	// the hyperlink and the text are exacly the same, the hyperlink effect goes
	// off. As a workaround, add a space after the URL (this is ignore in our
	// EN_LINK handler).
	if (out == GetText())
		out += " ";

	return out;
}

// Helper function to programmatically select the required
// radio button. The button is identified by an integer 0-4.
void CHyperLinkDialog::SelectRadio(int idx)
{
	ASSERT(idx >= 0 && idx <= 3);

	UpdateData();
	m_cLinkType = idx;
	UpdateData(FALSE);
	EnableAppropriately();
}

void CHyperLinkDialog::OnBnClickedChoosedir()
{
	// select file/dir radio button
	SelectRadio(1);

	// show dir select dialog
	if (!BrowseForFolder(m_cFileDir))
		return;

	// update UI
	UpdateData(FALSE);
}

void CHyperLinkDialog::OnBnClickedChoosefile()
{
	// select file/dir radio button
	SelectRadio(1);

	// show file open dialog
	CFileDialog aDlg(TRUE, NULL, m_cFileDir, 
		OFN_HIDEREADONLY|OFN_DONTADDTORECENT|OFN_FILEMUSTEXIST|
			OFN_PATHMUSTEXIST|OFN_SHAREAWARE, RCSTR(IDS_ALLFILES_FILT));
	if (aDlg.DoModal() == IDCANCEL)
		return;

	// update UI
	m_cFileDir = aDlg.GetPathName();
	UpdateData(FALSE);
}

void CHyperLinkDialog::OnBnClickedChoosenode()
{
	// select node radio button
	SelectRadio(0);

	// if no UID is present currently, choose the
	// currently selected node's UID
	struct tree_node_t *node;
	if (m_nNodeUID == 0)
		node = m_pNode;
	else
		node = guide_get_node_by_uid(m_pGuide, m_nNodeUID);

	// show select node dialog
	SelectNodeDialog aNodeDlg(m_pGuide, node);
	if (aNodeDlg.DoModal() == IDCANCEL)
		return;

	// get the selected node, it's uid and title; and update the UI
	node = aNodeDlg.GetNode();
	struct guide_nodedata_t *data = (struct guide_nodedata_t *)
		tree_get_data(aNodeDlg.GetNode());
	m_nNodeUID = data->uid;
	m_cNode = data->title;
	UpdateData(FALSE);
}
