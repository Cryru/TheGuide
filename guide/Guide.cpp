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
#include "MainFrm.h"
#include "GuideDoc.h"
#include "LeftView.h"
#include "KeyboardHelp.h"
#include ".\guide.h"

// registry root
#define RGY_ROOT			_T("MD's Stuff")	// uh, it just happened that way.. ;-)
#define RGY_APPNAME			_T("The Guide")
#define RGY_ROOT_FULL		_T("Software\\") RGY_ROOT _T("\\") RGY_APPNAME

// registry section names
#define	RGY_SECTION			_T("Settings")
#define	RGY_SECTION_TEMPL	_T("Templates")
// registry key/value names
// minimize to tray
#define RGY_MIN_TO_TRAY		_T("MinimizeToTray")
// tree color
#define RGY_TREE_COLOR		_T("TreeColor")
// text color
#define RGY_TEXT_COLOR		_T("TextColor")
// tree font
#define RGY_TREE_FONT		_T("TreeFont")
// text font
#define RGY_TEXT_FONT		_T("TextFont")
// startup opt
#define RGY_STARTUP_OPT		_T("StartupOpt")
// main frame location
#define RGY_MAINFRM_POS		_T("MainFramePos")
// pane widths
#define RGY_PANE_WIDTHS		_T("PaneWidths")
// custom colors
#define RGY_CUSTOM_COLORS	_T("CustomColors")
// date/time formats
#define RGY_DATEFMT_SYS		_T("UseSysDate")
#define RGY_DATEFMT_FMT		_T("DateFormat")
#define RGY_TIMEFMT_SYS		_T("UseSysTime")
#define RGY_TIMEFMT_FMT		_T("TimeFormat")
// esc minimizes
#define RGY_ESC_MIN			_T("EscMinimizes")
// X minimizes
#define RGY_X_MIN			_T("XMinimizes")
// auto save on close
#define RGY_SAVE_ON_CLOSE	_T("SaveOnClose")
// single-click restore
#define RGY_SINGLE_CLKRST	_T("SingleClickRestore")
// auto save
#define RGY_AUTO_SAVE		_T("Autosave")
// auto save interval
#define RGY_AUTO_SAVE_INTERVAL _T("AutosaveInterval")
// auto backup
#define RGY_AUTO_BACKUP		_T("AutoBackup")
// tree decoration
#define RGY_TREE_DECO		_T("TreeDeco")

// default template name
#define RGY_DEFAULT_TEMPL	_T("__default__")

// name of ini file
#define GUIDE_INI			_T("guide.ini")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// -- helper methods ----------------------------------------------------------

static void splitString(const CString& str,
	CStringArray& parts, TCHAR sep = _T('|'))
{
	int pos, ofs = 0;
	while ((pos = str.Find(sep, ofs)) > 0)
	{
		parts.Add(str.Mid(ofs, pos-ofs));
		ofs = pos+1;
	}
	parts.Add(str.Mid(ofs));
}

static bool toString(const HeadingStyle& hs, CString& out)
{
	out.Format(
		_T("%s|%d|%d|%d|%d|%d|%d|%d|%d"),
		hs.fontFamily, hs.fontPointSize,
		hs.fontBold, hs.fontItalic,
		hs.fontUnderline, hs.fontColor,
		hs.breakPage, hs.numbered,
		hs.justify);
	return true;
}

static bool fromString(const CString& str, HeadingStyle& hs)
{
	CStringArray parts;
	splitString(str, parts, _T('|'));
	if (parts.GetCount() != 9)
		return false;

	memset(&hs, 0, sizeof(hs));
	_tcsncpy(hs.fontFamily, parts[0], LF_FACESIZE);
	hs.fontPointSize = _ttoi(parts[1]);
	hs.fontBold      = parts[2] == _T("1");
	hs.fontItalic    = parts[3] == _T("1");
	hs.fontUnderline = parts[4] == _T("1");
	hs.fontColor     = _ttoi(parts[5]);
	hs.breakPage     = parts[6] == _T("1");
	hs.numbered      = parts[7] == _T("1");
	hs.justify       = HeadingStyle::_justify(_ttoi(parts[8]));

	if (hs.fontPointSize < 0 || hs.fontPointSize > 99 ||
		hs.justify < 0 || hs.justify > 2 ||
		hs.fontFamily[0] == 0)
		return false;
	return true;
}

static bool toString(const PrintTemplate& pt, CString& out)
{
	out = _T("1*"); // the '1' indicates version

	for (int i=0; i<5; ++i)
	{
		CString t;
		if (toString(pt.headings[i], t) == false)
			return false;
		if (t.Find(_T('*')) >= 0)
			return false;
		t += _T('*');
		out += t;
	}
	out.Delete(out.GetLength()-1);

	return true;
}

static bool fromString(const CString& str, PrintTemplate& pt)
{
	CStringArray parts;
	splitString(str, parts, _T('*'));
	if (parts.GetCount() != 6)
		return false;

	// it this a compatible version?
	if (parts[0] != _T("1"))
		return false;

	for (int i=0; i<5; ++i)
		if (fromString(parts[i+1], pt.headings[i]) == false)
			return false;

	return true;
}

static bool toString(const CustomColors& custom, CString& out)
{
	TCHAR tmp[32];
	out.Empty();

	for (int i=0; i<16; ++i)
	{
		out += _itot(custom.colors[i], tmp, 10);
		out += _T(",");
	}
	out.Delete(out.GetLength()-1);

	return true;
}

static bool fromString(const CString& str, CustomColors& custom)
{
	CStringArray parts;
	splitString(str, parts, _T(','));
	if (parts.GetCount() != 16)
		return false;

	for (int i=0; i<16; ++i)
		custom.colors[i] = (COLORREF)_ttoi(parts[i]);

	return true;
}

static bool isFontAvailable(LPCTSTR faceName)
{
	HFONT hFont =
		::CreateFont(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, faceName);
	if (hFont)
	{
		DeleteObject(hFont);
		return true;
	}
	return false;
}

// -- HeadingStyle ------------------------------------------------------------

HeadingStyle::HeadingStyle()
	: fontPointSize(120)
	, fontBold(false), fontItalic(false), fontUnderline(false)
	, fontColor(RGB(0,0,0)), breakPage(false), numbered(true)
	, justify(JFY_LEFT)
{
	wcscpy(fontFamily, L"Arial");
}

PrintTemplate::PrintTemplate()
{
	LPCTSTR fontFamily = isFontAvailable(L"Cambria") ? L"Cambria" : L"Trebuchet MS";

	// Office 2007 default theme:
	// Heading 1: Cambria 26pt, Underline, 23/54/93
	// Heading 2: Cambria 14pt, Bold, 54/95/145
	// Heading 3: Cambria 13pt, Bold, 79/129/189
	// Heading 4: Cambria 11pt, Bold, 79/129/189
	// Heading 5: Cambria 11pt, Bold+Italic, 79/129/189

	wcscpy(headings[0].fontFamily, fontFamily);
	headings[0].fontPointSize = 26;
//	headings[0].fontUnderline = true;
	headings[0].numbered = false;
	headings[0].fontColor = RGB(23,54,93);
	headings[0].breakPage = true;

	wcscpy(headings[1].fontFamily, fontFamily);
	headings[1].fontPointSize = 14;
	headings[1].fontBold = true;
	headings[1].fontColor = RGB(54,95,145);

	wcscpy(headings[2].fontFamily, fontFamily);
	headings[2].fontPointSize = 13;
	headings[2].fontBold = true;
	headings[2].fontColor = RGB(79,129,189);

	wcscpy(headings[3].fontFamily, fontFamily);
	headings[3].fontPointSize = 11;
	headings[3].fontBold = true;
	headings[3].fontColor = RGB(79,129,189);

	wcscpy(headings[4].fontFamily, fontFamily);
	headings[4].fontPointSize = 11;
	headings[4].fontBold = true;
	headings[4].fontItalic = true;
	headings[4].fontColor = RGB(79,129,189);
}

// -- Print item --------------------------------------------------------------

PrintItemInfo thePrintItemInfo;

// -- CAboutDlg ---------------------------------------------------------------

class CHeadingStatic : public CStatic
{
private:
	CFont m_Font;

public:
	CHeadingStatic();
	~CHeadingStatic();
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
};

CHeadingStatic::CHeadingStatic()
{
	// calculate font height
	HDC hScreenDC = ::GetDC(NULL);
	LONG lfHeight = 
		-MulDiv(24, GetDeviceCaps(hScreenDC, LOGPIXELSY), 72);
	::ReleaseDC(NULL, hScreenDC);

	m_Font.CreateFont(lfHeight, 0,900,0,0,0,0,0,0,0,0,CLEARTYPE_QUALITY,0, L"Times New Roman");
}

CHeadingStatic::~CHeadingStatic()
{
	m_Font.DeleteObject();
}

void CHeadingStatic::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CFont *pOldFont = pDC->SelectObject(&m_Font);

	pDC->FillSolidRect(&lpDrawItemStruct->rcItem, RGB(0xa8,0xc7,0xef));

	pDC->SetTextColor(RGB(79, 129, 189));

	CSize s = pDC->GetTextExtent(L"The  Guide");
	CRect r(lpDrawItemStruct->rcItem);
	int remainX = r.Width()-s.cy;
	int remainY = r.Height()-s.cx;
	pDC->TextOut(remainX/2, r.bottom-remainY/2, L"The Guide");

	pDC->SelectObject(pOldFont);
}

class CLinkStatic : public CStatic
{
private:
	CFont m_Font;
	HCURSOR m_hCursor;

public:
	CLinkStatic();
	HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	BOOL OnSetCursor(CWnd *, UINT, UINT);
	void OnLButtonUp(UINT, CPoint);
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CLinkStatic, CStatic)
	ON_WM_SETCURSOR()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

CLinkStatic::CLinkStatic()
{
	m_hCursor = theApp.LoadStandardCursor(MAKEINTRESOURCE(32649));
}

BOOL CLinkStatic::OnSetCursor(CWnd *, UINT, UINT)
{
	::SetCursor(m_hCursor);
	return TRUE;
}

void CLinkStatic::OnLButtonUp(UINT, CPoint)
{
	CString myText;
	GetWindowText(myText);

	CWaitCursor cursor;
	ShellExecute(GetParent()->GetSafeHwnd(), _T("open"), myText, NULL, NULL, SW_SHOW);
}

HBRUSH CLinkStatic::CtlColor(CDC* pDC, UINT nCtlColor)
{
	if (m_Font.m_hObject == NULL)
	{
		LOGFONT lf;
		GetFont()->GetObject(sizeof(lf), &lf);
		lf.lfUnderline = TRUE;
		m_Font.CreateFontIndirect(&lf);
	}

	pDC->SelectObject(&m_Font);
	pDC->SetTextColor(RGB(0,0,255));
	pDC->SetBkMode(TRANSPARENT);

	return reinterpret_cast<HBRUSH>(::GetStockObject(HOLLOW_BRUSH));
}

class CAboutDlg : public CDialog
{
public:
	CAboutDlg() : CDialog(IDD_ABOUTBOX) { }
	virtual void DoDataExchange(CDataExchange* pDX);
	CHeadingStatic m_heading;
	CLinkStatic m_link, m_email, m_gug;
};

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ABT_HEADING, m_heading);
	DDX_Control(pDX, IDC_ABT_URL, m_link);
	DDX_Control(pDX, IDC_ABT_EMAIL, m_email);
	DDX_Control(pDX, IDC_ABT_GUG, m_gug);
}

// -- CGuideApp ---------------------------------------------------------------

BEGIN_MESSAGE_MAP(CGuideApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_HELP_KEYBOARDSHORTCUTS, OnHelpKeyboardshortcuts)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
END_MESSAGE_MAP()

CGuideApp::CGuideApp()
	: hLeftWnd(0), hRightWnd(0)
	, hMainWnd(0), pLeftView(0)
	, pRightView(0), pFormatBar(0)
	, m_bPortable(false)
{
	// empty
}

CGuideApp theApp;

BOOL CGuideApp::InitInstance()
{
	InitCommonControls();

	CWinApp::InitInstance();

	// check whether we're to use registry or ini file
	InitPortableMode();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	// Load the rich edit 5.0 library
	if (LoadLibrary(L"msftedit.dll") == NULL)
	{
		AfxMessageBox(IDS_NORICHEDIT5DLL, MB_OK|MB_ICONWARNING);
	}

	// Load standard INI file options (including MRU)
	LoadStdProfileSettings(10);  

	// add the doc template
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CGuideDoc),
		RUNTIME_CLASS(CMainFrame),
		RUNTIME_CLASS(CLeftView));
	if (!pDocTemplate)
		return FALSE;
	pDocTemplate->SetContainerInfo(IDR_CNTR_INPLACE);
	AddDocTemplate(pDocTemplate);

	// donno what this is for
	EnableShellOpen();

	// register shell file types if we're running in non-portable mode
	if (! m_bPortable)
		RegisterShellFileTypes(TRUE);

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();

	m_pMainWnd->DragAcceptFiles();

	// we store the main window handle for use in ExitInstance(),
	// by which time the main window would have got destroyed.
	hMainWnd = m_pMainWnd->GetSafeHwnd();

	// see if we're required to open MRU file on startup
	StartupOption opt;
	GetStartupOption(opt);
	if (__argc == 1 && opt == SO_LAST_FILE)
		OnOpenRecentFile(ID_FILE_MRU_FILE1);

	return TRUE;
}

void CGuideApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CGuideApp::AddTrayIcon(const CString& tooltip)
{
	// load icon
	HICON trayIcon = LoadIcon(IDI_TRAYICON);
	ASSERT(trayIcon != NULL);

	// add the icon to system tray
	NOTIFYICONDATA nid;
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = hMainWnd;
	nid.uID = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_GUIDE_TRAYICON;
	nid.hIcon = trayIcon;
	wcscpy(nid.szTip, tooltip.Left(63));

	Shell_NotifyIcon(NIM_ADD, &nid);
}

void CGuideApp::RemoveTrayIcon()
{
	// remove icon from system tray
	NOTIFYICONDATA nid;
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = hMainWnd;
	nid.uID = 0;

	Shell_NotifyIcon(NIM_DELETE, &nid);
}

int CGuideApp::ExitInstance()
{
	// remove tray icon
	RemoveTrayIcon();

	return CWinApp::ExitInstance();
}

void CGuideApp::OnHelpKeyboardshortcuts()
{
	CKeyboardHelp().DoModal();
}

// -- preferences --

void CGuideApp::PrefHelp_GetColor(LPCTSTR key,
	bool& sysDefault, COLORREF& foreColor, COLORREF& backColor)
{
	sysDefault = true;
	foreColor = RGB(0,0,0);
	backColor = RGB(255,255,255);
	CString val = GetProfileStringEx(RGY_SECTION, key, L"default");

	if (val != L"default")
	{
		CStringArray parts;
		splitString(val, parts);
		if (parts.GetSize() == 3)
		{
			sysDefault = parts[0] == L"1";
			foreColor = _wtoi(parts[1]);
			backColor = _wtoi(parts[2]);
		}
	}
}

void CGuideApp::PrefHelp_SetColor(LPCTSTR key, 
	bool sysDefault, COLORREF foreColor, COLORREF backColor)
{
	CString val;
	val.Format(_T("%d|%u|%u"), sysDefault, foreColor, backColor);
	WriteProfileStringEx(RGY_SECTION, key, val);
}

static bool splitFontString(const CString& str0,
	bool& sysDefault, CString& fontName, int& height, bool& bold,
	bool& italic)
{
	CStringArray parts;
	splitString(str0, parts);
	if (parts.GetSize() != 5)
		return false;

	// sysdefault
	if (parts[0] != _T("0") && parts[0] != _T("1"))
		return false;
	sysDefault = parts[0] == _T("1");

	// font name
	fontName = parts[1];

	// height
	height = _wtoi(parts[2]);
	if (height == 0)
		return false;

	// bold
	if (parts[3] != _T("0") && parts[3] != _T("1"))
		return false;
	bold = parts[3] == _T("1");

	// italics
	if (parts[4] != _T("0") && parts[4] != _T("1"))
		return false;
	italic = parts[4] == _T("1");

	return true;
}

static CString makeFontString(
	bool sysDefault, const CString& fontName,
	int height, bool bold, bool italic)
{
	CString s;
	s.Format(_T("%d|%s|%d|%d|%d"), sysDefault, fontName, height, bold, italic);
	return s;
}

void CGuideApp::PrefHelp_GetFont(LPCTSTR key, 
	bool& sysDefault, PreferredFont& font)
{
	sysDefault = true;
	font = theDefaultFont;
	CString val = GetProfileStringEx(RGY_SECTION, key, _T("default"));

	if (val != _T("default"))
	{
		CString fontName;
		if (splitFontString(val, sysDefault, fontName, font.height,
			font.bold, font.italic))
		{
			_tcsncpy(font.faceName, fontName, LF_FACESIZE);
		}
	}
}

void CGuideApp::PrefHelp_SetFont(LPCTSTR key, bool sysDefault,
	const PreferredFont& font)
{
	WriteProfileStringEx(RGY_SECTION, key, 
		makeFontString(sysDefault, font.faceName, font.height, 
			font.bold, font.italic));
}

void CGuideApp::GetTreeColor(bool& sysDefault, COLORREF& foreColor,
	COLORREF& backColor)
{
	PrefHelp_GetColor(RGY_TREE_COLOR, sysDefault, foreColor, backColor);
}

void CGuideApp::SetTreeColor(bool sysDefault, COLORREF foreColor, COLORREF backColor)
{
	PrefHelp_SetColor(RGY_TREE_COLOR, sysDefault, foreColor, backColor);
}

void CGuideApp::GetTextColor(bool& sysDefault, COLORREF& foreColor, 
	COLORREF& backColor)
{
	PrefHelp_GetColor(RGY_TEXT_COLOR, sysDefault, foreColor, backColor);
}

void CGuideApp::SetTextColor(bool sysDefault, COLORREF foreColor, COLORREF backColor)
{
	PrefHelp_SetColor(RGY_TEXT_COLOR, sysDefault, foreColor, backColor);
}

void CGuideApp::GetTreeFont(bool& sysDefault, PreferredFont& font)
{
	PrefHelp_GetFont(RGY_TREE_FONT, sysDefault, font);
}

void CGuideApp::SetTreeFont(bool sysDefault, const PreferredFont& font)
{
	PrefHelp_SetFont(RGY_TREE_FONT, sysDefault, font);
}

void CGuideApp::GetTextFont(bool& sysDefault, PreferredFont& font)
{
	PrefHelp_GetFont(RGY_TEXT_FONT, sysDefault, font);
}

void CGuideApp::SetTextFont(bool sysDefault, const PreferredFont& font)
{
	PrefHelp_SetFont(RGY_TEXT_FONT, sysDefault, font);
}

void CGuideApp::GetStartupOption(StartupOption& opt)
{
	UINT val = GetProfileIntEx(RGY_SECTION, RGY_STARTUP_OPT, 0);
	if (val > 1) val = 0;
	opt = StartupOption(val);
}

void CGuideApp::SetStartupOption(StartupOption opt)
{
	UINT val = UINT(opt);
	ASSERT(val == 0 || val == 1);
	WriteProfileIntEx(RGY_SECTION, RGY_STARTUP_OPT, val);
}

void CGuideApp::SetPrintTemplate(const PrintTemplate& templ, LPCTSTR which)
{
	LPCTSTR which2 = which ? which : RGY_DEFAULT_TEMPL;
	
	CString value;
	if (toString(templ, value))
		WriteProfileStringEx(RGY_SECTION_TEMPL, which2, value);
}

void CGuideApp::GetPrintTemplate(PrintTemplate& templ, LPCTSTR which)
{
	LPCTSTR which2 = which ? which : RGY_DEFAULT_TEMPL;

	CString value = GetProfileStringEx(RGY_SECTION_TEMPL, which2);
	if (fromString(value, templ) == false)
		templ = PrintTemplate();
}

bool CGuideApp::GetMainFrameSize(int& posX, int& posY, int& width, int& height)
{
	int _posX   = GetProfileIntEx(RGY_SECTION, RGY_MAINFRM_POS _T("X"), -1);
	int _posY   = GetProfileIntEx(RGY_SECTION, RGY_MAINFRM_POS _T("Y"), -1);
	int _width  = GetProfileIntEx(RGY_SECTION, RGY_MAINFRM_POS _T("W"), -1);
	int _height = GetProfileIntEx(RGY_SECTION, RGY_MAINFRM_POS _T("H"), -1);

	if (_posX == -1 || _posY == -1 || _width == -1 || _height == -1)
		return false;

	// v1.6: sanity check. In v1.5., if you closed the app when it was
	// minimized, the invalid values were written into the rgy.
	if (_posX < 0 || _posY < 0 || _width < 0 || _height < 0)
		return false;

	posX = _posX;
	posY = _posY;
	width = _width;
	height = _height;
	return true;
}

void CGuideApp::SetMainFrameSize(int posX, int posY, int width, int height)
{
	WriteProfileIntEx(RGY_SECTION, RGY_MAINFRM_POS _T("X"), posX);
	WriteProfileIntEx(RGY_SECTION, RGY_MAINFRM_POS _T("Y"), posY);
	WriteProfileIntEx(RGY_SECTION, RGY_MAINFRM_POS _T("W"), width);
	WriteProfileIntEx(RGY_SECTION, RGY_MAINFRM_POS _T("H"), height);
}

void CGuideApp::GetPaneWidths(int& treePaneWidth, int& textPaneWidth)
{
	treePaneWidth = GetProfileIntEx(RGY_SECTION, RGY_PANE_WIDTHS _T("0"), 200);
	textPaneWidth = GetProfileIntEx(RGY_SECTION, RGY_PANE_WIDTHS _T("1"), 100);

	// v1.6: sanity check
	if (treePaneWidth <= 0 || textPaneWidth <= 0)
	{
		treePaneWidth = 200;
		textPaneWidth = 100;
	}
}

void CGuideApp::SetPaneWidths(int treePaneWidth, int textPaneWidth)
{
	WriteProfileIntEx(RGY_SECTION, RGY_PANE_WIDTHS _T("0"), treePaneWidth);
	WriteProfileIntEx(RGY_SECTION, RGY_PANE_WIDTHS _T("1"), textPaneWidth);
}

static const CustomColors _defaultCustomColor = { {
	0xffffff, 0xffffff, 0xffffff, 0xffffff,
	0xffffff, 0xffffff, 0xffffff, 0xffffff,
	0xffffff, 0xffffff, 0xffffff, 0xffffff,
	0xffffff, 0xffffff, 0xffffff, 0xffffff,
} };

void CGuideApp::GetCustomColors(CustomColors& colors)
{
	CString szClr = GetProfileStringEx(RGY_SECTION, RGY_CUSTOM_COLORS, _T(""));
	if (szClr.GetLength() == 0 || fromString(szClr, colors) == false)
		colors = _defaultCustomColor;
}

void CGuideApp::SetCustomColors(const CustomColors& colors)
{
	CString szClr;
	toString(colors, szClr);
	WriteProfileStringEx(RGY_SECTION, RGY_CUSTOM_COLORS, szClr);
}

void CGuideApp::GetDateTimeFormat(DateTimeFormat& format)
{
	format.useSystemDateFormat = GetProfileIntEx(RGY_SECTION, RGY_DATEFMT_SYS, 1) != 0;
	format.dateFormat = GetProfileStringEx(RGY_SECTION, RGY_DATEFMT_FMT, _T(""));
	format.useSystemTimeFormat = GetProfileIntEx(RGY_SECTION, RGY_TIMEFMT_SYS, 1) != 0;
	format.timeFormat = GetProfileStringEx(RGY_SECTION, RGY_TIMEFMT_FMT, _T(""));

	if (format.dateFormat.GetLength() == 0)
	{
		TCHAR value[81] = {0}; // MSDN says max 80
		::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, value, sizeof(value)/sizeof(*value));
		format.dateFormat = value;
	}

	if (format.timeFormat.GetLength() == 0)
	{
		TCHAR value[81] = {0}; // MSDN says max 80
		::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, value, sizeof(value)/sizeof(*value));
		format.timeFormat = value;
	}
}

void CGuideApp::SetDateTimeFormat(const DateTimeFormat& format)
{
	WriteProfileIntEx(RGY_SECTION, RGY_DATEFMT_SYS, format.useSystemDateFormat);
	WriteProfileStringEx(RGY_SECTION, RGY_DATEFMT_FMT, format.dateFormat);
	WriteProfileIntEx(RGY_SECTION, RGY_TIMEFMT_SYS, format.useSystemTimeFormat);
	WriteProfileStringEx(RGY_SECTION, RGY_TIMEFMT_FMT, format.timeFormat);
}

bool CGuideApp::GetMinimizeToSystemTray()
{
	return GetProfileIntEx(RGY_SECTION, RGY_MIN_TO_TRAY, 0) != 0;
}

void CGuideApp::SetMinimizeToSystemTray(bool min)
{
	WriteProfileIntEx(RGY_SECTION, RGY_MIN_TO_TRAY, min);
}

bool CGuideApp::GetEscMinimize()
{
	return GetProfileIntEx(RGY_SECTION, RGY_ESC_MIN, 0) != 0;
}

void CGuideApp::SetEscMinimize(bool escmin)
{
	WriteProfileIntEx(RGY_SECTION, RGY_ESC_MIN, escmin);
}

bool CGuideApp::GetXMinimize()
{
	return GetProfileIntEx(RGY_SECTION, RGY_X_MIN, 0) != 0;
}

void CGuideApp::SetXMinimize(bool xmin)
{
	WriteProfileIntEx(RGY_SECTION, RGY_X_MIN, xmin);
}

bool CGuideApp::GetSaveOnClose()
{
	return GetProfileIntEx(RGY_SECTION, RGY_SAVE_ON_CLOSE, 0) != 0;
}

void CGuideApp::SetSaveOnClose(bool save)
{
	WriteProfileIntEx(RGY_SECTION, RGY_SAVE_ON_CLOSE, save);
}

bool CGuideApp::GetSingleClickRestore()
{
	return GetProfileIntEx(RGY_SECTION, RGY_SINGLE_CLKRST, 0) != 0;
}

void CGuideApp::SetSingleClickRestore(bool singleClick)
{
	WriteProfileIntEx(RGY_SECTION, RGY_SINGLE_CLKRST, singleClick);
}

bool CGuideApp::GetAutoSave()
{
	return GetProfileIntEx(RGY_SECTION, RGY_AUTO_SAVE, 0) != 0;
}

void CGuideApp::SetAutoSave(bool save)
{
	WriteProfileIntEx(RGY_SECTION, RGY_AUTO_SAVE, save);
}

unsigned CGuideApp::GetAutosaveInterval()
{
	return (unsigned) GetProfileIntEx(RGY_SECTION, RGY_AUTO_SAVE_INTERVAL, 0);
}

void CGuideApp::SetAutosaveInterval(unsigned minutes)
{
	WriteProfileIntEx(RGY_SECTION, RGY_AUTO_SAVE_INTERVAL, minutes);
}

bool CGuideApp::GetAutoBackup()
{
	return GetProfileIntEx(RGY_SECTION, RGY_AUTO_BACKUP, 0) != 0;
}

void CGuideApp::SetAutoBackup(bool backup)
{
	WriteProfileIntEx(RGY_SECTION, RGY_AUTO_BACKUP, backup);
}

TreeDecoration CGuideApp::GetTreeDecoration()
{
	int dec = GetProfileIntEx(RGY_SECTION, RGY_TREE_DECO, 1);
	if (dec < 0 || dec > 3)
		dec = 1;

	return TreeDecoration(dec);
}

void CGuideApp::SetTreeDecoration(TreeDecoration dec)
{
	ASSERT(dec >= 0 && dec <= 3);
	WriteProfileIntEx(RGY_SECTION, RGY_TREE_DECO, dec);
}

//-----------------------------------------------------------------------------------

void CGuideApp::SetStatusBarText(LPCTSTR text)
{
	CMainFrame *pMainFrame = (CMainFrame *)m_pMainWnd;
	pMainFrame->SetStatusBarText(text);
}

void CGuideApp::SetStatusBarText(UINT id)
{
	SetStatusBarText(RCSTR(id));
}

void CGuideApp::SaveIfRequired()
{
	// check if document is not 'untitled' and is modified
	CFrameWnd *pFrame = (CFrameWnd *)m_pMainWnd;
	CDocument *pDoc = pFrame ? pFrame->GetActiveDocument() : NULL;

	if (pFrame && pDoc && pDoc->GetPathName().IsEmpty() == FALSE && pDoc->IsModified())
	{
		// ok, really save
		((CGuideDoc *)pDoc)->OnFileSave();
	}
}

void CGuideApp::OnFileNew()
{
	// save if 'auto save on close' is set
	if (GetSaveOnClose())
		SaveIfRequired();

	CWinApp::OnFileNew();
}

void CGuideApp::OnFileOpen()
{
	// save if 'auto save on close' is set
	if (GetSaveOnClose())
		SaveIfRequired();

	CWinApp::OnFileOpen();
}

void CGuideApp::OnAppExit()
{
	// save if 'auto save on close' is set
	if (GetSaveOnClose())
		SaveIfRequired();

	CWinApp::OnAppExit();
}

//-----------------------------------------------------------------------------------

// See MFC source appui3.cpp CWinApp::GetAppRegistryKey().
// Returns key for:
//		HKEY_CURRENT_USER\"Software"\RegistryKey\ProfileName
// creating it if it doesn't exist. Caller should RegCloseKey() on the 
// returned HKEY.
static HKEY _GetAppRegistryKey(LPCTSTR companyName, LPCTSTR appName)
{
	HKEY hAppKey = NULL;
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("software"), 0, KEY_WRITE|KEY_READ,
		&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if (RegCreateKeyEx(hSoftKey, companyName, 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			RegCreateKeyEx(hCompanyKey, appName, 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
				&hAppKey, &dw);
		}
	}
	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		RegCloseKey(hCompanyKey);

	return hAppKey;
}

// See MFC source appui3.cpp CWinApp::GetAppRegistryKey().
// Returns key for:
//      HKEY_CURRENT_USER\"Software"\RegistryKey\AppName\lpszSection
// Creating it if it doesn't exist. Caller should RegCloseKey() on the 
// returned HKEY.
static HKEY _GetSectionKey(LPCTSTR companyName, LPCTSTR appName, 
						   LPCTSTR lpszSection)
{
	HKEY hAppKey = _GetAppRegistryKey(companyName, appName);
	if (hAppKey == NULL)
		return NULL;

	DWORD dw;
	HKEY hSectionKey = NULL;
	RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE,
		REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
		&hSectionKey, &dw);
	RegCloseKey(hAppKey);
	return hSectionKey;
}

static BOOL _GetRegistryString(LPCTSTR keyName, LPCTSTR valueName, 
							   CString& data_out)
{
	HKEY hKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, keyName, 0, KEY_READ, &hKey) 
		!= ERROR_SUCCESS)
		return FALSE;

	BOOL ret = FALSE;
	TCHAR data[10240 + 1] = {0};
	DWORD dataLen = sizeof(data);
	if (RegQueryValueEx(hKey, valueName, NULL, NULL, (LPBYTE)data, &dataLen) 
		== ERROR_SUCCESS)
	{
		ret = TRUE;
		data_out = data;
	}

	RegCloseKey(hKey);
	return ret;
}

static BOOL _GetRegistryDword(LPCTSTR keyName, LPCTSTR valueName, DWORD& data_out)
{
	HKEY hKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, keyName, 0, KEY_READ, &hKey) 
		!= ERROR_SUCCESS)
		return FALSE;

	BOOL ret = FALSE;
	DWORD data = 0;
	DWORD dataLen = sizeof(data);
	if (RegQueryValueEx(hKey, valueName, NULL, NULL, (LPBYTE)&data, &dataLen) 
		== ERROR_SUCCESS)
	{
		ret = TRUE;
		data_out = data;
	}

	RegCloseKey(hKey);
	return ret;
}

static BOOL _SetRegistryString(LPCTSTR section, LPCTSTR valueName, LPCTSTR data)
{
	BOOL ret = FALSE;

	// open section key
	HKEY hKey = NULL;
	if ((hKey = _GetSectionKey(RGY_ROOT, RGY_APPNAME, section)) != NULL)
	{
		// set value
		if (RegSetValueEx(hKey, valueName, 0, REG_SZ, (LPBYTE)data, 
			(DWORD)(_tcslen(data)+1)*2) == ERROR_SUCCESS)
			ret = TRUE;
		RegCloseKey(hKey);
	}

	return ret;
}

static BOOL _SetRegistryDword(LPCTSTR section, LPCTSTR valueName, DWORD data)
{
	BOOL ret = FALSE;

	// open section key
	HKEY hKey = NULL;
	if ((hKey = _GetSectionKey(RGY_ROOT, RGY_APPNAME, section)) != NULL)
	{
		// set value
		if (RegSetValueEx(hKey, valueName, 0, REG_DWORD, (LPBYTE)&data, 
			sizeof(data)) == ERROR_SUCCESS)
			ret = TRUE;
		RegCloseKey(hKey);
	}

	return ret;
}

static CString escape(LPCTSTR lpszValue)
{
	// escape unicode characters before writing to file
	CString escaped;
	size_t len = _tcslen(lpszValue);
	TCHAR tmp[10];
	for (size_t i=0; i<len; ++i)
	{
		TCHAR ch = lpszValue[i];
		if (ch < 32 || ch > 127 || ch == _T('\\'))
		{
			_stprintf(tmp, _T("\\u%04x"), ch);
			escaped += tmp;
		}
		else
		{
			escaped += ch;
		}
	}

	return escaped;
}

static CString unescape(LPTSTR lpszValue)
{
	// unescape the unicode characters before passing back to app
	CString unescape;
	LPTSTR begin = lpszValue, pos = lpszValue;
	while ((pos = _tcsstr(begin, _T("\\u"))) != NULL)
	{
		// first check if there are 4 more chars and that they're valid
		if (_istxdigit(pos[2]) && _istxdigit(pos[3]) &&
			_istxdigit(pos[4]) && _istxdigit(pos[5]))
		{
			*pos = 0;
			unescape += begin;
			TCHAR digits[5] = { pos[2], pos[3], pos[4], pos[5], 0 };
			unescape += (TCHAR) _tcstol(digits, NULL, 16);
			begin = pos+6;
		}
		else
		{
			TCHAR ch = pos[2];
			pos[2] = 0;
			unescape += begin;
			pos[2] = ch;
			begin = pos+2;
		}
	}
	if (pos == NULL)
		unescape += begin;

	return unescape;
}

static inline void _makeKeyName(LPTSTR keyNameBuf, LPCTSTR lpszSection)
{
	_stprintf(keyNameBuf, _T("%s\\%s"), RGY_ROOT_FULL, lpszSection);
}

BOOL CGuideApp::WriteProfileStringEx(LPCTSTR lpszSection, LPCTSTR lpszEntry,
									 LPCTSTR lpszValue)
{
	if (m_bPortable)
	{
		CString escaped = escape(lpszValue);
		return WritePrivateProfileString(lpszSection, lpszEntry, escaped, 
			m_strIniFile);
	}
	else
	{
		return _SetRegistryString(lpszSection, lpszEntry, lpszValue);
	}
}

CString CGuideApp::GetProfileStringEx(LPCTSTR lpszSection, LPCTSTR lpszEntry,
									  LPCTSTR lpszDefault)
{
	if (m_bPortable)
	{
		TCHAR buf[1024 + 1] = {0}; // intentionally limited to max 1k
		::GetPrivateProfileString(lpszSection, lpszEntry, lpszDefault, buf, 
			sizeof(buf)/sizeof(*buf), m_strIniFile);
		return unescape(buf);
	}
	else
	{
		TCHAR keyName[1024 + 1] = {0};
		_makeKeyName(keyName, lpszSection);

		CString result;
		return _GetRegistryString(keyName, lpszEntry, result) ? result : lpszDefault;
	}
}

UINT CGuideApp::GetProfileIntEx(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
	if (m_bPortable)
	{
		return ::GetPrivateProfileInt(lpszSection, lpszEntry, nDefault, m_strIniFile);
	}
	else
	{
		TCHAR keyName[1024 + 1] = {0};
		_makeKeyName(keyName, lpszSection);

		DWORD result = 0;
		return _GetRegistryDword(keyName, lpszEntry, result) ? result : nDefault;
	}
}

BOOL _WritePrivateProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue, LPCTSTR iniFile)
{
	TCHAR tmp[32];
	_stprintf(tmp, _T("%u"), nValue);
	return WritePrivateProfileString(lpszSection, lpszEntry, tmp, iniFile);
}

BOOL CGuideApp::WriteProfileIntEx(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
	if (m_bPortable)
	{
		return _WritePrivateProfileInt(lpszSection, lpszEntry, nValue, m_strIniFile);
	}
	else
	{
		return _SetRegistryDword(lpszSection, lpszEntry, (DWORD)nValue);
	}
}

//-----------------------------------------------------------------------------------

void CGuideApp::InitPortableMode()
{
	m_bPortable = false;

	// form the path to the .ini file
	m_strIniFile = _tpgmptr;
	int nSlashPos = m_strIniFile.ReverseFind(L'\\');
	if (nSlashPos == -1)
		return; // bah?
	m_strIniFile.Delete(nSlashPos, m_strIniFile.GetLength()-nSlashPos);
	m_strIniFile += _T('\\');
	m_strIniFile += GUIDE_INI;

	// try to read the mode from the ini file
	if (::GetPrivateProfileInt(_T("main"), _T("portable"), 0, m_strIniFile) != 0)
		m_bPortable = true;

	// tell mfc
	_SetMFCPortableMode();
}

// tell MFC if we're using registry or ini file
void CGuideApp::_SetMFCPortableMode()
{
	if (m_bPortable)
	{
		m_pszProfileName = _tcsdup(m_strIniFile);
	}
	else
	{
		SetRegistryKey(RGY_ROOT);
	}
}

bool CGuideApp::GetPortableMode()
{
	return m_bPortable;
}

void CGuideApp::SetPortableMode(bool makePortable)
{
	m_bPortable = makePortable;

	// update ini file
	::WritePrivateProfileString(_T("main"), _T("portable"), 
		makePortable ? _T("1") : _T("0"), m_strIniFile);
}

void CGuideApp::CopyFromRegistry()
{
	HKEY hKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, RGY_ROOT_FULL, 0,
			KEY_ENUMERATE_SUB_KEYS|KEY_READ|KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
		return;

	DWORD nIndex = 0;
	while (1)
	{
		TCHAR keyName[255 + 1];
		DWORD keyNameLen = sizeof(keyName)/sizeof(*keyName);
		LONG ret = RegEnumKeyEx(hKey, nIndex, keyName, &keyNameLen, NULL, NULL, NULL, NULL);
		if (ret != ERROR_SUCCESS)
			break;

		CString subKey = RGY_ROOT_FULL; subKey += _T('\\'); subKey += keyName;
		HKEY hSubKey = NULL;
		if (RegOpenKeyEx(HKEY_CURRENT_USER, subKey, 0, 
				KEY_ENUMERATE_SUB_KEYS|KEY_READ|KEY_QUERY_VALUE, &hSubKey) == ERROR_SUCCESS)
		{
			DWORD nSubIndex = 0;
			while (1)
			{
				TCHAR valueName[255 + 1]; // actually should be 16383+1 but is it really reqd?
				DWORD valueNameLen = sizeof(valueName)/sizeof(*valueName);
				DWORD valueType;
				unsigned char valueValue[20480 + 1] = {0}; // need 0-fill
				DWORD valueValueLen = sizeof(valueValue);
				if (RegEnumValue(hSubKey, nSubIndex, valueName, &valueNameLen, NULL, &valueType,
						valueValue, &valueValueLen) != ERROR_SUCCESS)
					break;

				if (valueType == REG_SZ)
					WritePrivateProfileString(keyName, valueName, (LPCTSTR)valueValue, m_strIniFile);
				else if (valueType == REG_DWORD)
					_WritePrivateProfileInt(keyName, valueName, *(int *)valueValue, m_strIniFile);

				++nSubIndex;
			}
			RegCloseKey(hSubKey);
		}

		++nIndex;
	}

	RegCloseKey(hKey);
}

void CGuideApp::CopyFromIni()
{
	// save original current location
	bool origMode = m_bPortable;

	// pretend we're using location1
	m_bPortable = true;

	// get everything from location1
	//..
	bool treeColorSysDefault; COLORREF treeColorFore, treeColorBack;
	GetTreeColor(treeColorSysDefault, treeColorFore, treeColorBack);
	//..
	bool textColorSysDefault; COLORREF textColorFore, textColorBack;
	GetTextColor(textColorSysDefault, textColorFore, textColorBack);
	//..
	bool treeFontSysDefault; PreferredFont treeFont;
	GetTreeFont(treeFontSysDefault, treeFont);
	//..
	bool textFontSysDefault; PreferredFont textFont;
	GetTextFont(textFontSysDefault, textFont);
	//..
	StartupOption startupOpt;
	GetStartupOption(startupOpt);
	//..
	PrintTemplate printTempl;
	GetPrintTemplate(printTempl);
	//..
	int mfX, mfY, mfW, mfH;
	bool mfValid = GetMainFrameSize(mfX, mfY, mfW, mfH);
	//..
	int treePW, textPW;
	GetPaneWidths(treePW, textPW);
	//..
	CustomColors customColors;
	GetCustomColors(customColors);
	//..
	DateTimeFormat dtFormat;
	GetDateTimeFormat(dtFormat);
	//..
	bool minToTray;
	minToTray = GetMinimizeToSystemTray();
	//..
	bool singleClickRestore;
	singleClickRestore = GetSingleClickRestore();
	//..
	bool escMinimize;
	escMinimize = GetEscMinimize();
	//..
	bool xMinimize;
	xMinimize = GetXMinimize();
	//..
	bool saveOnClose;
	saveOnClose = GetSaveOnClose();
	//..
	bool autoSave;
	autoSave = GetAutoSave();
	//..
	unsigned autoSaveInterval;
	autoSaveInterval = GetAutosaveInterval();
	//..
	bool autoBackup;
	autoBackup = GetAutoBackup();

	// pretend we're using location2
	m_bPortable = false;

	// set everything into location2
	SetTreeColor(treeColorSysDefault, treeColorFore, treeColorBack);
	SetTextColor(textColorSysDefault, textColorFore, textColorBack);
	SetTreeFont(treeFontSysDefault, treeFont);
	SetTextFont(textFontSysDefault, textFont);
	SetStartupOption(startupOpt);
	SetPrintTemplate(printTempl);
	if (mfValid) SetMainFrameSize(mfX, mfY, mfW, mfH);
	SetPaneWidths(treePW, textPW);
	SetCustomColors(customColors);
	SetDateTimeFormat(dtFormat);
	SetMinimizeToSystemTray(minToTray);
	SetSingleClickRestore(singleClickRestore);
	SetEscMinimize(escMinimize);
	SetXMinimize(xMinimize);
	SetSaveOnClose(saveOnClose);
	SetAutoSave(autoSave);
	SetAutosaveInterval(autoSaveInterval);
	SetAutoBackup(autoBackup);

	// restore original current location
	m_bPortable = origMode;
}

CString CGuideApp::GetString(UINT id)
{
	CString val;

	if (m_rcStrings.Lookup(id, val) == FALSE)
	{
		val.LoadString(id);
		m_rcStrings[id] = val;
	}

	return val;
}
