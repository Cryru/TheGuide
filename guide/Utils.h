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

struct wintree_t;

//-----------------------------------------------------------------------------
// TemplateHelper

class TemplateHelper
{
public:
	TemplateHelper();
	void SetTemplate(const PrintTemplate& t);
	CStringA GetHeading(const char *headingNumber, int headingLevel, 
		const wchar_t *headingText, bool omitLeadingBlankLine,
		DWORD fgColor, DWORD bgColor);
	void Clear();

private:
	PrintTemplate m_Template;
	bool m_bFirstTime;
};

//-----------------------------------------------------------------------------
// HeadingCounter

class HeadingCounter
{
public:
	HeadingCounter();
	void Clear();
	void CollectAll(struct wintree_t *pWinTree);
	void CollectRecurse(struct wintree_t *pWinTree, HTREEITEM item);
	void CollectOne(struct wintree_t *pWinTree, HTREEITEM item);
	CStringA GetHeadingNumber(struct tree_node_t *node);

private:
	struct MapValue
	{
		UINT m_nSelfNumber;
		UINT m_nChildCounter;
	};

	typedef CMap<struct tree_node_t *, struct tree_node_t *&, MapValue, MapValue&> map_t;

	map_t m_Map;

	UINT m_nTopLevelCounter;

	static int HeadingCallback(HTREEITEM item, 
		struct tree_node_t *node, void *cargo);
};

//-----------------------------------------------------------------------------
// RTFConcatenator

class RTFConcatenator
{
public:
	RTFConcatenator();
	~RTFConcatenator();

	void SetTemplate(const PrintTemplate& t);
	void CollectAll(struct wintree_t *pWinTree);
	void CollectRecurse(struct wintree_t *pWinTree, HTREEITEM hItem);
	void CollectOne(struct wintree_t *pWinTree, HTREEITEM hItem);
	void GetRTF(CStringA& rtf);
	void Clear();
	HWND GetWnd() const { return m_hWnd; }

private:
	HWND m_hWnd;
	HWND m_hZeroCheckWnd;
	struct tree_node_t *m_pTopNode;
	TemplateHelper m_TemplateHelper;
	HeadingCounter m_HeadingCounter;
	bool m_bLastTextEmpty;

	static int CollectCallback(HTREEITEM item, 
		struct tree_node_t *node, void *cargo);
};

//-----------------------------------------------------------------------------
// Color Chooser

bool GuideChooseColor(COLORREF initial, HWND owner, COLORREF& color_out);

//-----------------------------------------------------------------------------
// Get date/time strings according to the given format

CString GetDateString(LPCTSTR format);
CString GetTimeString(LPCTSTR format);

CString GetConfiguredDateString();
CString GetConfiguredTimeString();

//-----------------------------------------------------------------------------
// Misc

// Create a RTF-compliant string given a regular text. Escapes metacharacters
// and non-ASCII chars in the given text. Returned text is single-byte format.
CStringA escapeRTF(const CString& string);

// Create a unicode string from an RTF fragment. All control sequences are
// dropped any only the text is extracted. The codepage is required for
// MBCS -> unicode conversion.
CString stripRTF(const CStringA& rtf, unsigned codepage);

// Migrate all old-style links in the given guide. Call only if guide is in
// v1 format (else nothing happens, inefficiently). Unmatchable links (links
// that point to deleted titles for e.g.) are left in the format
// oldlink://./<title> where <title> is the old link title.
void migrateLinks(struct guide_t *guide);

// Populate a combo box with font names and sizes.
void FillFontNames(CComboBox& cb);
void FillFontSizes(CComboBox& cb);
