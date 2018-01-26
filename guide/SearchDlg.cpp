/* Copyright 2005-08 Mahadevan R
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
#include <libguide/wintree.h>
#include ".\searchdlg.h"

IMPLEMENT_DYNAMIC(CSearchDlg, CDialog)

BEGIN_MESSAGE_MAP(CSearchDlg, CDialog)
	ON_BN_CLICKED(IDC_SCLOSE, OnBnClickedSclose)
	ON_BN_CLICKED(IDC_SFIND, OnBnClickedSfind)
	ON_NOTIFY(NM_DBLCLK, IDC_SRESULT, OnDblclkResult)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
END_MESSAGE_MAP()

CSearchDlg::CSearchDlg(CWnd* pParent)
	: CDialog(IDD_SEARCH_DIALOG, pParent)
	, min_w(100), min_h(100)
	, listctrl_border_w(10), listctrl_border_h(10)
{
	// empty
}

CSearchDlg::~CSearchDlg()
{
	::DestroyWindow(m_hRichEditWnd);
	m_hRichEditWnd = NULL;
}

void CSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SRESULT, m_result);
}

BOOL CSearchDlg::OnInitDialog()
{
	// resource strings:
	//	IDS_SEARCHCOL1	"#"
	//	IDS_SEARCHCOL2	"Node Title"
	//	IDS_SEARCHCOL3	"Text Context"

	CDialog::OnInitDialog();

	// create the hidden rtf window
	m_hRichEditWnd = ::CreateWindow(RICHEDIT50W, L"SearchTemp", ES_MULTILINE,
					0, 0, 100, 100, NULL, NULL, NULL, NULL);
	ASSERT(m_hRichEditWnd);

	// setup the list control
	m_result.InsertColumn(0, RCSTR(IDS_SEARCHCOL1), LVCFMT_LEFT, 25);
	m_result.InsertColumn(1, RCSTR(IDS_SEARCHCOL2), LVCFMT_LEFT, 200);
	m_result.InsertColumn(2, RCSTR(IDS_SEARCHCOL3), LVCFMT_LEFT, 200);
	m_result.SetExtendedStyle(
		m_result.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	// take the initial w, h as the minimum w, h
	CRect r;
	GetWindowRect(&r);
	min_w = r.Width();
	min_h = r.Height();

	// remember the border sizes around the list control, for resizing later
	CRect r2;
	m_result.GetWindowRect(&r2);
	listctrl_border_w = r.Width() - r2.Width();
	listctrl_border_h = r.Height() - r2.Height();

	// create a size_grip
	CRect cr;
	GetClientRect(&cr);
	m_hSizeBox = CreateWindowEx(WS_EX_TRANSPARENT, _T("SCROLLBAR"), NULL, 
		WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP | SBS_SIZEBOXBOTTOMRIGHTALIGN,
		0, 0, cr.Width(), cr.Height(), m_hWnd, NULL, AfxGetInstanceHandle(), NULL);

	return TRUE;
}

// CSearchDlg message handlers

void CSearchDlg::OnBnClickedSclose()
{
	ShowWindow(SW_HIDE);
}

void CSearchDlg::OnOK()
{
	OnBnClickedSfind();
}

void CSearchDlg::OnCancel()
{
	ShowWindow(SW_HIDE);
}

// callback function for wintree_traverse, which search one
// single node.
int CSearchDlg::SearchCallback(HTREEITEM item, 
	struct tree_node_t *node, void *cargo)
{
	CSearchDlg *that = (CSearchDlg *)cargo;

	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);

	// check in title
	CString theTitle(data->title);
	if (that->m_bMatchCase == false)
		theTitle.MakeLower();
	if (theTitle.Find(that->m_szText) != -1)
	{
		// found in the title!
		int nCount = that->m_result.GetItemCount();
		TCHAR slNo[33];
		_itow(nCount+1, slNo, 10);
		that->m_result.InsertItem(nCount, slNo);
		that->m_result.SetItemText(nCount, 1, data->title);
		that->m_result.SetItemText(nCount, 2, RCSTR(IDS_SEARCH_INTITLE));

		ResultInfo ri;
		ri.item = item;
		ri.range.cpMin = ri.range.cpMax = 0;
		that->m_arrResultInfo.Add(ri);
	}

	// are we checking only in the titles? then our job is done
	if (that->m_bOnlyTitle)
		return 0;

	// ok, check in the text too
	SetRTF(that->m_hRichEditWnd, data->text, false);

	// setup the range to search for
	FINDTEXTEX ft;
	ft.chrg.cpMin = 0;
	ft.chrg.cpMax = -1;
	ft.lpstrText = that->m_szText;
	ft.chrgText.cpMin = 0;
	ft.chrgText.cpMax = -1;

	// setup the search options
	WPARAM opts = FR_DOWN;
	if (that->m_bMatchCase)
		opts |= FR_MATCHCASE;

	// search!
	int ret;
	while (((ret = (int) ::SendMessage(that->m_hRichEditWnd, EM_FINDTEXTEX, opts, (LPARAM)&ft)))!= -1)
	{
		// found!
		int nCount = that->m_result.GetItemCount();

		// insert # and node title
		TCHAR slNo[33];
		_itow(nCount+1, slNo, 10);
		that->m_result.InsertItem(nCount, slNo);
		that->m_result.SetItemText(nCount, 1, data->title);

		// add a result info
		ResultInfo ri;
		ri.item = item;
		ri.range = ft.chrgText;
		that->m_arrResultInfo.Add(ri);

		// make up some context info
		TEXTRANGE tr;
		tr.chrg = ft.chrgText;
		tr.chrg.cpMin = __max(0, tr.chrg.cpMin-10);
		tr.chrg.cpMax += 10;
		int nChars = tr.chrg.cpMax - tr.chrg.cpMin + 1;
		if (nChars > 30) // too long for our taste
		{
			nChars = 30;
			tr.chrg.cpMax = nChars + tr.chrg.cpMin - 1;
		}

		// try to extract context
		CString context;
		if (nChars > 0)
		{
			TCHAR rangeText[32] = { 0 }; // 32! (<30!!)
			tr.lpstrText = rangeText;

			::SendMessage(that->m_hRichEditWnd, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

			if (tr.chrg.cpMin == 0)
				// avoid leading ... if we matched beginning of text
				context.Format(_T("%s..."), tr.lpstrText);
			else
				context.Format(_T("...%s..."), tr.lpstrText);
			context.Replace(_T("\r"), _T(""));
		}

		// and set it..
		that->m_result.SetItemText(nCount, 2, context);

		// move to next one
		ft.chrg.cpMin = ft.chrgText.cpMax;
		ft.chrg.cpMax = -1;
	}

	// '0' continues the search
	return 0;
}

void CSearchDlg::OnBnClickedSfind()
{
	// ask the views to commit the currently entered rtf in the text pane
	theApp.GetLeftView()->SendMessage(WM_GUIDEUI, GM_COMMIT_RTF_NOW);

	// clear results
	m_result.DeleteAllItems();
	m_arrResultInfo.RemoveAll();

	// save status of check boxes (prevents repeated reading in c/b)
	m_bMatchCase = IsDlgButtonChecked(IDC_SMATCHCASE)  == BST_CHECKED;
	m_bOnlyTitle = IsDlgButtonChecked(IDC_SONLYTITLES) == BST_CHECKED;

	// save text from edit box
	GetDlgItem(IDC_SWHAT)->GetWindowText(m_szText);
	if (m_bMatchCase == false)
		m_szText.MakeLower();

	// get the wintree and item
	HWND leftWnd = theApp.GetLeftWnd();
	wintree_t *pWinTree = (wintree_t *)::SendMessage(leftWnd, WM_GUIDEUI, GM_GET_WINTREE, 0);
	HTREEITEM item = TreeView_GetSelection(leftWnd);

	// no items in tree?
	if (item == NULL)
	{
		AfxMessageBox(IDS_SEARCH_NOPAGES, MB_ICONEXCLAMATION);
		return;
	}

	// no characters in the string?
	if (m_szText.IsEmpty())
	{
		AfxMessageBox(IDS_SEARCH_NOSTRING, MB_ICONEXCLAMATION);
		return;
	}

	// search subtree..
	if (IsDlgButtonChecked(IDC_SONLYSUBTREE) == BST_CHECKED)
		wintree_traverse_subtree(pWinTree, item, SearchCallback, this);
	// ..or entire tree
	else
		wintree_traverse(pWinTree, SearchCallback, this);
}

void CSearchDlg::OnDblclkResult(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMITEMACTIVATE *pAct = (NMITEMACTIVATE *)pNMHDR;

	// get item number
	int iItem = pAct->iItem;
	if (iItem != -1)
	{
		// get corresponding result info
		const ResultInfo& ri = m_arrResultInfo[iItem];

		// select the node in the tree
		HTREEITEM hTreeItem = ri.item;
		HWND leftWnd = theApp.GetLeftWnd();
		TreeView_SelectItem(leftWnd, hTreeItem);

		// select the text in the rich text view
		if (ri.range.cpMin != 0 || ri.range.cpMax != 0)
		{
			HWND rightWnd = theApp.GetRightWnd();
			::SendMessage(rightWnd, EM_SETSEL, ri.range.cpMin, ri.range.cpMax);
		}
	}

	*pResult = 0;
}

void CSearchDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// enforce min. width and height
	lpMMI->ptMinTrackSize.x = min_w;
	lpMMI->ptMinTrackSize.y = min_h;

	CDialog::OnGetMinMaxInfo(lpMMI);
}

void CSearchDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// don't interfere during WM_CREATE
	if (!m_result.GetSafeHwnd())
		return;

	// get dialog rect
	CRect r;
	GetWindowRect(r);

	// get list control rect (in client co-ords)
	CRect r2;
	m_result.GetWindowRect(r2);
	ScreenToClient(r2);
	m_result.MoveWindow(r2.left, r2.top, r.Width()-listctrl_border_w,
		r.Height()-listctrl_border_h);

	// move the size grip
	CRect sbRect;
	::GetWindowRect(m_hSizeBox, &sbRect);
	CRect cliRect;
	GetClientRect(&cliRect);
	::MoveWindow(m_hSizeBox, cliRect.right-sbRect.Width(), cliRect.bottom-sbRect.Height(),
		sbRect.Width(), sbRect.Height(), TRUE);
}

BOOL CSearchDlg::PreTranslateMessage(MSG* pMsg)
{
	// if Ctrl+A is pressed, select all text in the edit control
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == 'a' || pMsg->wParam == 'A') && 
		(GetKeyState(VK_CONTROL) & 0x8000))
	{
		GetDlgItem(IDC_SWHAT)->SendMessage(EM_SETSEL, 0, -1);
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
