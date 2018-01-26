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
#include ".\colorpref.h"

// CColorPref dialog

IMPLEMENT_DYNAMIC(CColorPref, CPropertyPage)

BEGIN_MESSAGE_MAP(CColorPref, CPropertyPage)
	ON_BN_CLICKED(IDC_RADIO_TEXT_COLOR_SYSDEFAULT, OnBnClickedRadioTextColorSysdefault)
	ON_BN_CLICKED(IDC_RADIO_TEXT_COLOR_USER, OnBnClickedRadioTextColorUser)
	ON_BN_CLICKED(IDC_RADIO_TREE_COLOR_SYSDEFAULT, OnBnClickedRadioTreeColorSysdefault)
	ON_BN_CLICKED(IDC_RADIO_TREE_COLOR_USER, OnBnClickedRadioTreeColorUser)
	ON_BN_CLICKED(IDC_TEXT_BACK, OnBnClickedTextBack)
	ON_BN_CLICKED(IDC_TEXT_FORE, OnBnClickedTextFore)
	ON_BN_CLICKED(IDC_TREE_BACK, OnBnClickedTreeBack)
	ON_BN_CLICKED(IDC_TREE_FORE, OnBnClickedTreeFore)
END_MESSAGE_MAP()

CColorPref::CColorPref()
	: CPropertyPage(CColorPref::IDD)
	, m_nTreeRadio(0)
	, m_nTextRadio(0)
{
	// empty
}

CColorPref::~CColorPref()
{
	// empty
}

void CColorPref::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_TREE_COLOR_SYSDEFAULT, m_nTreeRadio);
	DDX_Radio(pDX, IDC_RADIO_TEXT_COLOR_SYSDEFAULT, m_nTextRadio);
	DDX_Control(pDX, IDC_TREE_COLOR_SAMPLE, m_oTreeSample);
	DDX_Control(pDX, IDC_TEXT_COLOR_SAMPLE, m_oTextSample);
}

BOOL CColorPref::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	bool treeSysDefault = false;
	COLORREF treeFore, treeBack;
	theApp.GetTreeColor(treeSysDefault, treeFore, treeBack);
	m_nTreeRadio = treeSysDefault ? 0 : 1;
	m_oTreeSample.SetForeground(treeFore);
	m_oTreeSample.SetBackground(treeBack);

	bool textSysDefault = false;
	COLORREF textFore, textBack;
	theApp.GetTextColor(textSysDefault, textFore, textBack);
	m_nTextRadio = textSysDefault ? 0 : 1;
	m_oTextSample.SetForeground(textFore);
	m_oTextSample.SetBackground(textBack);

	UpdateData(FALSE);

	return TRUE;
}

BOOL CColorPref::OnApply()
{
	if (!UpdateData())
		return FALSE;

	theApp.SetTreeColor(
		m_nTreeRadio == 0,
		m_oTreeSample.GetForeground(),
		m_oTreeSample.GetBackground());

	theApp.SetTextColor(
		m_nTextRadio == 0,
		m_oTextSample.GetForeground(),
		m_oTextSample.GetBackground());

	// notify the views
	theApp.GetLeftView()->SendMessage(WM_GUIDEUI, GM_APPLY_COLOR);
	theApp.GetRightView()->SendMessage(WM_GUIDEUI, GM_APPLY_COLOR);

	return CPropertyPage::OnApply();
}

void CColorPref::OnBnClickedRadioTextColorSysdefault()
{
	if (m_nTextRadio == 1)
		SetModified();
}

void CColorPref::OnBnClickedRadioTextColorUser()
{
	if (m_nTextRadio == 0)
		SetModified();
}

void CColorPref::OnBnClickedRadioTreeColorSysdefault()
{
	if (m_nTreeRadio == 1)
		SetModified();
}

void CColorPref::OnBnClickedRadioTreeColorUser()
{
	if (m_nTreeRadio == 0)
		SetModified();
}

#define SET_RADIO(id, val) \
	GetDlgItem((id))->SendMessage(BM_SETCHECK, (val), 0L);

void CColorPref::OnBnClickedTextFore()
{
	COLORREF result;
	if (GuideChooseColor(m_oTextSample.GetForeground(), m_hWnd, result))
	{
		m_oTextSample.SetForeground(result);
		SET_RADIO(IDC_RADIO_TEXT_COLOR_SYSDEFAULT,	0);
		SET_RADIO(IDC_RADIO_TEXT_COLOR_USER,		1);
		if (m_nTextRadio == 1)
			SetModified();
	}
}

void CColorPref::OnBnClickedTextBack()
{
	COLORREF result;
	if (GuideChooseColor(m_oTextSample.GetBackground(), m_hWnd, result))
	{
		m_oTextSample.SetBackground(result);
		SET_RADIO(IDC_RADIO_TEXT_COLOR_SYSDEFAULT,	0);
		SET_RADIO(IDC_RADIO_TEXT_COLOR_USER,		1);
		if (m_nTextRadio == 1)
			SetModified();
	}
}

void CColorPref::OnBnClickedTreeFore()
{
	COLORREF result;
	if (GuideChooseColor(m_oTreeSample.GetForeground(), m_hWnd, result))
	{
		m_oTreeSample.SetForeground(result);
		SET_RADIO(IDC_RADIO_TREE_COLOR_SYSDEFAULT,	0);
		SET_RADIO(IDC_RADIO_TREE_COLOR_USER,		1);
		if (m_nTreeRadio == 0)
			SetModified();
	}
}

void CColorPref::OnBnClickedTreeBack()
{
	COLORREF result;
	if (GuideChooseColor(m_oTreeSample.GetBackground(), m_hWnd, result))
	{
		m_oTreeSample.SetBackground(result);
		SET_RADIO(IDC_RADIO_TREE_COLOR_SYSDEFAULT,	0);
		SET_RADIO(IDC_RADIO_TREE_COLOR_USER,		1);
		if (m_nTreeRadio == 0)
			SetModified();
	}
}
