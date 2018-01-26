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
#include ".\flatbutton.h"

// CFlatButtonBase

IMPLEMENT_DYNAMIC(CFlatButtonBase, CButton)

CFlatButtonBase::CFlatButtonBase()
	: m_bMouseInHouse(false)
	, m_bChecked(false)
{
}

CFlatButtonBase::~CFlatButtonBase()
{
}

void CFlatButtonBase::SetChecked(bool checked)
{
	if (m_bChecked != checked)
	{
		m_bChecked = checked;
		if (::IsWindowVisible(m_hWnd))
			Invalidate();
	}
}

BEGIN_MESSAGE_MAP(CFlatButtonBase, CButton)
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

// CFlatButtonBase message handlers

void CFlatButtonBase::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bMouseInHouse)
	{
		m_bMouseInHouse = true;

		TRACKMOUSEEVENT tme = {0};
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;

		TrackMouseEvent(&tme);

		Invalidate();
	}

	CButton::OnMouseMove(nFlags, point);
}

LRESULT CFlatButtonBase::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_bMouseInHouse);

	m_bMouseInHouse = false;
	Invalidate();

	return 0;
}

// CFlatButton

static CBrush gFrameBrush(RGB(10, 36, 106));
static CBrush gGrayBrush(RGB(128, 128, 128));

IMPLEMENT_DYNAMIC(CFlatButton, CFlatButtonBase)

CFlatButton::CFlatButton()
{
}

CFlatButton::~CFlatButton()
{
}

BEGIN_MESSAGE_MAP(CFlatButton, CFlatButtonBase)
END_MESSAGE_MAP()

// CFlatButton message handlers

void CFlatButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect r = lpDrawItemStruct->rcItem;

	CString text;
	GetWindowText(text);

	bool bFocus = (lpDrawItemStruct->itemState & ODS_FOCUS) != 0;

	if (IsChecked() == false && IsMouseInHouse() == false)
	{
		// normal
		pDC->FillSolidRect(&r, ::GetSysColor(COLOR_BTNFACE));
		pDC->SetTextColor(RGB(0,0,0));
		pDC->DrawText(text, &r, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	}
	else if (IsChecked() && IsMouseInHouse() == false)
	{
		// checked, not hover
		pDC->FillSolidRect(&r, RGB(212, 213, 216));
		pDC->SetTextColor(RGB(0,0,0));
		pDC->DrawText(text, &r, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
		pDC->FrameRect(&r, &gFrameBrush);
	}
	else if (IsChecked() == false && IsMouseInHouse())
	{
		// not checked, hover
		pDC->FillSolidRect(&r, RGB(182, 189, 210));
		pDC->SetTextColor(RGB(0,0,0));
		pDC->DrawText(text, &r, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
		pDC->FrameRect(&r, &gFrameBrush);
	}
	else
	{
		// checked, hover
		pDC->FillSolidRect(&r, RGB(133, 146, 181));
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->DrawText(text, &r, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
		pDC->FrameRect(&r, &gFrameBrush);
	}
}

// CColorButton

IMPLEMENT_DYNAMIC(CColorButton, CFlatButtonBase)

CColorButton::CColorButton()
	: m_Color(0)
{
}

CColorButton::~CColorButton()
{
}

void CColorButton::SetColor(COLORREF c)
{
	if (c != m_Color)
	{
		m_Color = c;
		if (IsWindowVisible())
			Invalidate();
	}
}

BEGIN_MESSAGE_MAP(CColorButton, CFlatButtonBase)
END_MESSAGE_MAP()

// CColorButton message handlers

void CColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect r = lpDrawItemStruct->rcItem;
	CRect r1(r);
	r1.DeflateRect(1, 1);

	CRect r2(r);
	r2.DeflateRect(3, 3);

	CRect r3(r);
	r3.DeflateRect(4, 4);

	pDC->FillSolidRect(&r, ::GetSysColor(COLOR_BTNFACE));
	if (IsChecked() || IsMouseInHouse())
		pDC->FrameRect(&r1, &gFrameBrush);
	pDC->FrameRect(&r2, &gGrayBrush);
	pDC->FillSolidRect(&r3, m_Color);
}
