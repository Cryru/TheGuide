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
#include ".\fgcolordialog.h"

// CFgColorDialog dialog

IMPLEMENT_DYNAMIC(CFgColorDialog, CDialog)

CFgColorDialog::CFgColorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFgColorDialog::IDD, pParent)
	, m_clrInitial(RGB(255,255,255))
	, m_clrFinal(RGB(255,255,255))
	, m_wParam(0)
{
}

CFgColorDialog::~CFgColorDialog()
{
}

void CFgColorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	for (int i=0; i<40; ++i)
		DDX_Control(pDX, IDC_FGCOL1+i, m_buttons[i]);
	DDX_Control(pDX, IDC_FGCOLOR_AUTO, m_buttonAuto);
	DDX_Control(pDX, IDC_FGCOLOR_MORE, m_buttonMore);
}

BEGIN_MESSAGE_MAP(CFgColorDialog, CDialog)
	ON_BN_CLICKED(IDC_FGCOLOR_AUTO, OnBnClickedFgcolorAuto)
	ON_BN_CLICKED(IDC_FGCOLOR_MORE, OnBnClickedFgcolorMore)
	ON_WM_ACTIVATE()
	ON_COMMAND_RANGE(IDC_FGCOL1, IDC_FGCOL40, OnFgColor)
END_MESSAGE_MAP()

static const COLORREF colors[] = {
	// row 1
    RGB(0x00, 0x00, 0x00), RGB(0xA5, 0x2A, 0x00), RGB(0x00, 0x40, 0x40), RGB(0x00, 0x55, 0x00), 
		RGB(0x00, 0x00, 0x5E), RGB(0x00, 0x00, 0x8B), RGB(0x4B, 0x00, 0x82), RGB(0x28, 0x28, 0x28), 
	// row 2
    RGB(0x8B, 0x00, 0x00), RGB(0xFF, 0x68, 0x20), RGB(0x8B, 0x8B, 0x00), RGB(0x00, 0x93, 0x00), 
		RGB(0x38, 0x8E, 0x8E), RGB(0x00, 0x00, 0xFF), RGB(0x7B, 0x7B, 0xC0), RGB(0x66, 0x66, 0x66), 
	// row 3
    RGB(0xFF, 0x00, 0x00), RGB(0xFF, 0xAD, 0x5B), RGB(0x32, 0xCD, 0x32), RGB(0x3C, 0xB3, 0x71), 
		RGB(0x7F, 0xFF, 0xD4), RGB(0x7D, 0x9E, 0xC0), RGB(0x80, 0x00, 0x80), RGB(0x80, 0x80, 0x80), 
	// row 4
    RGB(0xFF, 0xC0, 0xCB), RGB(0xFF, 0xD7, 0x00), RGB(0xFF, 0xFF, 0x00), RGB(0x00, 0xFF, 0x00), 
	    RGB(0x40, 0xE0, 0xD0), RGB(0xC0, 0xFF, 0xFF), RGB(0x48, 0x00, 0x48), RGB(0xC0, 0xC0, 0xC0), 
	// row 5
    RGB(0xFF, 0xE4, 0xE1), RGB(0xD2, 0xB4, 0x8C), RGB(0xFF, 0xFF, 0xE0), RGB(0x98, 0xFB, 0x98), 
		RGB(0xAF, 0xEE, 0xEE), RGB(0x68, 0x83, 0x8B), RGB(0xE6, 0xE6, 0xFA), RGB(0xFF, 0xFF, 0xFF), 
};

// CFgColorDialog message handlers

BOOL CFgColorDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_toolTip.Create(this);
	m_toolTip.SetDelayTime(0);

	CString t;
	for (int i=0; i<40; ++i)
	{
		t.LoadString(IDC_FGCOL1+i);
		m_buttons[i].SetColor(colors[i]);
		m_toolTip.AddTool(m_buttons+i, t);
	}

	m_toolTip.Activate(TRUE);

	return TRUE;
}

BOOL CFgColorDialog::PreTranslateMessage(MSG* pMsg)
{
	if (m_toolTip.m_hWnd)
		m_toolTip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

void CFgColorDialog::OnBnClickedFgcolorAuto()
{
	m_clrFinal = (DWORD)-1;
	OnSelectColor(m_clrFinal);
	OnOK();
}

void CFgColorDialog::OnBnClickedFgcolorMore()
{
	COLORREF result;

	if (GuideChooseColor(m_clrInitial, m_hWnd, result))
	{
		m_clrFinal = result;
		OnSelectColor(m_clrFinal);
	}

	theApp.GetRightView()->SetFocus();
}

void CFgColorDialog::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialog::OnActivate(nState, pWndOther, bMinimized);

	if (nState == WA_INACTIVE)
		ShowWindow(SW_HIDE);
}

void CFgColorDialog::OnFgColor(UINT nID)
{
	m_clrFinal = colors[nID-IDC_FGCOL1];
	OnSelectColor(m_clrFinal);
	OnOK();
}

void CFgColorDialog::SetInitialColor(COLORREF c)
{
	m_clrInitial = c;
	m_buttonAuto.SetChecked(m_clrInitial == (DWORD)-1);

	for (int i=0; i<40; ++i)
	{
		bool checked = (m_clrInitial != (DWORD)-1) && (m_clrInitial == colors[i]);
		m_buttons[i].SetChecked(checked);
	}

	m_clrFinal = m_clrInitial;
}
