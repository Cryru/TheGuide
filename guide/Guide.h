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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"


// -- Preferences -------------------------------------------------------------

struct PreferredFont
{
	TCHAR faceName[LF_FACESIZE];
	int	 height;
	bool bold;
	bool italic;

	inline bool operator==(const PreferredFont& that) const;
};

inline bool PreferredFont::operator==(const PreferredFont& that) const
{
	return _tcscmp(faceName, that.faceName) == 0 &&
		height == that.height &&
		bold == that.bold &&
		italic == that.italic;
}

extern const PreferredFont theDefaultFont;

enum StartupOption
{
	SO_EMPTY_ENV,
	SO_LAST_FILE
};

struct HeadingStyle
{
	enum _justify { JFY_LEFT, JFY_CENTER, JFY_RIGHT };

	TCHAR fontFamily[LF_FACESIZE];
	int  fontPointSize;
	bool fontBold;
	bool fontItalic;
	bool fontUnderline;
	COLORREF fontColor;

	bool breakPage;
	bool numbered;
	_justify justify;

	HeadingStyle();
};

#define MAX_HEADING_LEVEL		5

struct PrintTemplate
{
	HeadingStyle headings[MAX_HEADING_LEVEL];

	PrintTemplate();
};

struct CustomColors
{
	COLORREF colors[16];
};

struct DateTimeFormat
{
	bool useSystemDateFormat;
	CString dateFormat;

	bool useSystemTimeFormat;
	CString timeFormat;
};

enum TreeDecoration
{
	TD_NONE					= 0x0000,
	TD_ICONS				= 0x0001,
	TD_CHECKBOXES			= 0x0002,
	TD_BOTH					= 0x0003,
};
#define SHOW_ICONS(td)			(((td) & 0x0001) != 0)
#define SHOW_CHECKBOXES(td)		(((td) & 0x0002) != 0)

// -- CGuideApp  --------------------------------------------------------------

class CGuideApp : public CWinApp
{
public:
	CGuideApp();
	HWND GetLeftWnd() const { return hLeftWnd; }
	void SetLeftWnd(HWND l) { hLeftWnd = l; }
	CView *GetLeftView() const { return pLeftView; }
	void SetLeftView(CView *p) { pLeftView = p; }
	HWND GetRightWnd() const { return hRightWnd; }
	void SetRightWnd(HWND h) { hRightWnd = h; }
	CView *GetRightView() const { return pRightView; }
	void SetRightView(CView *p) { pRightView = p; }
	CToolBar *GetFormatBar() { return pFormatBar; }
	void SetFormatBar(CToolBar *bp) { pFormatBar = bp; }
	void SetStatusBarText(LPCTSTR text);
	void SetStatusBarText(UINT id);

	// -- functions related to tray icon --
	// add/remove tray icon
	void AddTrayIcon(const CString& tooltip);
	void RemoveTrayIcon();

	// -- do auto save --
	void SaveIfRequired();

	// -- "meta" preferences --
	bool GetPortableMode();
	void SetPortableMode(bool mode);
	void CopyFromRegistry();
	void CopyFromIni();

	// -- preferences --
	// tree color
	void GetTreeColor(bool& sysDefault, COLORREF& foreColor, COLORREF& backColor);
	void SetTreeColor(bool sysDefault, COLORREF foreColor, COLORREF backColor);
	// text color
	void GetTextColor(bool& sysDefault, COLORREF& foreColor, COLORREF& backColor);
	void SetTextColor(bool sysDefault, COLORREF foreColor, COLORREF backColor);
	// tree font
	void GetTreeFont(bool& sysDefault, PreferredFont& font);
	void SetTreeFont(bool sysDefault, const PreferredFont& font);
	// text font
	void GetTextFont(bool& sysDefault, PreferredFont& font);
	void SetTextFont(bool sysDefault, const PreferredFont& font);
	// startup option
	void GetStartupOption(StartupOption& opt);
	void SetStartupOption(StartupOption opt);
	// print template
	void GetPrintTemplate(PrintTemplate& templ, LPCTSTR which = NULL);
	void SetPrintTemplate(const PrintTemplate& templ, LPCTSTR which = NULL);
	// position and sizes
	bool GetMainFrameSize(int& posX, int& posY, int& width, int& height);
	void SetMainFrameSize(int posX, int posY, int width, int height);
	void GetPaneWidths(int& treePaneWidth, int& textPaneWidth);
	void SetPaneWidths(int treePaneWidth, int textPaneWidth);
	// custom colors
	void GetCustomColors(CustomColors& colors);
	void SetCustomColors(const CustomColors& colors);
	// date and time formats
	void GetDateTimeFormat(DateTimeFormat& format);
	void SetDateTimeFormat(const DateTimeFormat& format);
	// minimize to system tray
	bool GetMinimizeToSystemTray();
	void SetMinimizeToSystemTray(bool min);
	// single click restore
	bool GetSingleClickRestore();
	void SetSingleClickRestore(bool singleClick);
	// "esc" minimizes
	bool GetEscMinimize();
	void SetEscMinimize(bool escmin);
	// "x" minimizes
	bool GetXMinimize();
	void SetXMinimize(bool xmin);
	// auto save on close
	bool GetSaveOnClose();
	void SetSaveOnClose(bool save);
	// auto save
	bool GetAutoSave();
	void SetAutoSave(bool save);
	// auto save interval
	unsigned GetAutosaveInterval();
	void SetAutosaveInterval(unsigned minutes);
	// auto backup
	bool GetAutoBackup();
	void SetAutoBackup(bool backup);
	// tree decoration
	TreeDecoration GetTreeDecoration();
	void SetTreeDecoration(TreeDecoration dec);

	// -- resource strings --
	CString GetString(UINT id);

private:
	void PrefHelp_GetColor(LPCTSTR key,
		bool& sysDefault, COLORREF& foreColor, COLORREF& backColor);
	void PrefHelp_SetColor(LPCTSTR key, 
		bool sysDefault, COLORREF foreColor, COLORREF backColor);
	void PrefHelp_GetFont(LPCTSTR key, bool& sysDefault, PreferredFont& font);
	void PrefHelp_SetFont(LPCTSTR key, bool sysDefault, const PreferredFont& font);

	// use ini/reg for portability
	UINT GetProfileIntEx(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault);
	BOOL WriteProfileIntEx(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);
	BOOL WriteProfileStringEx(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
	CString GetProfileStringEx(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = NULL);
	void InitPortableMode();
	void _SetMFCPortableMode();
	void CopyFromRegistryOrIni(bool fromRegistry);

private:
	HWND hLeftWnd;
	HWND hRightWnd;
	HWND hMainWnd;
	CView *pLeftView;
	CView *pRightView;
	CToolBar *pFormatBar;
	bool m_bPortable;
	CString m_strIniFile;

	// cache of resource strings
	CMap<UINT, UINT, CString, LPCTSTR> m_rcStrings;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	afx_msg void OnAppAbout();
	afx_msg void OnHelpKeyboardshortcuts();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnAppExit();
};


// -- print/preview/export of a tree/subtree/single node ----------------------

enum PrintType
{
	PT_ENTIRE_DOCUMENT, 
	PT_ONLY_SELECTED,
	PT_SELECTED_AND_CHILDREN
};

struct PrintItemInfo
{
	void SetPrintType(PrintType pt) { printType = pt; }
	PrintType GetPrintType() const { return printType; }
	HTREEITEM GetSelectedItem() const { return selectedItem; }
	void SetSelectedItem(HTREEITEM item) { selectedItem = item; }

private:
	HTREEITEM selectedItem;
	PrintType printType;
};


// -- global variables --------------------------------------------------------

extern CGuideApp theApp;

extern const char *theEmptyRTFText;
extern const char *theStartupRTFText;

extern PrintItemInfo thePrintItemInfo;


// -- constants ---------------------------------------------------------------

#define WM_GUIDEUI				((WM_USER) + 100)
#define GM_COMMIT_RTF_NOW		1
#define GM_NUMBER_DROP_DOWN		2
// #define GM_CLEAR_NUMBERING	3 (not used anymore)
#define GM_ABOUT_TO_DELETE		4
#define GM_APPLY_FONT			5
#define GM_APPLY_COLOR			6
#define GM_SELECT_NODE			7
#define GM_GET_WINTREE			8
#define GM_SHOW_SEARCH			9
#define GM_NEW_PAGE				10
#define GM_GET_CHILD_COUNT		11
#define GM_TREERMB_PRINT		12
#define GM_PREVIEW_PRINT		13
#define GM_TREERMB_PREVIEW		14
#define GM_PREVIEW_PREVIEW		15
#define GM_GET_LC				16
#define GM_SET_BGCOLOR			17
#define GM_SET_FGCOLOR			18
#define GM_APPLY_AUTOSAVE		19
#define GM_SET_NODE_FGCOLOR		20
#define GM_SET_NODE_BGCOLOR		21
#define GM_GET_SELECTED_NODE	22
#define GM_SELECT_NODE_BY_UID	23
#define GM_ICONWND_DESTROYED	24
#define GM_SET_NODE_ICON		25

// LPARAM for GM_SET_NODE_{F,B}GCOLOR is a pointer to such a
// structure. Callee does not delete.
struct SetNodeColorInput
{
	HTREEITEM hItem;
	COLORREF color;
};

// sent from tray icon window to main window
#define WM_GUIDE_TRAYICON		((WM_USER) + 101)

// the rich edit class name to use
#define RICHEDIT50W				L"RichEdit50W"

// -- global functions --------------------------------------------------------

CStringA GetRTF(HWND hwnd);
void SetRTF(HWND hwnd, const CStringA& rtf, bool fixStyle = true);
void SetRTFSelection(HWND hwnd, const CStringA& rtf);

// -- utility functions -------------------------------------------------------

inline void SetRadioCheck_(CWnd *dlgWnd, UINT id, bool condition)
{
	dlgWnd->GetDlgItem(id)->SendMessage(BM_SETCHECK, condition ? BST_CHECKED : BST_UNCHECKED);
}

#define SetRadioCheck(id, condition)	SetRadioCheck_(this, id, condition)

inline bool IsRadioChecked_(CWnd *dlgWnd, UINT id)
{
	return dlgWnd->GetDlgItem(id)->SendMessage(BM_GETCHECK) == BST_CHECKED;
}

#define IsRadioChecked(id)	IsRadioChecked_(this, id)

// get a cached resource string
#define RCSTR(id)	theApp.GetString( (id) )
