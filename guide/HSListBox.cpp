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
#include "HSListBox.h"

// -- helper methods ----------------------------------------------------------

void makeLogFont(LOGFONT& logFont, const HeadingStyle& hs)
{
	// calculate font height
	HDC hScreenDC = ::GetDC(NULL);
	LONG lfHeight = 
		-MulDiv(hs.fontPointSize, GetDeviceCaps(hScreenDC, LOGPIXELSY), 72);
	::ReleaseDC(NULL, hScreenDC);

	// fill out the structure
	memset(&logFont, 0, sizeof(logFont));
	logFont.lfHeight = lfHeight;
	logFont.lfWeight = hs.fontBold ? FW_BOLD : FW_REGULAR;
	logFont.lfItalic = hs.fontItalic;
	logFont.lfUnderline = hs.fontUnderline;
	logFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
	wcscpy(logFont.lfFaceName, hs.fontFamily);
}

// -- CHSListBox --------------------------------------------------------------

IMPLEMENT_DYNAMIC(CHSListBox, CListBox)

CHSListBox::CHSListBox(PrintTemplate& t)
	: m_refPrintTemplate(t)
{
	// create the pen used to draw the selection border
	m_penSelectionBorder.CreatePen(PS_SOLID, 2, RGB(0x08,0x22,0x6a));
}

CHSListBox::~CHSListBox()
{
	DestroyAllFonts();
}

BEGIN_MESSAGE_MAP(CHSListBox, CListBox)
END_MESSAGE_MAP()

// CHSListBox message handlers

CFont* CHSListBox::GetFontForIndex(int idx)
{
	CFont *pFont = NULL;

	// see if we have in the map already
	if (!m_fonts.Lookup(idx, pFont))
	{
		// get the style
		const HeadingStyle& hs = m_refPrintTemplate.headings[idx];

		// fill out a logfont structure
		LOGFONT logFont;
		makeLogFont(logFont, hs);

		// create the font
		pFont = new CFont();
		VERIFY(pFont->CreateFontIndirect(&logFont));

		// add it in the map
		m_fonts[idx] = pFont;
	}

	return pFont;
}

void CHSListBox::DestroyFontForIndex(int idx)
{
	CFont *pFont = NULL;

	if (m_fonts.Lookup(idx, pFont))
	{
		delete pFont;
		m_fonts.RemoveKey(idx);
	}
}

void CHSListBox::DestroyAllFonts()
{
	POSITION pos = m_fonts.GetStartPosition();

	while (pos)
	{
		CFont *pFont = NULL;
		int idx = 0;
		m_fonts.GetNextAssoc(pos, idx, pFont);

		ASSERT(pFont);
		delete pFont;
	}

	m_fonts.RemoveAll();
}

void CHSListBox::RedrawItem(int idx, bool fontChanged)
{
	if (fontChanged)
		DestroyFontForIndex(idx);

	// till we find a better strategy, repaint everything
	Invalidate();
}

void CHSListBox::RedrawAll()
{
	DestroyAllFonts();
	Invalidate();
}

CString CHSListBox::GetTextForIndex(int itemID)
{
	// resource strings:
	//	IDS_HDGFMT1		"%s Heading %d"
	//	IDS_HDGFMT2		"Heading %d"

	CString text;

	// get the heading style
	const HeadingStyle& hs = m_refPrintTemplate.headings[itemID];

	// if numbered, prepend a "1.1.1" etc.
	if (hs.numbered)
	{
		CString t;
		for (int k=0; k<=itemID; ++k)
		{
			t += L"1.";
		}
		t.Delete(t.GetLength()-1);
		text.Format(RCSTR(IDS_HDGFMT1), t, itemID+1);
	}
	else
	{
		text.Format(RCSTR(IDS_HDGFMT2), itemID+1);
	}

	// if page break, prepend a paragraph character
	if (hs.breakPage)
	{
		text = L"\xb6 " + text;
	}

	return text;
}

void CHSListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	int itemID = lpDrawItemStruct->itemID;
	RECT &r = lpDrawItemStruct->rcItem;

	// use our font and pen
	CFont *oldFont = (CFont *)pDC->SelectObject(GetFontForIndex(itemID));
	CPen *oldPen = (CPen *)pDC->SelectObject(&m_penSelectionBorder);

	// do all this on selection change or redraw
	if (lpDrawItemStruct->itemAction == ODA_SELECT ||
		lpDrawItemStruct->itemAction == ODA_DRAWENTIRE)
	{
		// clear background -- XXX: is this required?
		pDC->FillSolidRect(&r, RGB(255,255,255));

		// draw selection rect
		if (lpDrawItemStruct->itemState & ODS_SELECTED)
		{
			// draw selection rectangle
			pDC->Rectangle(r.left+2, r.top+2, r.right-1, r.bottom-1);
		}

		// draw text:

		// get rectangle to draw it in
		CRect rectText(r);
		rectText.DeflateRect(4, 4);
		// get the style
		const HeadingStyle& hs = m_refPrintTemplate.headings[itemID];
		// make flags
		UINT flags = DT_LEFT;
		if (hs.justify == HeadingStyle::JFY_CENTER)
			flags = DT_CENTER;
		else if (hs.justify == HeadingStyle::JFY_RIGHT)
			flags = DT_RIGHT;
		// set the color
		pDC->SetTextColor(hs.fontColor);
		// actually draw it
		pDC->DrawText(GetTextForIndex(itemID), &rectText, flags);
	}

	// put back old pen and font
	pDC->SelectObject(oldPen);
	pDC->SelectObject(oldFont);
}

int CHSListBox::CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct)
{
	return 0;
}

void CHSListBox::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
	// empty
}

void CHSListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = 50;
}
