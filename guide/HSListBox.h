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

#pragma once

// CHSListBox

class CHSListBox : public CListBox
{
private:
	// yeah, I'm dynamic!
	DECLARE_DYNAMIC(CHSListBox)

	// the print template which we're rendering
	PrintTemplate& m_refPrintTemplate;

	// pen to draw the selection border
	CPen m_penSelectionBorder;

	// map of fonts for each heading
	typedef CFont *CFontPtr;
	CMap<int, int&, CFontPtr, CFontPtr&> m_fonts;

	// methods to manipulate the map
	CFont* GetFontForIndex(int idx);
	void DestroyFontForIndex(int idx);
	void DestroyAllFonts();

	// make up text for a given index
	CString GetTextForIndex(int idx);

public:
	// ctor/dtor
	CHSListBox(PrintTemplate& t);
	~CHSListBox();

	// owner-draw
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	int CompareItem(LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);
	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

	// API to redraw items
	void RedrawItem(int idx, bool fontChanged = false);
	void RedrawAll();

	// the message map
	DECLARE_MESSAGE_MAP()
};

// A Helper API
void makeLogFont(LOGFONT& logFont, const HeadingStyle& hs);
