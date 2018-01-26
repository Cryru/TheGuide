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
#include "Utils.h"
#include ".\printpref.h"

// -- helper methods ----------------------------------------------------------

static HANDLE loadIcon(UINT id)
{
	return ::LoadImage(AfxGetInstanceHandle(), 
		MAKEINTRESOURCE(id), IMAGE_ICON, 16, 16, 0);
}

// -- CPrintPref --------------------------------------------------------------

IMPLEMENT_DYNAMIC(CPrintPref, CPropertyPage)

CPrintPref::CPrintPref()
	: CPropertyPage(IDD_PREF_PRINT)
	, m_hsListBox(m_Template)
{
	// Load icons
	m_hIconLeft   = loadIcon(IDI_JFY_LEFT);
	m_hIconCenter = loadIcon(IDI_JFY_CENTER);
	m_hIconRight  = loadIcon(IDI_JFY_RIGHT);
	m_hIconBold   = loadIcon(IDI_FMT_BOLD);
	m_hIconItalic = loadIcon(IDI_FMT_ITALIC);
	m_hIconUnderline = loadIcon(IDI_FMT_UNDERLINE);

	// Load template
	theApp.GetPrintTemplate(m_Template);
}

CPrintPref::~CPrintPref()
{
	// Unload icons
	::DestroyIcon((HICON)m_hIconLeft);		m_hIconLeft = 0;
	::DestroyIcon((HICON)m_hIconCenter);	m_hIconCenter = 0;
	::DestroyIcon((HICON)m_hIconRight);		m_hIconRight = 0;
	::DestroyIcon((HICON)m_hIconBold);		m_hIconBold = 0;
	::DestroyIcon((HICON)m_hIconItalic);	m_hIconItalic = 0;
	::DestroyIcon((HICON)m_hIconUnderline);	m_hIconUnderline = 0;
}

void CPrintPref::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HDGLIST, m_hsListBox);
	DDX_Control(pDX, IDC_PFONTNAMES, m_fontNames);
	DDX_Control(pDX, IDC_PFONTSIZES, m_fontSizes);
}

BEGIN_MESSAGE_MAP(CPrintPref, CPropertyPage)
	ON_BN_CLICKED(IDC_PFXNUM, OnBnClickedPfxnum)
	ON_BN_CLICKED(IDC_BEGINONNEW, OnBnClickedBeginonnew)
	ON_BN_CLICKED(IDC_JUSLEFT, OnBnClickedJusleft)
	ON_BN_CLICKED(IDC_JUSCENTER, OnBnClickedJuscenter)
	ON_BN_CLICKED(IDC_JUSRIGHT, OnBnClickedJusright)
	ON_BN_CLICKED(IDC_HCOLOR, OnBnClickedHcolor)
	ON_BN_CLICKED(IDC_RESETALL, OnBnClickedResetall)
	ON_LBN_SELCHANGE(IDC_HDGLIST, OnLbnSelchangeHdglist)
	ON_CBN_SELCHANGE(IDC_PFONTNAMES, OnCbnSelchangePfontnames)
	ON_CBN_SELCHANGE(IDC_PFONTSIZES, OnCbnSelchangePfontsizes)
	ON_CBN_EDITCHANGE(IDC_PFONTSIZES, OnCbnSelchangePfontsizes)
	ON_CBN_KILLFOCUS(IDC_PFONTSIZES, OnCbnSelchangePfontsizes)
	ON_BN_CLICKED(IDC_PFMTBOLD, OnBnClickedPfmtbold)
	ON_BN_CLICKED(IDC_PFMTITALIC, OnBnClickedPfmtitalic)
	ON_BN_CLICKED(IDC_PFMTUNDERLINE, OnBnClickedPfmtunderline)
END_MESSAGE_MAP()

// CPrintPref message handlers

static UINT controlIDs[] = { 
	IDC_PFXNUM, IDC_BEGINONNEW, IDC_JUSLEFT, IDC_JUSCENTER, IDC_JUSRIGHT,
	IDC_HCOLOR, IDC_PFONTNAMES, IDC_PFONTSIZES, IDC_PFMTBOLD, IDC_PFMTITALIC,
	IDC_PFMTUNDERLINE
};

BOOL CPrintPref::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// set icons on the justify buttons
	GetDlgItem(IDC_JUSLEFT)->SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hIconLeft);
	GetDlgItem(IDC_JUSCENTER)->SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hIconCenter);
	GetDlgItem(IDC_JUSRIGHT)->SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hIconRight);
	GetDlgItem(IDC_PFMTBOLD)->SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hIconBold);
	GetDlgItem(IDC_PFMTITALIC)->SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hIconItalic);
	GetDlgItem(IDC_PFMTUNDERLINE)->SendMessage(BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hIconUnderline);

	// disable the style edit buttons (enabled on first selection)
	for (int i=0; i<sizeof(controlIDs)/sizeof(*controlIDs); ++i)
		GetDlgItem(controlIDs[i])->EnableWindow(FALSE);

	// insert as many entries as there are headings, text is dummy
	m_hsListBox.AddString(L"1");
	m_hsListBox.AddString(L"2");
	m_hsListBox.AddString(L"3");
	m_hsListBox.AddString(L"4");
	m_hsListBox.AddString(L"5");

	// fill font names and sizes
	FillFontNames(m_fontNames);
	FillFontSizes(m_fontSizes);

	return TRUE;
}

BOOL CPrintPref::OnApply()
{
	theApp.SetPrintTemplate(m_Template);

	return CPropertyPage::OnApply();
}

void CPrintPref::OnLbnSelchangeHdglist()
{
	// TODO: do this only once
	for (int i=0; i<sizeof(controlIDs)/sizeof(*controlIDs); ++i)
		GetDlgItem(controlIDs[i])->EnableWindow();

	// get selected template
	int idx = m_hsListBox.GetCurSel();
	const HeadingStyle& hs = m_Template.headings[idx];

	// set controls accordingly
	SetRadioCheck(IDC_PFXNUM, hs.numbered);
	SetRadioCheck(IDC_BEGINONNEW, hs.breakPage);
	SetRadioCheck(IDC_JUSLEFT, hs.justify == HeadingStyle::JFY_LEFT);
	SetRadioCheck(IDC_JUSCENTER, hs.justify == HeadingStyle::JFY_CENTER);
	SetRadioCheck(IDC_JUSRIGHT, hs.justify == HeadingStyle::JFY_RIGHT);
	SetRadioCheck(IDC_PFMTBOLD, hs.fontBold);
	SetRadioCheck(IDC_PFMTITALIC, hs.fontItalic);
	SetRadioCheck(IDC_PFMTUNDERLINE, hs.fontUnderline);
	m_fontNames.SelectString(-1, hs.fontFamily);

	TCHAR fontSize[32];
	_itot(hs.fontPointSize, fontSize, 10);
	if (m_fontSizes.SelectString(-1, fontSize) == CB_ERR)
	{
		m_fontSizes.SetCurSel(-1);
		m_fontSizes.SetWindowText(fontSize);
	}
}

void CPrintPref::OnBnClickedResetall()
{
	m_Template = PrintTemplate();
	m_hsListBox.RedrawAll();
	SetModified();
}

#define _GET_SELECTION(idx, hs)	\
	int idx = m_hsListBox.GetCurSel();				\
	ASSERT(idx >= 0);								\
	if (idx < 0) return;							\
	HeadingStyle& hs = m_Template.headings[idx];

void CPrintPref::OnBnClickedPfxnum()
{
	_GET_SELECTION(idx, hs);

	hs.numbered = !hs.numbered;
	m_hsListBox.RedrawItem(idx);
	SetModified();
}

void CPrintPref::OnBnClickedBeginonnew()
{
	_GET_SELECTION(idx, hs);

	hs.breakPage = !hs.breakPage;

	m_hsListBox.RedrawItem(idx);
	SetModified();
}

void CPrintPref::OnBnClickedJusleft()
{
	_GET_SELECTION(idx, hs);

	hs.justify = HeadingStyle::JFY_LEFT;

	m_hsListBox.RedrawItem(idx);
	SetModified();
}

void CPrintPref::OnBnClickedJuscenter()
{
	_GET_SELECTION(idx, hs);

	hs.justify = HeadingStyle::JFY_CENTER;

	m_hsListBox.RedrawItem(idx);
	SetModified();
}

void CPrintPref::OnBnClickedJusright()
{
	_GET_SELECTION(idx, hs);

	hs.justify = HeadingStyle::JFY_RIGHT;

	m_hsListBox.RedrawItem(idx);
	SetModified();
}

void CPrintPref::OnBnClickedHcolor()
{
	_GET_SELECTION(idx, hs);

	CColorDialog aDlg(hs.fontColor, CC_ANYCOLOR|CC_RGBINIT|CC_FULLOPEN);
	if (aDlg.DoModal() == IDOK)
	{
		hs.fontColor = aDlg.GetColor();

		// update display
		m_hsListBox.RedrawItem(idx, true);
		SetModified();
	}
}

void CPrintPref::OnCbnSelchangePfontnames()
{
	_GET_SELECTION(idx, hs);

	// get the selected font name
	CString fontName;
	m_fontNames.GetLBText(m_fontNames.GetCurSel(), fontName);
	wcscpy(hs.fontFamily, fontName);

	// update display
	m_hsListBox.RedrawItem(idx, true);
	SetModified();
}

void CPrintPref::OnCbnSelchangePfontsizes()
{
	_GET_SELECTION(idx, hs);

	// get the selected font size
	CString fontSize;
	int cbIdx = m_fontSizes.GetCurSel();
	if (cbIdx != CB_ERR)
		m_fontSizes.GetLBText(cbIdx, fontSize);
	else
		m_fontSizes.GetWindowText(fontSize);
	int pointSize = _ttoi(fontSize);

	if (pointSize > 0 && pointSize < 99)
	{
		hs.fontPointSize = _ttoi(fontSize);

		// update display
		m_hsListBox.RedrawItem(idx, true);
		SetModified();
	}
}

void CPrintPref::OnBnClickedPfmtbold()
{
	_GET_SELECTION(idx, hs);

	hs.fontBold = !hs.fontBold;

	// update display
	m_hsListBox.RedrawItem(idx, true);
	SetModified();
}

void CPrintPref::OnBnClickedPfmtitalic()
{
	_GET_SELECTION(idx, hs);

	hs.fontItalic = !hs.fontItalic;

	// update display
	m_hsListBox.RedrawItem(idx, true);
	SetModified();
}

void CPrintPref::OnBnClickedPfmtunderline()
{
	_GET_SELECTION(idx, hs);

	hs.fontUnderline = !hs.fontUnderline;

	// update display
	m_hsListBox.RedrawItem(idx, true);
	SetModified();
}
