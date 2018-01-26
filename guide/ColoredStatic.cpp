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
#include ".\coloredstatic.h"

// CColoredStatic

IMPLEMENT_DYNAMIC(CColoredStatic, CStatic)

BEGIN_MESSAGE_MAP(CColoredStatic, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

CColoredStatic::CColoredStatic()
	: m_cFore(RGB(0,0,0))
	, m_cBack(RGB(255,255,255))
{
	m_hFont = ::GetStockObject(DEFAULT_GUI_FONT);
	ASSERT(m_hFont);
}

CColoredStatic::~CColoredStatic()
{
	// no need to:
	// DeleteObject(m_hFont);
	// since it is a stock object.

	m_hFont = NULL;
}

void CColoredStatic::SetForeground(COLORREF fore)
{
	if (m_cFore != fore)
	{
		m_cFore = fore;
		Invalidate();
	}
}

void CColoredStatic::SetBackground(COLORREF back)
{
	if (m_cBack != back)
	{
		m_cBack = back;
		Invalidate();
	}
}

void CColoredStatic::OnPaint()
{
	CPaintDC dc(this);

	// use a nice font
	HGDIOBJ oldFont = dc.SelectObject(m_hFont);

	// draw background
	CRect aRect;
	GetClientRect(aRect);
	dc.FillSolidRect(aRect, m_cBack);

	// set foreground color
	dc.SetTextColor(m_cFore);

	// sample text
	const CString& sampleText = RCSTR(IDS_SAMPLETEXT);
	static const unsigned sampleTextLen = sampleText.GetLength();

	// draw text at centre of rect
	dc.SetTextAlign(TA_CENTER|TA_TOP);
	CSize aExtent = dc.GetTextExtent(sampleText, sampleTextLen);
	dc.TextOut(
		aRect.Width()/2, 
		(aRect.Height()-aExtent.cy)/2, 
		sampleText, sampleTextLen);

	// reset font
	dc.SelectObject(oldFont);
}
