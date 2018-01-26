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
#include "utils.h"
#include ".\fontpref.h"

// CFontPref dialog

IMPLEMENT_DYNAMIC(CFontPref, CPropertyPage)

BEGIN_MESSAGE_MAP(CFontPref, CPropertyPage)
	ON_BN_CLICKED(IDC_TREE_FONT_CHOOSE, OnBnClickedTreeFontChoose)
	ON_BN_CLICKED(IDC_TEXT_FONT_CHOOSE, OnBnClickedTextFontChoose)
	ON_BN_CLICKED(IDC_RADIO_TREE_FONT_SYSDEFAULT, OnBnClickedRadioTreeFontSysdefault)
	ON_BN_CLICKED(IDC_RADIO_TREE_FONT_USER, OnBnClickedRadioTreeFontUser)
	ON_BN_CLICKED(IDC_RADIO_TEXT_FONT_SYSDEFAULT, OnBnClickedRadioTextFontSysdefault)
	ON_BN_CLICKED(IDC_RADIO_TEXT_FONT_USER, OnBnClickedRadioTextFontUser)
END_MESSAGE_MAP()

CFontPref::CFontPref()
	: CPropertyPage(CFontPref::IDD)
	, m_nTextRadio(0)
	, m_nTreeRadio(0)
{
	// empty
}

CFontPref::~CFontPref()
{
	// empty
}

void CFontPref::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_TREE_FONT_SYSDEFAULT, m_nTreeRadio);
	DDX_Radio(pDX, IDC_RADIO_TEXT_FONT_SYSDEFAULT, m_nTextRadio);
}

static CString formatFontName(const PreferredFont& font)
{
	// resource strings:
	//	IDS_FMTFONT1	"%s, %dpt"
	//	IDS_FMTFONT2	", bold+italic"
	//	IDS_FMTFONT3	", bold"
	//	IDS_FMTFONT4	", italic"

	CString f;
	f.Format(RCSTR(IDS_FMTFONT1), font.faceName, font.height);
	if (font.bold && font.italic)
		f += RCSTR(IDS_FMTFONT2);
	else if (font.bold)
		f += RCSTR(IDS_FMTFONT3);
	else if (font.italic)
		f += RCSTR(IDS_FMTFONT4);
	return f;
}

BOOL CFontPref::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// get and display tree pane font
	bool treeSysDefault;
	theApp.GetTreeFont(treeSysDefault, m_oTreeFont);
	GetDlgItem(IDC_RADIO_TREE_FONT_USER)->SetWindowText(
		formatFontName(m_oTreeFont));
	m_nTreeRadio = treeSysDefault ? 0 : 1;

	// get and display text pane font
	bool textSysDefault;
	theApp.GetTextFont(textSysDefault, m_oTextFont);
	GetDlgItem(IDC_RADIO_TEXT_FONT_USER)->SetWindowText(
		formatFontName(m_oTextFont));
	m_nTextRadio = textSysDefault ? 0 : 1;

	// actually show
	UpdateData(FALSE);

	return TRUE;
}

BOOL CFontPref::OnApply()
{
	if (!UpdateData())
		return FALSE;

	// set into rgy
	theApp.SetTreeFont(m_nTreeRadio == 0, m_oTreeFont);
	theApp.SetTextFont(m_nTextRadio == 0, m_oTextFont);

	// notify the views
	theApp.GetLeftView()->SendMessage(WM_GUIDEUI, GM_APPLY_FONT);
	theApp.GetRightView()->SendMessage(WM_GUIDEUI, GM_APPLY_FONT);

	return CPropertyPage::OnApply();
}

#define SET_RADIO(id, val) \
	GetDlgItem((id))->SendMessage(BM_SETCHECK, (val), 0L);

// WINBUG: Unicode font names will work only with CHARFORMAT2.
static void fillCHARFORMAT(const PreferredFont& font,
	CHARFORMAT2& cf)
{
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_FACE | CFM_SIZE;
	wcscpy(cf.szFaceName, font.faceName);
	cf.dwEffects =
		(font.bold ? CFE_BOLD : 0) |
		(font.italic ? CFE_ITALIC : 0);
	cf.yHeight = font.height * 20;
}

void CFontPref::OnBnClickedTreeFontChoose()
{
	CHARFORMAT2 cf;
	fillCHARFORMAT(m_oTreeFont, cf);

	CFontDialog aDlg(cf, CF_BOTH);
	if (aDlg.DoModal() == IDOK)
	{
		wcscpy(m_oTreeFont.faceName, aDlg.GetFaceName());
		m_oTreeFont.height = aDlg.GetSize() / 10;
		m_oTreeFont.bold = aDlg.IsBold() == TRUE;
		m_oTreeFont.italic = aDlg.IsItalic() == TRUE;

		GetDlgItem(IDC_RADIO_TREE_FONT_USER)->SetWindowText(
			formatFontName(m_oTreeFont));

		SET_RADIO(IDC_RADIO_TREE_FONT_SYSDEFAULT, 0);
		SET_RADIO(IDC_RADIO_TREE_FONT_USER,		  1);

		SetModified();
	}
}

void CFontPref::OnBnClickedTextFontChoose()
{
	CHARFORMAT2 cf;
	fillCHARFORMAT(m_oTextFont, cf);

	CFontDialog aDlg(cf, CF_BOTH);
	if (aDlg.DoModal() == IDOK)
	{
		wcscpy(m_oTextFont.faceName, aDlg.GetFaceName());
		m_oTextFont.height = aDlg.GetSize() / 10;
		m_oTextFont.bold = aDlg.IsBold() == TRUE;
		m_oTextFont.italic = aDlg.IsItalic() == TRUE;

		GetDlgItem(IDC_RADIO_TEXT_FONT_USER)->SetWindowText(
			formatFontName(m_oTextFont));

		SET_RADIO(IDC_RADIO_TEXT_FONT_SYSDEFAULT, 0);
		SET_RADIO(IDC_RADIO_TEXT_FONT_USER,		  1);

		SetModified();
	}
}

void CFontPref::OnBnClickedRadioTreeFontSysdefault()
{
	if (m_nTreeRadio == 1)
		SetModified();
}

void CFontPref::OnBnClickedRadioTreeFontUser()
{
	if (m_nTreeRadio == 0)
		SetModified();
}

void CFontPref::OnBnClickedRadioTextFontSysdefault()
{
	if (m_nTextRadio == 1)
		SetModified();
}

void CFontPref::OnBnClickedRadioTextFontUser()
{
	if (m_nTextRadio == 0)
		SetModified();
}
