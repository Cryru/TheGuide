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
#include "guide.h"
#include "utils.h"
#include <libguide/wintree.h>

//-- TemplateHelper -----------------------------------------------------------

static const char *_TH_TEMPLATE =
"{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033{\\fonttbl{\\f0\\fmodern\\fprq0\\fcharset1 @FONTFACE@;}}\n"
"@COLORTBL@" //"{\colortbl ;\red0\green128\blue0;}\n"
"\\viewkind4\\uc1\\pard@PAGE@\\sb0\\sa0\\f0\\fs@TWIPS@@LEAD@\n"
"@TITLE@\\par\\par}\n";

TemplateHelper::TemplateHelper()
	: m_bFirstTime(true)
{
	// empty
}

void TemplateHelper::SetTemplate(const PrintTemplate& t)
{
	m_Template = t;
}

CStringA TemplateHelper::GetHeading(const char *headingNumber, int headingLevel,
	const wchar_t *headingText, bool omitLeadingBlankLine, DWORD fgColor, DWORD bgColor)
{
	// do not exceed max heading level!!
	if (headingLevel >= MAX_HEADING_LEVEL)
		headingLevel = MAX_HEADING_LEVEL-1;
	if (headingLevel < 0)
		headingLevel = 0;

	// get the heading style we have to use
	const HeadingStyle& hs = m_Template.headings[headingLevel];

	// get font size in twips
	char fontTwips[32];
	itoa(hs.fontPointSize * 2, fontTwips, 10);

	// NOTE: the order of the following steps is important!

	// get the title, with heading number if required
	CStringA title;
	if (hs.numbered)
	{
		// add the heading number, followed by a space
		title += headingNumber;
		title += " ";
	}
	title += escapeRTF(headingText);

	// use color if required, modify `title'.
	CStringA colorTbl;
	if (fgColor != (DWORD)-1 || hs.fontColor != 0 || bgColor != (DWORD)-1)
	{
		COLORREF clr = fgColor != (DWORD)(-1) ? fgColor : hs.fontColor;

		if (bgColor != (DWORD)-1)
		{
			// add two color table entries with the first entry as the hdg's color
			// and the send as the bg color
			colorTbl.Format(
				"{\\colortbl ;\\red%d\\green%d\\blue%d;\\red%d\\green%d\\blue%d;}\n",
				GetRValue(clr), GetGValue(clr), GetBValue(clr),
				GetRValue(bgColor), GetGValue(bgColor), GetBValue(bgColor));

			// change the title to use color
			title = "\\highlight2\\cf1 " + title;
			title += "\\highlight0\\cf0";
		}
		else
		{
			// add a color table with the first entry as the hdg's color
			colorTbl.Format(
				"{\\colortbl ;\\red%d\\green%d\\blue%d;}\n", 
				GetRValue(clr), GetGValue(clr), GetBValue(clr));

			// change the title to use color
			title = "\\cf1 " + title;
			title += "\\cf0";
		}
	}

	// modify `title' for bold, italic and/or underline.
	if (hs.fontBold || hs.fontItalic || hs.fontUnderline)
	{
		CStringA decorate;
		decorate.Format(
			"%s%s%s%s%s%s%s%s",
			hs.fontBold		 ? "\\b"  : "",
			hs.fontItalic	 ? "\\i"  : "",
			hs.fontUnderline ? "\\ul" : "",
			title,
			title[0] == '\\' ? "" : " ",
			hs.fontUnderline ? "\\ul0" : "",
			hs.fontItalic	 ? "\\i0"  : "",
			hs.fontBold		 ? "\\b0"  : "");
		title = decorate;
	}

	// modify `title' for justification (center or right only).
	// NOTE: keep the following array in sync with the order of `HeadingStyle::_justify' !
	static const char *_jfy_codes[] = { "\\ql", "\\qc", "\\qr" };
	if (hs.justify == HeadingStyle::JFY_CENTER || hs.justify == HeadingStyle::JFY_RIGHT)
	{
		CStringA t;
		if (title[0] == '\\') // so that an unnecessary blank is avoided
		{
			t = _jfy_codes[hs.justify];
		}
		else
		{
			t.Format("%s ", _jfy_codes[hs.justify]); // need a blank in this case
		}
		title = t + title;
	}

	// replace entries in template
	CStringA out(_TH_TEMPLATE);
	out.Replace("@LEAD@", omitLeadingBlankLine ? "" : "\\par");
	out.Replace("@PAGE@", (hs.breakPage && !m_bFirstTime) ? "\\page" : "");
	out.Replace("@FONTFACE@", escapeRTF(hs.fontFamily));
	out.Replace("@COLORTBL@", colorTbl);
	out.Replace("@TWIPS@", fontTwips);
	out.Replace("@TITLE@", title);

	// to avoid page break as the first thing in the output!
	m_bFirstTime = false;

	// done
	return out;
}

void TemplateHelper::Clear()
{
	m_bFirstTime = true;
}

//-- HeadingCounter -----------------------------------------------------------

HeadingCounter::HeadingCounter()
{
	Clear();
}

void HeadingCounter::Clear()
{
	m_nTopLevelCounter = 0;
	m_Map.RemoveAll();
}

int HeadingCounter::HeadingCallback(HTREEITEM item, 
	struct tree_node_t *node, void *cargo)
{
	HeadingCounter *that = (HeadingCounter *)cargo;

	struct tree_node_t *parent = tree_get_parent(node);
	MapValue parentValue;
	UINT selfNumber;
	if (that->m_Map.Lookup(parent, parentValue))
	{
		selfNumber = ++parentValue.m_nChildCounter;
		that->m_Map[parent] = parentValue;
	}
	else
	{
		selfNumber = ++that->m_nTopLevelCounter;
	}
	MapValue nodeValue = { selfNumber, 0 };
	that->m_Map[node] = nodeValue;

	return 0;
}

void HeadingCounter::CollectAll(struct wintree_t *pWinTree)
{
	ASSERT(pWinTree);

	Clear();
	wintree_traverse(pWinTree, HeadingCallback, this);
}

void HeadingCounter::CollectRecurse(struct wintree_t *pWinTree, HTREEITEM hItem)
{
	ASSERT(pWinTree);
	ASSERT(hItem);

	Clear();
	wintree_traverse_subtree(pWinTree, hItem, HeadingCallback, this);
}

void HeadingCounter::CollectOne(struct wintree_t *pWinTree, HTREEITEM hItem)
{
	ASSERT(pWinTree);
	ASSERT(hItem);

	Clear();
	struct tree_node_t *node = wintree_get_node_from_item(pWinTree, hItem);
	ASSERT(node);
	HeadingCallback(hItem, node, this);
}

CStringA HeadingCounter::GetHeadingNumber(struct tree_node_t *node)
{
	CStringA result;
	char tmp[1024];

	// collect the numbers as a string (in reverse order)
	MapValue value;
	while (m_Map.Lookup(node, value))
	{
		result.Insert(0, itoa(value.m_nSelfNumber, tmp, 10));
		result.Insert(0, ".");

		node = tree_get_parent(node);
	}

	// remove the leading '.'
	ASSERT(result.IsEmpty() == false);
	if (!result.IsEmpty())
		result.Delete(0);

	return result;
}

//-----------------------------------------------------------------------------

RTFConcatenator::RTFConcatenator()
	: m_bLastTextEmpty(false)
{
	// create the hidden rtf window
	m_hWnd = ::CreateWindow(RICHEDIT50W, _T("CollectTemp"), ES_MULTILINE,
					0, 0, 100, 100, NULL, NULL, NULL, NULL);
	ASSERT(m_hWnd);

	// set to empty RTF
	::SetRTF(m_hWnd, theEmptyRTFText, false);

	// set the text limit
	::SendMessage(m_hWnd, EM_EXLIMITTEXT, 0, CRichEditView::lMaxSize);

	// create another hidden rtf window to get length of RTF strings!
	m_hZeroCheckWnd = ::CreateWindow(RICHEDIT50W, _T("CollectTemp0"), ES_MULTILINE,
					0, 0, 100, 100, NULL, NULL, NULL, NULL);
	ASSERT(m_hZeroCheckWnd);
	::SetRTF(m_hZeroCheckWnd, theEmptyRTFText, false);
	::SendMessage(m_hZeroCheckWnd, EM_EXLIMITTEXT, 0, CRichEditView::lMaxSize);
}

void RTFConcatenator::SetTemplate(const PrintTemplate& t)
{
	m_TemplateHelper.SetTemplate(t);
}

RTFConcatenator::~RTFConcatenator()
{
	if (m_hWnd)
		::DestroyWindow(m_hWnd);
	m_hWnd = 0;

	if (m_hZeroCheckWnd)
		::DestroyWindow(m_hZeroCheckWnd);
	m_hZeroCheckWnd = 0;
}

int RTFConcatenator::CollectCallback(HTREEITEM item, 
	struct tree_node_t *node, void *cargo)
{
	RTFConcatenator *that = (RTFConcatenator *)cargo;
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);
	ASSERT(node);
	ASSERT(cargo);
	ASSERT(item);

	// position cursor at end and clear selection
	HWND hWnd = that->m_hWnd;
	::SendMessage(hWnd, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);

	// get heading level
	int level = 0; // 0 => no other top nodes
	struct tree_node_t *node2 = node;
	while (tree_get_parent(node2) != that->m_pTopNode)
	{
		level++;
		node2 = tree_get_parent(node2);
	}

	// append heading rtf text
	CStringA heading = 
		that->m_TemplateHelper.GetHeading(
			that->m_HeadingCounter.GetHeadingNumber(node),
			level,
			data->title,
			that->m_bLastTextEmpty,
			data->color, data->bgcolor);
	SETTEXTEX tex;
	tex.codepage = CP_ACP; // ???
	tex.flags = ST_SELECTION;
	LRESULT lr = ::SendMessage(hWnd, EM_SETTEXTEX, (WPARAM)&tex, (LPARAM)(const char *)heading);
	ASSERT(lr > 0);

	// check the number of characters in the text
	SetRTF(that->m_hZeroCheckWnd, data->text, false);
	LRESULT nChars = ::SendMessage(that->m_hZeroCheckWnd, WM_GETTEXTLENGTH, 0, 0);
	if (nChars > 0)
	{
		// append rtf text (only if some text is really present)
		if (data->text[0] != 0 && strcmp(theEmptyRTFText, data->text) != 0) //theEmptyRTFText != data->text)
			::SendMessage(hWnd, EM_SETTEXTEX, (WPARAM)&tex, (LPARAM)data->text);
	}
	that->m_bLastTextEmpty = (nChars == 0);

	// continue
	return 0;
}

void RTFConcatenator::CollectAll(struct wintree_t *pWinTree)
{
	ASSERT(pWinTree);

	// set top node
	m_pTopNode = tree_get_root(wintree_get_tree(pWinTree));

	// make headings
	m_HeadingCounter.CollectAll(pWinTree);

	wintree_traverse(pWinTree, CollectCallback, this);
}

void RTFConcatenator::CollectRecurse(struct wintree_t *pWinTree, HTREEITEM hItem)
{
	ASSERT(pWinTree);
	ASSERT(hItem);

	// set top node
	m_pTopNode = tree_get_parent(wintree_get_node_from_item(pWinTree, hItem));

	// make headings
	m_HeadingCounter.CollectRecurse(pWinTree, hItem);

	wintree_traverse_subtree(pWinTree, hItem, CollectCallback, this);
}

void RTFConcatenator::CollectOne(struct wintree_t *pWinTree, HTREEITEM hItem)
{
	ASSERT(pWinTree);
	ASSERT(hItem);

	// set top node
	m_pTopNode = tree_get_parent(wintree_get_node_from_item(pWinTree, hItem));

	// make headings
	m_HeadingCounter.CollectOne(pWinTree, hItem);

	struct tree_node_t *node = wintree_get_node_from_item(pWinTree, hItem);
	ASSERT(node);
	CollectCallback(hItem, node, this);
}

void RTFConcatenator::GetRTF(CStringA& rtf)
{
	ASSERT(m_hWnd);
	rtf = ::GetRTF(m_hWnd);
}

void RTFConcatenator::Clear()
{
	// set to empty RTF
	::SetRTF(m_hWnd, theEmptyRTFText, false);

	// reset heading counter
	m_HeadingCounter.Clear();

	// reset template helper
	m_TemplateHelper.Clear();

	// reset state
	m_bLastTextEmpty = false;
}

//-----------------------------------------------------------------------------

bool GuideChooseColor(COLORREF initial, HWND owner, COLORREF& color_out)
{
	CustomColors custom;
	theApp.GetCustomColors(custom);

	CHOOSECOLOR aChooseColor = {0};
	aChooseColor.lStructSize = sizeof(CHOOSECOLOR);
	aChooseColor.hwndOwner = owner;
	aChooseColor.rgbResult = initial;
	aChooseColor.lpCustColors = custom.colors;
	aChooseColor.Flags = CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT;

	bool ok = ::ChooseColor(&aChooseColor) != 0;
	if (ok)
	{
		theApp.SetCustomColors(custom);
		color_out = aChooseColor.rgbResult;
	}

	return ok;
}

//-----------------------------------------------------------------------------
// Get date/time strings according to the given format

CString GetDateString(LPCTSTR format)
{
	TCHAR result[128] = {0};

	::GetDateFormat(LOCALE_USER_DEFAULT, 0, NULL, format, result, 
		sizeof(result)/sizeof(*result));
	return result;
}

CString GetTimeString(LPCTSTR format)
{
	TCHAR result[128] = {0};

	::GetTimeFormat(LOCALE_USER_DEFAULT, 0, NULL, format, result, 
		sizeof(result)/sizeof(*result));
	return result;
}

CString GetConfiguredDateString()
{
	DateTimeFormat f;
	theApp.GetDateTimeFormat(f);

	return GetDateString(f.useSystemDateFormat ? NULL : (LPCTSTR)f.dateFormat);
}

CString GetConfiguredTimeString()
{
	DateTimeFormat f;
	theApp.GetDateTimeFormat(f);

	return GetTimeString(f.useSystemTimeFormat ? NULL : (LPCTSTR)f.timeFormat);
}

//-------------------------------------------------------------------------------
// escape RTF

CStringA escapeRTF(const CString& string)
{
	CStringA out;
	char tmp[10];

	const wchar_t *p = string;
	wchar_t ch;
	while ((ch = *p++))
	{
		if (ch == L'\\' || ch == L'{' || ch == L'}')
		{
			out += '\\';
			out += (char) (ch & 0xff);
		}
		else if (ch < 32 || (ch > 127 && ch <= 255))
		{
			sprintf(tmp, "\\'%02x", ch);
			out += tmp;
		}
		else if (ch >= 32 && ch <= 127)
		{
			out += (char) (ch & 0xff);
		}
		else
		{
			sprintf(tmp, "\\u%u?", ch); // note: msftedit 5.4 apparently uses \uc1
			out += tmp;
		}
	}

	return out;
}

static char unhex(char d1, char d2)
{
	char v1 = 
		(d1 >= '0' && d1 <= '9') ? (d1-'0') : 
			(d1 >= 'a' && d1 <= 'f') ? (d1-'a'+10) :
				(d1 >= 'A' && d1 <= 'F') ? (d1-'A'+10) : 0;
	char v2 = 
		(d2 >= '0' && d2 <= '9') ? (d2-'0') : 
			(d2 >= 'a' && d2 <= 'f') ? (d2-'a'+10) :
				(d2 >= 'A' && d2 <= 'F') ? (d2-'A'+10) : 0;

	return v1<<4 | v2;
}

CString stripRTF(const CStringA& rtf, unsigned codepage)
{
	CStringA out;
	out.Preallocate(rtf.GetLength());

	int state = 0;
	const char *p = (const char *)rtf;
	while (*p)
	{
		char ch = *p;

		if (state == 0)
		{
			if (ch == '\\')
				state = 1;
			else if (ch == '\r')
				/*ignore*/;
			else
//				out += wchar_t(ch);
				out += ch;
		}
		else if (state == 1)
		{
			if (ch == '\\' || ch == '{')
			{
				out += ch; // escaped characters
				state = 0;
			}
			else if (ch == '\'')
			{
				out += unhex(p[1], p[2]);
				p += 2;
				state = 0;
			}
			else if (ch == 'u')
			{
				// unicode character
//				out += wchar_t((unsigned)atoi(p+1));
//				p += strspn(p, "0123456789");
//				state = 0;
				ASSERT(FALSE);
			}
			else
			{
				state = 2;
			}
		}
		else if (state == 2)
		{
			if (ch == ' ' || ch == '\n' || ch == '}' || ch == '{')
				state = 0;
			else if (ch == '\\')
				state = 1;
		}

		++p;
	}

	CString out2;
	LPTSTR buf = out2.GetBuffer(out.GetLength() + 10);
	MultiByteToWideChar(codepage, 0, out, -1, buf, out.GetLength() + 10);
	out2.ReleaseBuffer();

	return out2;
}

//-------------------------------------------------------------------------------
// link migration stuff

static int getColorTableIndex(CStringA& rtf)
{
	// the result
	int theIndex = 0;

	// see if the color table is present
	int clrTablePos = rtf.Find("{\\colortbl ;");

	// if not, insert one
	if (clrTablePos == -1)
	{
		// locate font table
		int fontTablePos = rtf.Find("\\fonttbl{");
		ASSERT(fontTablePos);
		if (fontTablePos == -1)
		{
			// This means that this is not something generated by
			// the guide. It is also unlikely that these will
			// have links that need to be fixed anyway.

			// For now just throw up our hands..
			ASSERT(FALSE);
			return -1;
		}

		// find the end of the font table
		int fontTableEndPos = rtf.Find("}}", fontTablePos);
		ASSERT(fontTableEndPos != -1);
		if (fontTableEndPos == -1)
			return -1; // duh.. unterminated font table?
		fontTableEndPos += 2;

		// insert the color table
		rtf.Insert(fontTableEndPos, "{\\colortbl ;\\red0\\green0\\blue255;}");

		// only one entry in the colour table, so the index is 1
		// (the index is 1-based).
		return 1;
	}

	// color table exists, add a new entry at the end.
	int clrTableEndPos = rtf.Find(";}", clrTablePos);
	ASSERT(clrTableEndPos != -1);
	if (clrTableEndPos == -1)
		return -1; // duh.. unterminated color table?
	clrTableEndPos += 2;

	// before inserting, count the number of existing entries.
	const char *p = (const char *)rtf + clrTablePos + sizeof("{\\colortbl ;"); // just beyond the ;
	const char *q = (const char *)rtf + clrTableEndPos;
	int count = 0;
	while (p < q)
	{
		if (*p == ';')
			++count;
		++p;
	}

	// insert the new entry.
	rtf.Insert(clrTableEndPos-1, "\\red0\\green0\\blue255;");

	// return the index
	return count + 1;
}

static bool isURL(const CString& link)
{
	return
	   (_tcsncmp(link, _T("http://"), 7) == 0 || _tcsncmp(link, _T("ftp://"),   6) == 0 ||
		_tcsncmp(link, _T("file://"), 7) == 0 || _tcsncmp(link, _T("https://"), 8) == 0 ||
		_tcsncmp(link, _T("mailto:"), 7) == 0);
}

typedef CMap<CString, LPCTSTR, uint32, uint32> uidToTitleMap_t;

static const char *_linkRTF =
	"{\\field{\\*\\fldinst{HYPERLINK \"%s\"}}{\\fldrslt{\\cf%d\\ul %s}}}\\cf0\\ulnone";

static bool fixLinks(CStringA& rtf, uidToTitleMap_t *uidToTitleMapPtr)
{
	// quick check, quick exit
	if (rtf.Find("\\lnkd\\protect") == -1)
		return false;

	int clrTableIndex = getColorTableIndex(rtf);
	if (clrTableIndex == -1)
	{
		// something bad happened
		return false;
	}

	// get code page
	unsigned codepage = CP_ACP;
	int cpgPos = rtf.Find("\\ansicpg");
	if (cpgPos != -1)
		codepage = atoi((LPCSTR)rtf + cpgPos + sizeof("\\ansicpg")-1);

	// for each pattern of 
	//		\lnkd\protect[<space>]<text>\protect0\lnkd0
	// replace with "out", where out is:
	//      sprintf(out, _linkRTF, <nodeurl>, clrTableIndex, <text>)
	// where <nodeurl> is:
	//		lookupNode(stripRTF(<text>))

	int pos, ofs = 0;
	while ( (pos = rtf.Find("\\lnkd\\protect", ofs)) != -1 )
	{
		// ofs2 points to start of text
		int ofs2 = pos + sizeof("\\lnkd\\protect")-1;

		// check if the \protect had a ' ' delimiter, eat it if so
		if (rtf[ofs2] == ' ')
			ofs2++;

		// locate the end tag
		int pos2 = rtf.Find("\\protect0\\lnkd0", ofs2);
		ASSERT(pos2 != -1);
		if (pos2 == -1)
			return false;

		// get the text inbetween
		// here is the link (title of page)..
		CStringA rtfLink = rtf.Mid(ofs2, pos2-ofs2);
		CString link = stripRTF(rtfLink, codepage);

		// look it up
		CStringA url;
		if (isURL(link))
		{
			// points to a URL itself (http://, https://, ftp:// etc..)
			url = escapeRTF(link);

			// There appears to be a strange bug with the rich edit control, in that if
			// the hyperlink and the text are exacly the same, the hyperlink effect goes
			// off. As a workaround, add a space after the URL (this is ignore in our
			// EN_LINK handler).
			url += " ";
		}
		else
		{
			uint32 nodeUid = 0;
			if (uidToTitleMapPtr->Lookup(link, nodeUid))
				// matched a node
				url.Format("node://./%u", nodeUid);
			else
				// unmatched node, we'll handle this later
				url.Format("oldlink://./%s", escapeRTF(link));
		}

		// make the replacement
		CStringA replacement;
		replacement.Format(_linkRTF, url, clrTableIndex, escapeRTF(link));

		// delete [pos, pos2+15]
		rtf.Delete(pos, pos2+15-pos);
		rtf.Insert(pos, replacement);

		// repositon to just after the replacement
		ofs = pos + replacement.GetLength();
	}

	return true;
}

static int _linkFixer(struct tree_node_t *node, void *cargo)
{
	uidToTitleMap_t *uidToTitleMapPtr = (uidToTitleMap_t *)cargo;

	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);

	CStringA rtf = data->text;
	if (fixLinks(rtf, uidToTitleMapPtr))
		guide_nodedata_set_text(data, rtf);

	return 0;
}

static int _mapMaker(struct tree_node_t *node, void *cargo)
{
	uidToTitleMap_t *uidToTitleMapPtr = (uidToTitleMap_t *)cargo;
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);

	uint32 existingUID = 0;
	if (uidToTitleMapPtr->Lookup(data->title, existingUID) == FALSE)
		uidToTitleMapPtr->SetAt(data->title, data->uid);

	return 0;
}

// main API to migrate links to new format
void migrateLinks(struct guide_t *guide)
{
	uidToTitleMap_t uidToTitleMap;
	tree_traverse_preorder(guide->tree, _mapMaker, &uidToTitleMap);

	tree_traverse_preorder(guide->tree, _linkFixer, &uidToTitleMap);
}

struct _fontcb_info
{
	CComboBox *combo;
	CMap<CString, LPCTSTR, UINT, UINT> seen;
};

static int __stdcall FontEnumProc(
  const LOGFONT *p,		// ENUMLOGFONTEX *lpelfe,    // logical-font data
  const TEXTMETRIC *q,	// NEWTEXTMETRICEX *lpntme,  // physical-font data
  DWORD FontType,       // type of font
  LPARAM lParam         // application-defined data
)
{
	ENUMLOGFONTEX *lpelfe = (ENUMLOGFONTEX *)p;
	_fontcb_info *info = (_fontcb_info *)lParam;
	LPCTSTR name = lpelfe->elfLogFont.lfFaceName;
	UINT dummy;
	if (info->seen.Lookup(name, dummy) == FALSE)
	{
		info->combo->AddString(name);
		info->seen.SetAt(name, 1);
	}
	return 1;
}

void FillFontNames(CComboBox& cb)
{
	LOGFONT lFont;
	memset(&lFont, 0, sizeof(lFont));
	lFont.lfCharSet = DEFAULT_CHARSET;

	_fontcb_info fi;
	fi.combo = &cb;

	HDC hScreenDC = ::GetDC(NULL);
	::EnumFontFamiliesEx(hScreenDC, &lFont, &FontEnumProc, (LPARAM)&fi, 0);
	::ReleaseDC(NULL, hScreenDC);
}

// This is simply hard-coded for convenience. Should be good enough for
// The Guide. The user can always edit it anyway.
void FillFontSizes(CComboBox& cb)
{
	const static TCHAR *szFontSizes[] = 
		{  _T("8"),  _T("9"), _T("10"), _T("11"), _T("12"), _T("14"), _T("16"),
		  _T("18"), _T("20"), _T("22"), _T("24"), _T("26"), _T("28"), _T("36"), 
		  _T("48"), _T("72") };

	for (int i=0; i<sizeof(szFontSizes)/sizeof(*szFontSizes); ++i)
		cb.AddString(szFontSizes[i]);
}
