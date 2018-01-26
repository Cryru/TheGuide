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
#include "FormatBar.h"
#include "FormatBarDefs.h"

BEGIN_MESSAGE_MAP(CFormatBar, CToolBar)
	ON_WM_CREATE()
	ON_CBN_SELENDOK(IDC_FONTNAME, OnSelectFontName)
	ON_CBN_SELENDOK(IDC_FONTSIZE, OnSelectFontSize)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CFormatBar, CToolBar)

CFormatBar::CFormatBar()
{
}

CFormatBar::~CFormatBar()
{
}

void CFormatBar::PositionCombos()
{
	CRect rect;
	m_comboFontName.GetWindowRect(&rect);
	int nHeight = rect.Height();

	// FontName ComboBox
	m_comboFontName.GetWindowRect(&rect);
	SetButtonInfo(BUTTON_FONTNAME, IDC_FONTNAME, TBBS_SEPARATOR, rect.Width());
	GetItemRect(BUTTON_FONTNAME, &rect);
	m_comboFontName.SetWindowPos(NULL, rect.left, rect.top, 0, 0,
		SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

	// FontSize ComboBox
	m_comboFontSize.GetWindowRect(&rect);
	SetButtonInfo(BUTTON_FONTSIZE, IDC_FONTSIZE, TBBS_SEPARATOR, rect.Width());
	GetItemRect(BUTTON_FONTSIZE, &rect);
	m_comboFontSize.SetWindowPos(NULL, rect.left, rect.top, 0, 0,
		SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
}

int CFormatBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;
//	CRect rect(0,0, (3*LF_FACESIZE*m_szBaseUnits.cx)/2, 200);
	CRect rect(0, 0, 180, 150);
	if (!m_comboFontName.Create(WS_TABSTOP|WS_VISIBLE|WS_TABSTOP|
		WS_VSCROLL|CBS_DROPDOWNLIST|CBS_SORT|CBS_AUTOHSCROLL|CBS_HASSTRINGS,
		rect, this, IDC_FONTNAME))
	{
		return -1;
	}
	m_comboFontName.LimitText(LF_FACESIZE);

	rect.SetRect(0, 0, 40, 150);
	if (!m_comboFontSize.Create(WS_TABSTOP|WS_VISIBLE|WS_TABSTOP|
		WS_VSCROLL|CBS_DROPDOWN, rect, this, IDC_FONTSIZE))
	{
		return -1;
	}

	LOGFONT logFont;
	memset(&logFont, 0, sizeof(logFont));
	logFont.lfHeight = -11;
	logFont.lfWeight = FW_REGULAR;
	logFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	wcscpy(logFont.lfFaceName, L"Tahoma");
	m_Font.CreateFontIndirect(&logFont);
	m_comboFontName.SetFont(&m_Font);
	m_comboFontSize.SetFont(&m_Font);

	m_comboFontSize.LimitText(4);

	FillFontNames(m_comboFontName);
	FillFontSizes(m_comboFontSize);

	theApp.SetFormatBar(this);

	return 0;
}

static BOOL HasFocus(HWND hWnd)
{
	HWND hFocusWnd = ::GetFocus();
	return (hFocusWnd == hWnd || ::IsChild(hWnd, hFocusWnd));
}

void CFormatBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CToolBar::OnUpdateCmdUI(pTarget, bDisableIfNoHndler);
	// don't update combo boxes if either one has the focus
	if (HasFocus(theApp.GetRightWnd()) || 
		HasFocus(m_comboFontName) ||
		HasFocus(m_comboFontSize)
		)
	{
		m_comboFontName.EnableWindow();
		m_comboFontSize.EnableWindow();
		if (!HasFocus(m_comboFontName) && !HasFocus(m_comboFontSize))
			SyncToView();
	}
	else
	{
		m_comboFontName.EnableWindow(FALSE);
		m_comboFontSize.EnableWindow(FALSE);
	}
}

void CFormatBar::SyncToView()
{
	// get the current font from the view and update
	CHARHDR fh;
	CHARFORMAT& cf = fh.cf;
	fh.hwndFrom = m_hWnd;
	fh.idFrom = GetDlgCtrlID();
	fh.code = FN_GETFORMAT;
	VERIFY(::SendMessage(theApp.GetRightWnd(), WM_NOTIFY, fh.idFrom, (LPARAM)&fh));

	// the selection must be same font and charset to display correctly
	if (cf.dwMask & CFM_FACE)
	{
		int id = m_comboFontName.FindStringExact(-1, cf.szFaceName);
		if (id != CB_ERR && m_comboFontName.GetCurSel() != id)
			m_comboFontName.SetCurSel(id);
	}
	else
	{
		m_comboFontName.SetCurSel(-1);
	}

	if (cf.dwMask & CFM_SIZE)
	{
		TCHAR t[33];
		_itow(cf.yHeight/20, t, 10);
		int id = m_comboFontSize.FindStringExact(-1, t);
		if (id != CB_ERR)
		{
			if (m_comboFontSize.GetCurSel() != id)
				m_comboFontSize.SetCurSel(id);
		}
		else
		{
			CString t2;
			m_comboFontSize.GetWindowText(t2);
			if (t2 != t)
				m_comboFontSize.SetWindowText(t);
		}
	}
	else
	{
		m_comboFontSize.SetCurSel(-1);
	}
}

void CFormatBar::OnSelectFontName()
{
	CString szFontName;
	int idx = m_comboFontName.GetCurSel();
	m_comboFontName.GetLBText(idx, szFontName);

	CHARHDR fh;

	CHARFORMAT2& cf = fh.cf;
	fh.hwndFrom = m_hWnd;
	fh.idFrom = GetDlgCtrlID();
	fh.code = FN_SETFORMAT;
	cf.dwMask = CFM_FACE;
	_tcsncpy(cf.szFaceName, szFontName, LF_FACESIZE);
	::SendMessage(theApp.GetRightWnd(), WM_NOTIFY, fh.idFrom, (LPARAM)&fh);
}

void CFormatBar::OnSelectFontSize()
{
	CString szSize;
	int idx = m_comboFontSize.GetCurSel();
	if (idx == CB_ERR)
		m_comboFontSize.GetWindowText(szSize);
	else
		m_comboFontSize.GetLBText(idx, szSize);
	int nSize = _ttoi(szSize);
	if (!nSize)
		return;

	CHARHDR fh;
	CHARFORMAT2& cf = fh.cf;
	fh.hwndFrom = m_hWnd;
	fh.idFrom = GetDlgCtrlID();
	fh.code = FN_SETFORMAT;
	cf.dwMask = CFM_SIZE;
	cf.yHeight = nSize * 20;
	::SendMessage(theApp.GetRightWnd(), WM_NOTIFY, fh.idFrom, (LPARAM)&fh);
}

BOOL CFormatBar::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		NMHDR nm;
		nm.hwndFrom = m_hWnd;
		nm.idFrom = GetDlgCtrlID();
		nm.code = NM_RETURN;
		switch (pMsg->wParam)
		{
			case VK_RETURN:
				// Send change notification
				if (HasFocus(m_comboFontName))
					OnSelectFontName();
				if (HasFocus(m_comboFontSize))
					OnSelectFontSize();
				//Fall through

			case VK_ESCAPE:
				GetOwner()->SendMessage(WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
				return TRUE;
		}
	}
	
	return CToolBar::PreTranslateMessage(pMsg);
}
