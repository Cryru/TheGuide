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
#include <winspool.h>			// for GetDefaultPrinter()
#include <io.h>					// for _tchmod
#include <sys/stat.h>			// for _S_IREAD, _S_IWRITE
#include "Guide.h"
#include "GuideDoc.h"
#include "CntrItem.h"
#include "FormatBar.h"
#include "Utils.h"
#include "GuidePreview.h"
#include "GuidePrintDlg.h"
#include "ExportFileDialog.h"
#include "HyperLinkDialog.h"
#include <libguide/wintree.h>

#include ".\guideview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// the "system default" is set here -- Arial 9pt regular.
const PreferredFont theDefaultFont =
	{ _T("Arial"), 9, false, false };

// the RTF for an empty rich text window.
const char *theEmptyRTFText = "{\\rtf1}";

const char *theStartupRTFText = 
"{\\rtf1\\ansi Right-click on the left pane and add a page to start editing.\\par}";

enum {
	_PFN_ARABIC = 2,		_PFN_LOWER_LETTERS = 3,
	_PFN_UPPER_LETTERS = 4,	_PFN_LOWER_ROMAN = 5,
	_PFN_UPPER_ROMAN = 6
};

static const int marginSize = 10;

void CRightViewColorDialog::OnSelectColor(COLORREF selectedColor)
{
	theApp.GetRightView()->SendMessage(WM_GUIDEUI, m_wParam, selectedColor);
}

IMPLEMENT_DYNCREATE(CGuideView, CRichEditView)

BEGIN_MESSAGE_MAP(CGuideView, CRichEditView)
	ON_WM_DESTROY()
	// formatting
	ON_COMMAND(ID_FORMAT_BOLD, OnCharBold)
	ON_UPDATE_COMMAND_UI(ID_FORMAT_BOLD, OnUpdateCharBold)
	ON_COMMAND(ID_FORMAT_ITALIC, OnCharItalic)
	ON_UPDATE_COMMAND_UI(ID_FORMAT_ITALIC, OnUpdateCharItalic)
	ON_COMMAND(ID_FORMAT_UNDERLINE, OnCharUnderline)
	ON_UPDATE_COMMAND_UI(ID_FORMAT_UNDERLINE, OnUpdateCharUnderline)
	ON_COMMAND(ID_FORMAT_LEFTJUSTIFIED, OnParaLeft)
	ON_UPDATE_COMMAND_UI(ID_FORMAT_LEFTJUSTIFIED, OnUpdateParaLeft)
	ON_COMMAND(ID_FORMAT_RIGHTJUSTIFIED, OnParaRight)
	ON_UPDATE_COMMAND_UI(ID_FORMAT_RIGHTJUSTIFIED, OnUpdateParaRight)
	ON_COMMAND(ID_FORMAT_CENTERJUSTIFIED, OnParaCenter)
	ON_UPDATE_COMMAND_UI(ID_FORMAT_CENTERJUSTIFIED, OnUpdateParaCenter)
	ON_COMMAND(ID_FORMAT_BULLETED, CRichEditView::OnBullet)
	ON_UPDATE_COMMAND_UI(ID_FORMAT_BULLETED, OnUpdateBullet)
	ON_COMMAND(ID_FORMAT_NUMBERED, OnNumber)
	ON_UPDATE_COMMAND_UI(ID_FORMAT_NUMBERED, OnUpdateNumber)
	// colors
	ON_COMMAND(ID_FORMAT_BACKGROUNDCOLOUR, OnBackgroundColor)
	ON_COMMAND(ID_FORMAT_FOREGROUNDCOLOUR, OnForegroundColor)
	// strikethrough
	ON_COMMAND(ID_FORMAT_STRIKETHROUGH, OnFormatStrikethrough)
	ON_UPDATE_COMMAND_UI(ID_FORMAT_STRIKETHROUGH, OnUpdateFormatStrikethrough)
	// sent by the formatting toolbar (font name and size combos)
	ON_NOTIFY(FN_GETFORMAT, ID_VIEW_FORMATBAR, OnGetCharFormat)
	ON_NOTIFY(FN_SETFORMAT, ID_VIEW_FORMATBAR, OnSetCharFormat)
	// Our own messages
	ON_MESSAGE(WM_GUIDEUI, OnGuideUI)
	// number formats
	ON_COMMAND(ID_FMTNUM_ARABIC, OnFmtnumArabic)
	ON_COMMAND(ID_FMTNUM_LOWER_ALPHA, OnFmtnumLowerAlpha)
	ON_COMMAND(ID_FMTNUM_LOWER_ROMAN, OnFmtnumLowerRoman)
	ON_COMMAND(ID_FMTNUM_UPPER_ALPHA, OnFmtnumUpperAlpha)
	ON_COMMAND(ID_FMTNUM_UPPER_ROMAN, OnFmtnumUpperRoman)
	// linespacing
	ON_COMMAND(ID_FORMAT_LINESPACING, OnLineSpacing)
	ON_COMMAND(ID_FORMAT_LINESPC_ONE, OnFormatLinespcOne)
	ON_COMMAND(ID_FORMAT_LINESPC_ONEHALF, OnFormatLinespcOnehalf)
	ON_COMMAND(ID_FORMAT_LINESPC_TWO, OnFormatLinespcTwo)
	//
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_FORMAT_FONT, OnFormatFont)
	//
	ON_NOTIFY_REFLECT(EN_LINK, OnEnLink)
	ON_NOTIFY_REFLECT(EN_MSGFILTER, OnEnMsgfilter)
	ON_COMMAND(ID_FORMAT_HYPERLINK, OnFormatHyperlink)
	// indent
	ON_COMMAND(ID_FORMAT_DECINDENT, OnFormatDecindent)
	ON_COMMAND(ID_FORMAT_INCINDENT, OnFormatIncindent)
	// export
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	// printing support
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrint) // does not work yet (RMB on .gde file -> print)
	ON_COMMAND(ID_EDIT_INSERTDATE, OnEditInsertdate)
	ON_COMMAND(ID_EDIT_INSERTTIME, OnEditInserttime)
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_EDIT_PASTEASTEXT, OnEditPasteastext)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTEASTEXT, OnUpdateEditPasteastext)
END_MESSAGE_MAP()

CGuideView::CGuideView()
	: m_fgColorDialog(GM_SET_FGCOLOR)
	, m_bgColorDialog(GM_SET_BGCOLOR)
	, wLastNumbering(_PFN_ARABIC)
	, m_pRTFConcatenator(0)
	, m_bClearStatusBar(false)
{
	m_nBulletIndent = 360;
// TODO: Provide this as a config option.
//	m_nWordWrap = WrapNone;

	// Load the RMB menu
	m_RMBMenu.LoadMenu(IDR_RMB);
}

CGuideView::~CGuideView()
{
	// Destroy the RMB menu
	m_RMBMenu.DestroyMenu();
}

BOOL CGuideView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.lpszClass = NULL;
	m_strClass = RICHEDIT50W;
	return CRichEditView::PreCreateWindow(cs);
}

void CGuideView::OnInitialUpdate()
{
	CRichEditCtrl& theCtrl = GetRichEditCtrl();
	CRichEditView::OnInitialUpdate();

	// Set the printing margins (1440 twips = 1 inch)
	SetMargins(CRect(1440, 1440, 1440, 1440));

	// v1.4: Add some decent margins for on-screen display
	CRect r;
	GetClientRect(&r);
	r.DeflateRect(marginSize, marginSize, marginSize, marginSize);
	SendMessage(EM_SETRECT, 0, (LPARAM)&r);

	// Set required color and font
	SetUserPreferences();

	// Tell the app about us
	theApp.SetRightWnd(theCtrl.m_hWnd);
	theApp.SetRightView(this);

	// Enable/disable right view depending on item count
	UpdateEnabledState();

	// Clear the RichEdit control's modified flag, and also the
	// document's modified flag.
	GetDocument()->SetModifiedFlag(FALSE);
	theCtrl.SetModify(FALSE);
	theCtrl.GetModify();

	// Enable EN_LINK and other notifications
	theCtrl.SetEventMask(theCtrl.GetEventMask() | ENM_LINK | ENM_KEYEVENTS);

	// Turn off the IME_DUALFONT option of the control
	LRESULT opts = theCtrl.SendMessage(EM_GETLANGOPTIONS);
	opts &= ~(IMF_DUALFONT);
	theCtrl.SendMessage(EM_SETLANGOPTIONS, 0, opts);
}

static bool hasPrinters()
{
	DWORD nReq = 0;
	::GetDefaultPrinter(NULL, &nReq);
	return nReq != 0;
}

BOOL CGuideView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// if no printers are installed, abort the operation.
	if (!hasPrinters())
	{
		AfxMessageBox(IDS_NO_PRINTERS, MB_ICONSTOP);
		return FALSE;
	}

	// Delete the CPrintDialog object created in the CPrintInfo
	// constructor, and substitute with customized print dialog.
	delete pInfo->m_pPD;

	// Construct and substitute with customized print dialog.
	pInfo->m_pPD = new CGuidePrintDlg(FALSE);

	// Set the page range.
	pInfo->m_pPD->m_pd.nMinPage = 1;      // one based page numbers.
	pInfo->m_pPD->m_pd.nMaxPage = 0xffff; // how many pages is unknown.

	// Change the PRINTDLG struct so that the custom print dialog box will
	// be used.
	pInfo->m_pPD->m_pd.hInstance = AfxGetInstanceHandle();
	pInfo->m_pPD->m_pd.lpPrintTemplateName =
										MAKEINTRESOURCE(1538);

	// Set the Flags of the PRINTDLG structure as shown, else the
	// changes will have no effect.
	pInfo->m_pPD->m_pd.Flags |= PD_ALLPAGES|PD_ENABLEPRINTTEMPLATE|PD_NOSELECTION;
	pInfo->m_pPD->m_pd.Flags &= ~( PD_NOPAGENUMS );

	// For details about these flags, refer to the SDK documentation
	// on the PRINTDLG structure.

	return DoPreparePrinting(pInfo);
}

void CGuideView::OnDestroy()
{
	// Deactivate the item on destruction; this is important
	// when a splitter view is being used
   COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
   if (pActiveItem != NULL && pActiveItem->GetActiveView() == this)
   {
      pActiveItem->Deactivate();
      ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
   }
   CRichEditView::OnDestroy();
}

void CGuideView::OnGetCharFormat(NMHDR* pNMHDR, LRESULT* pRes)
{
	ASSERT(pNMHDR != NULL);
	ASSERT(pRes != NULL);
	((CHARHDR*)pNMHDR)->cf = GetCharFormatSelection();
	*pRes = 1;
}

void CGuideView::OnSetCharFormat(NMHDR* pNMHDR, LRESULT* pRes)
{
	ASSERT(pNMHDR != NULL);
	ASSERT(pRes != NULL);
	SetCharFormat(((CHARHDR*)pNMHDR)->cf);
	*pRes = 1;
}

void CGuideView::SetNumbering(WORD wNumbering)
{
	GetParaFormatSelection();
	m_paraformat.wNumbering = wNumbering;
	m_paraformat.wNumberingStyle = 0x200;
	m_paraformat.wNumberingStart = 1;
	m_paraformat.dxOffset = m_nBulletIndent;
	m_paraformat.dwMask = PFM_NUMBERINGSTART | PFM_NUMBERINGSTYLE | 
			PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
	SetParaFormat(m_paraformat);
	wLastNumbering = wNumbering;
}

void CGuideView::ClearNumbering()
{
	GetParaFormatSelection();
	m_paraformat.wNumbering = 0;
	m_paraformat.wNumberingStyle = 0;
	m_paraformat.wNumberingStart = 0;
	m_paraformat.dxOffset = 0;
	m_paraformat.dxStartIndent = 0;
	m_paraformat.dwMask = PFM_NUMBERINGSTART | PFM_NUMBERINGSTYLE | 
			PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
	SetParaFormat(m_paraformat);
}

void CGuideView::OnNumber()
{
	GetParaFormatSelection();
	if ((m_paraformat.dwMask & PFM_NUMBERING) && 
			m_paraformat.wNumbering >= _PFN_ARABIC)
	{
		ClearNumbering();
	}
	else
	{
		SetNumbering(wLastNumbering);
	}
}

void CGuideView::OnUpdateNumber(CCmdUI* pCmdUI)
{
	GetParaFormatSelection();
	pCmdUI->SetCheck( 
		(m_paraformat.dwMask & PFM_NUMBERING) ? 
			((m_paraformat.wNumbering >= _PFN_ARABIC) ? 1 : 0) : 2);
}

void CGuideView::OnBackgroundColor()
{
	// create the dialog if required
	if (m_bgColorDialog.GetSafeHwnd() == NULL)
		m_bgColorDialog.Create(CFgColorDialog::IDD, this);

	// get the location of the button on the toolbar
	CRect br;
	CToolBar *pFormatBar = theApp.GetFormatBar();
	int index = pFormatBar->CommandToIndex(ID_FORMAT_BACKGROUNDCOLOUR);
	pFormatBar->GetItemRect(index, &br);
	pFormatBar->ClientToScreen(br);

	// get the current location of the dialog
	CRect dr;
	m_bgColorDialog.GetWindowRect(&dr);

	// reposition the dialog
	m_bgColorDialog.MoveWindow(br.left+1, br.bottom+1, dr.Width(), dr.Height());

	// get the current text color and set it for the dialog
	m_bSyncCharFormat = TRUE;
	const CHARFORMAT2& cf = GetCharFormatSelection();
	if ((cf.dwMask & CFM_COLOR) && (cf.dwEffects & CFE_AUTOBACKCOLOR))
		m_bgColorDialog.SetInitialColor((DWORD)-1);
	else
		m_bgColorDialog.SetInitialColor(cf.crBackColor);

	// show it
	m_bgColorDialog.SetFocus();
	m_bgColorDialog.ShowWindow(SW_SHOW);
}

void CGuideView::OnForegroundColor()
{
	// create the dialog if required
	if (m_fgColorDialog.GetSafeHwnd() == NULL)
		m_fgColorDialog.Create(CFgColorDialog::IDD, this);

	// get the location of the button on the toolbar
	CRect br;
	CToolBar *pFormatBar = theApp.GetFormatBar();
	int index = pFormatBar->CommandToIndex(ID_FORMAT_FOREGROUNDCOLOUR);
	pFormatBar->GetItemRect(index, &br);
	pFormatBar->ClientToScreen(br);

	// get the current location of the dialog
	CRect dr;
	m_fgColorDialog.GetWindowRect(&dr);

	// reposition the dialog
	m_fgColorDialog.MoveWindow(br.left+1, br.bottom+1, dr.Width(), dr.Height());

	// get the current text color and set it for the dialog
	m_bSyncCharFormat = TRUE;
	const CHARFORMAT2& cf = GetCharFormatSelection();
	if ((cf.dwMask & CFM_COLOR) && (cf.dwEffects & CFE_AUTOCOLOR))
		m_fgColorDialog.SetInitialColor((DWORD)-1);
	else
		m_fgColorDialog.SetInitialColor(cf.crTextColor);

	// show it
	m_fgColorDialog.SetFocus();
	m_fgColorDialog.ShowWindow(SW_SHOW);
}

void CGuideView::OnSetBgColor(COLORREF cr)
{
	CHARFORMAT2 cf;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BACKCOLOR;

	if (cr == (DWORD)-1)
		cf.dwEffects = CFE_AUTOBACKCOLOR;
	else
		cf.crBackColor = cr;

	SetCharFormat(cf);

	// TODO: Update the button with the new color?
}

void CGuideView::OnSetFgColor(COLORREF cr)
{
	if (cr == (DWORD)-1)
	{
		CHARFORMAT2 cf;
		memset(&cf, 0, sizeof(cf));
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_COLOR;
		cf.dwEffects = CFE_AUTOCOLOR;
		SetCharFormat(cf);
	}
	else
	{
		CRichEditView::OnColorPick(cr);
	}

	// TODO: Update the button with the new color?
}

// Enable/disable right view depending on item count
void CGuideView::UpdateEnabledState()
{
	HWND hWnd = theApp.GetLeftWnd();
	LRESULT nItems = ::SendMessage(hWnd, TVM_GETCOUNT, 0, 0);
	if (nItems <= 0)
	{
		// Startup with a helpful text.
		SetRTF(m_hWnd, theStartupRTFText);
		GetDocument()->SetModifiedFlag(FALSE);
	}
	EnableWindow(nItems > 0);
}

LRESULT CGuideView::OnGuideUI(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case GM_NUMBER_DROP_DOWN:
		OnDropDownNumberingButton();
		break;

	case GM_APPLY_COLOR:
		ApplyColor();
		break;

	case GM_APPLY_FONT:
		ApplyFont();
		break;

	case GM_NEW_PAGE:
		ApplyFont();
		ApplyColor();
		break;

	case GM_TREERMB_PRINT:
		OnTreeRmbPrint((HTREEITEM)lParam);
		break;

	case GM_PREVIEW_PRINT:
		OnPreviewPrint((PrintType)lParam);
		break;

	case GM_TREERMB_PREVIEW:
		OnTreeRmbPreview((HTREEITEM)lParam);
		break;

	case GM_PREVIEW_PREVIEW:
		OnPreviewPreview((PrintType)lParam);
		break;

	case GM_GET_LC:
		(*(CString *)lParam) = GetCurPos();
		break;

	case GM_SET_FGCOLOR:
		OnSetFgColor((COLORREF)lParam);
		break;

	case GM_SET_BGCOLOR:
		OnSetBgColor((COLORREF)lParam);
		break;
	}

	return 0;
}

void CGuideView::OnDropDownNumberingButton()
{
	// get menu
	CMenu numMenu, *subMenu;
	numMenu.LoadMenu(IDR_NUMBER_MENU);
	subMenu = numMenu.GetSubMenu(0);

#define _check_item(id) subMenu->CheckMenuItem((id), MF_BYCOMMAND|MF_CHECKED)
	// set check appropriately
	m_bSyncParaFormat = true;
	GetParaFormatSelection();
	if (m_paraformat.dwMask & PFM_NUMBERING)
	{
		if (m_paraformat.wNumbering == _PFN_ARABIC) 
			_check_item(ID_FMTNUM_ARABIC);
		else if (m_paraformat.wNumbering == _PFN_LOWER_LETTERS) 
			_check_item(ID_FMTNUM_LOWER_ALPHA);
		else if (m_paraformat.wNumbering == _PFN_UPPER_LETTERS)
			_check_item(ID_FMTNUM_UPPER_ALPHA);
		else if (m_paraformat.wNumbering == _PFN_LOWER_ROMAN)
			_check_item(ID_FMTNUM_LOWER_ROMAN);
		else if (m_paraformat.wNumbering == _PFN_UPPER_ROMAN)
			_check_item(ID_FMTNUM_UPPER_ROMAN);
	}
#undef _check_item

	// show menu
	CRect rc;
	CToolBar *pFormatBar = theApp.GetFormatBar();
	int index = pFormatBar->CommandToIndex(ID_FORMAT_NUMBERED);
	pFormatBar->GetItemRect(index, &rc);
	pFormatBar->ClientToScreen(rc);
	subMenu->TrackPopupMenu(
		TPM_LEFTALIGN|TPM_LEFTBUTTON,rc.left,rc.bottom, this);
}

void CGuideView::OnFmtnumArabic()
{
	SetNumbering(_PFN_ARABIC);
}

void CGuideView::OnFmtnumLowerAlpha()
{
	SetNumbering(_PFN_LOWER_LETTERS);
}

void CGuideView::OnFmtnumUpperAlpha()
{
	SetNumbering(_PFN_UPPER_LETTERS);
}

void CGuideView::OnFmtnumLowerRoman()
{
	SetNumbering(_PFN_LOWER_ROMAN);
}

void CGuideView::OnFmtnumUpperRoman()
{
	SetNumbering(_PFN_UPPER_ROMAN);
}

void CGuideView::OnUpdateBullet(CCmdUI* pCmdUI)
{
	GetParaFormatSelection();
	pCmdUI->SetCheck( 
		(m_paraformat.dwMask & PFM_NUMBERING) ? 
			((m_paraformat.wNumbering == PFN_BULLET) ? 1 : 0) : 2);
}

void CGuideView::OnLineSpacing()
{
	// get menu
	CMenu numMenu, *subMenu;
	numMenu.LoadMenu(IDR_LINESPACING);
	subMenu = numMenu.GetSubMenu(0);

#define _check_item(id) subMenu->CheckMenuItem((id), MF_BYCOMMAND|MF_CHECKED)
	// set check appropriately
	m_bSyncParaFormat = true;
	GetParaFormatSelection();
	if (m_paraformat.dwMask & PFM_LINESPACING)
	{
		if (m_paraformat.dyLineSpacing == 20)
			_check_item(ID_FORMAT_LINESPC_ONE);
		else if (m_paraformat.dyLineSpacing == 30)
			_check_item(ID_FORMAT_LINESPC_ONEHALF);
		else if (m_paraformat.dyLineSpacing == 40)
			_check_item(ID_FORMAT_LINESPC_TWO);
	}
#undef _check_item

	// show menu
	CRect rc;
	CToolBar *pFormatBar = theApp.GetFormatBar();
	int index = pFormatBar->CommandToIndex(ID_FORMAT_LINESPACING);
	pFormatBar->GetItemRect(index, &rc);
	pFormatBar->ClientToScreen(rc);
	subMenu->TrackPopupMenu(
		TPM_LEFTALIGN|TPM_LEFTBUTTON,rc.left,rc.bottom, this);
}

void CGuideView::SetSpacing(LONG dyLineSpacing)
{
	m_paraformat.dySpaceAfter = 0;
	m_paraformat.dyLineSpacing = dyLineSpacing;
	m_paraformat.bLineSpacingRule = 5;
	m_paraformat.dwMask = PFM_SPACEAFTER | PFM_LINESPACING;
	SetParaFormat(m_paraformat);
	m_bSyncParaFormat = true;
}

void CGuideView::OnFormatLinespcOne()
{
	SetSpacing(20);
}

void CGuideView::OnFormatLinespcOnehalf()
{
	SetSpacing(30);
}

void CGuideView::OnFormatLinespcTwo()
{
	SetSpacing(40);
}

void CGuideView::OnFormatStrikethrough()
{
	OnCharEffect(CFM_STRIKEOUT, CFM_STRIKEOUT);
}

void CGuideView::OnUpdateFormatStrikethrough(CCmdUI *pCmdUI)
{
	OnUpdateCharEffect(pCmdUI, CFM_STRIKEOUT, CFM_STRIKEOUT);
}

void CGuideView::ApplyFont()
{
	CRichEditCtrl& theCtrl = GetRichEditCtrl();

	// do not do anything if any text is present
	if (theCtrl.GetTextLength() > 0)
		return;

	// get font
	bool fontSysDefault;
	PreferredFont font;
	theApp.GetTextFont(fontSysDefault, font);
	if (fontSysDefault)
		memcpy(&font, &theDefaultFont, sizeof(font));

	// set foreground color and font
	CHARFORMAT2 cf2;
	memset(&cf2, 0, sizeof(cf2));
	cf2.cbSize = sizeof(cf2);
	cf2.dwMask = 
		// font face and size
		CFM_FACE | CFM_SIZE |
		// bold and italic
		CFM_BOLD | CFM_ITALIC | 
		// rest are turned off
		CFM_UNDERLINE | CFM_STRIKEOUT | CFM_PROTECTED | CFM_LINK;
	cf2.dwEffects = 
		(font.bold ? CFE_BOLD : 0) | 
		(font.italic ? CFE_ITALIC : 0);
	cf2.yHeight = 20 * font.height;
	wcscpy(cf2.szFaceName, font.faceName);
	cf2.bCharSet = DEFAULT_CHARSET;
	cf2.bPitchAndFamily = DEFAULT_PITCH;

	theCtrl.SetDefaultCharFormat(cf2);
	SetCharFormat(cf2);
}

void CGuideView::ApplyColor()
{
	CRichEditCtrl& theCtrl = GetRichEditCtrl();

	// get colors
	bool colorSysDefault;
	COLORREF foreColor, backColor;
	theApp.GetTextColor(colorSysDefault, foreColor, backColor);

	// set background color
	if (colorSysDefault)
	{
		theCtrl.SendMessage(EM_SETBKGNDCOLOR, 1);
	}
	else
	{
		theCtrl.SendMessage(EM_SETBKGNDCOLOR, 0, backColor);
	}

	// set the foreground color if there is no text already
	if (theCtrl.GetTextLength() == 0)
	{
		CHARFORMAT2 cf2;
		memset(&cf2, 0, sizeof(cf2));
		cf2.cbSize = sizeof(cf2);
		cf2.dwMask = CFM_COLOR | CFM_BACKCOLOR;
		cf2.dwEffects = CFE_AUTOBACKCOLOR; // bgcolor is always auto
		if (colorSysDefault)
		{
			cf2.dwEffects |= CFE_AUTOCOLOR; // fg is also auto
		}
		else
		{
			cf2.dwEffects   = CFE_AUTOBACKCOLOR; // fg is the specified one
			cf2.crTextColor = foreColor;
		}

		theCtrl.SetDefaultCharFormat(cf2);
		SetCharFormat(cf2);
	}
}

void CGuideView::SetUserPreferences()
{
	ApplyFont();
	ApplyColor();
}

static void ReallyGetCharFormatSelection(CRichEditCtrl& theCtrl, CHARFORMAT2& cf2)
{
	memset(&cf2, 0, sizeof(cf2));
	cf2.cbSize = sizeof(cf2);
	theCtrl.SendMessage(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
}

// Try to move the selection point as much to the left (or right) as
// possible, over a link. Stop when the control does not move the selection
// point further in the given direction OR we reach the end of the link.
static void ExtendSelection(CRichEditCtrl& theCtrl, int direction)
{
	CHARFORMAT2 cf2;
	memset(&cf2, 0, sizeof(cf2));
	cf2.cbSize = sizeof(cf2);

	while (1)
	{
		// get charformat
		theCtrl.SendMessage(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
		// is in link?
		if ((cf2.dwMask & CFM_LINK) && (cf2.dwEffects & CFE_LINK))
		{
			// yes, try moving left (or right)
			CHARRANGE pos, origPos;
			theCtrl.GetSel(pos);
			origPos = pos;
			// try to set new position
			pos.cpMin += direction;
			pos.cpMax += direction;
			theCtrl.SetSel(pos);
			// get current position
			theCtrl.GetSel(pos);
			// if nothing happened, break out
			if (origPos.cpMin + direction != pos.cpMin ||
				origPos.cpMax + direction != pos.cpMax)
				break;
		}
		else
		{
			// not in link, break out
			break;
		}
	}
}

// assuming nothing is selected, and the current selection point is within
// a link, extend the selection over the link
static void ExtendSelectionOverLink(CRichEditCtrl& theCtrl)
{
	// get current selection
	CHARRANGE origPos;
	theCtrl.GetSel(origPos);

	// go left as much as possible, get the position
	ExtendSelection(theCtrl, -1);
	CHARRANGE leftEnd;
	theCtrl.GetSel(leftEnd);

	// restore original position
	theCtrl.SetSel(origPos);

	// go right as much as possible, get the position
	ExtendSelection(theCtrl, +1);
	CHARRANGE rightEnd;
	theCtrl.GetSel(rightEnd);

	// special case: if the link starts with the very first character of the
	// link, then the seeking-left thing stops a bit before where we want it
	// to. In this case, the range (0, leftEnd.cpMin) will contain "HYPERLINK \"xx\""
	// with xx being the actual hyperlink target. If this is the case, just
	// start our selection from 0 rather than the current leftEnd.cpMin.
	CString head;
	theCtrl.GetTextRange(0, leftEnd.cpMin, head);
	if (_tcsncmp(head, L"HYPERLINK \"", 11) == 0 && 
		_tcschr((LPCTSTR)head+11, L'"') == (LPCTSTR)head+head.GetLength()-1)
	{
		// adjust the left end
		leftEnd.cpMin = 0;
	}

	// actually set the selection
	theCtrl.SetSel(leftEnd.cpMin, rightEnd.cpMax);
}

void CGuideView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CRichEditCtrl& theCtrl = GetRichEditCtrl();

	// if nothing is selected
	if (theCtrl.GetSelectionType() == SEL_EMPTY)
	{
		// move selection to click location
		long pos = theCtrl.CharFromPos(point);
		theCtrl.SetSel(pos, pos);

		// check if we're in a link
		CHARFORMAT2 cf2;
		ReallyGetCharFormatSelection(theCtrl, cf2);
		if ((cf2.dwMask & CFM_LINK) && (cf2.dwEffects & CFE_LINK))
			// if so, extend the selection over the link
			ExtendSelectionOverLink(theCtrl);
	}

	CRichEditView::OnRButtonDown(nFlags, point);
}

void CGuideView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ASSERT(m_RMBMenu.m_hMenu);

	ClientToScreen(&point);
	m_RMBMenu.GetSubMenu(0)->TrackPopupMenu(
		TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this);

	CRichEditView::OnRButtonUp(nFlags, point);
}

void CGuideView::OnFormatFont()
{
	GetCharFormatSelection();
	CFontDialog dlg(m_charformat, CF_EFFECTS|CF_BOTH|CF_NOOEMFONTS);
	if (dlg.DoModal() == IDOK)
	{
		dlg.GetCharFormat(m_charformat);
		SetCharFormat(m_charformat);
	}
}

void CGuideView::ProcessHyperlink(const CString& link)
{
	if (_tcsncmp(link, _T("http://"), 7) == 0 || _tcsncmp(link, _T("ftp://"),   6) == 0 ||
		_tcsncmp(link, _T("file://"), 7) == 0 || _tcsncmp(link, _T("https://"), 8) == 0 ||
		_tcsncmp(link, _T("mailto:"), 7) == 0 || _tcsncmp(link, _T("www."),     4) == 0 )
	{
		// strip trailing whitespace characters
		// !!! This is very important since v2.0dev2. This is because of a bug(?) in
		// !!! rich edit control that if the anchor and the hyperlink are exactly
		// !!! the same, the "hyperlink" is degraded to normal text by the control.
		// !!! As a workaround, elsewhere a space is added to the end of the URL
		// !!! to keep it different from the anchor text. Prior to 2.0dev2, this
		// !!! stripping was only as a convenience to those users who accidently
		// !!! selected an extra space and then did 'toggle hyperlink'.
		CString url(link);
		url.TrimRight();

		// a concession for lazy people.. :)
		if (_tcsncmp(link, _T("www."), 4) == 0)
			url.Insert(0, L"http://");

		int ret = (int)ShellExecute(NULL, NULL, url, NULL, NULL, SW_SHOW);
		if (ret <= 32)
		{
			// could not launch URL
			CString msg;
			msg.FormatMessage(IDS_NOLAUNCH_URL, url);
			AfxMessageBox(msg, MB_ICONEXCLAMATION);
		}
	}
	else if (_tcsncmp(link, _T("node://"), 7) == 0)
	{
		int pos = link.ReverseFind(L'/');
		ASSERT(pos != -1);

		// take the number after that (this is the node uid)
		unsigned int uid = (unsigned int)_wtoi((LPCTSTR)link + pos + 1);

		// ask the left view to locate it
		LRESULT result = 
			::SendMessage(theApp.GetLeftWnd(), WM_GUIDEUI, 
				GM_SELECT_NODE_BY_UID, (LPARAM) uid);

		// if we can't find it, the target was deleted etc
		if (result != 1)
		{
			AfxMessageBox(RCSTR(IDS_HLINK_NOTGT), MB_ICONEXCLAMATION);
		}
	}
	else if (_tcsncmp(link, _T("oldlink://"), 10) == 0)
	{
		// ask the left view to locate the node
		LRESULT result = 
			::SendMessage(theApp.GetLeftWnd(), WM_GUIDEUI, GM_SELECT_NODE, 
				(LPARAM) (LPCTSTR) link+10);
		if (result != 1)
		{
			CString msg;
			msg.FormatMessage(IDS_NOLAUNCH_PAGE, link);
			AfxMessageBox(msg, MB_ICONEXCLAMATION);
		}
	}
	else
	{
		// unknown link type
		CString msg;
		msg.FormatMessage(IDS_UNKNOWN_URLTYPE, link);
		AfxMessageBox(msg, MB_ICONEXCLAMATION);
	}
}

// The m_bClearStatusBar is a hack for setting the status bar text back to
// 'Ready' after the mouse leaves the link. In normal case (mouse is over
// regular text, this var. is false, and nothing happens in OnMouseMove().
// When the mouse enters the 'air space' over the link, WM_MOUSEMOVE comes
// first, then an EN_LINK with msg=WM_MOUSEMOVE. In the handler of the 
// second message (OnEnLink) we set m_bClearStatusBar to true. In subsequent
// messages, the WM_MOUSEMOVE will set the status bar text to 'Ready' and
// the EN_LINK set the status bar text to the URL name and also the var.
// to true. When the stupid mouse finally moves out from over the link,
// the final word will be from EN_LINK which sets the var. to true. The
// very next action, however is that the mouse enters the 'regular air
// space', which causes a WM_MOUSEMOVE, and which sets the status bar
// text back to 'Ready'. It also sets the var. to false, so that all this
// nonsense is not repeated.
//
// Thank you, Microsoft.

void CGuideView::OnEnLink(NMHDR *pNMHDR, LRESULT *pResult)
{
	ENLINK *pEnLink = reinterpret_cast<ENLINK *>(pNMHDR);
	CRichEditCtrl& theCtrl = GetRichEditCtrl();

	if (pEnLink->msg == WM_MOUSEMOVE)
	{
		CString link;
		theCtrl.GetTextRange(pEnLink->chrg.cpMin, pEnLink->chrg.cpMax, link);

		theApp.SetStatusBarText(link);
		m_bClearStatusBar = true;
	}
	else
	if ((pEnLink->msg == WM_LBUTTONDOWN && theCtrl.GetSelectionType() == SEL_EMPTY) ||
		pEnLink->msg == WM_LBUTTONDBLCLK)
	{
		CString link;
		theCtrl.GetTextRange(pEnLink->chrg.cpMin, pEnLink->chrg.cpMax, link);

		ProcessHyperlink(link);
	}

	*pResult = 0;
}

static CStringA encodeShiftJIS(LPCTSTR s)
{
	// check how many bytes will be present in converted output
	int nBytes = WideCharToMultiByte(932 /*ShiftJIS*/, 0, s, -1, NULL, 0, 0, 0);
	if (!nBytes) return ""; // failed to convert
	++nBytes; // add 1 for '\0' (required?)

	// allocate that many bytes
	char *buf = new char[nBytes];
	ASSERT(buf);
	if (!buf) return ""; // out of memory

	// convert it
	memset(buf, 0, nBytes);
	WideCharToMultiByte(932 /*ShiftJIS*/, 0, s, -1, buf, nBytes, NULL, NULL);

	// create the final string, after escaping "bad" bytes
	size_t len = strlen(buf);
	CStringA out;
	char tmp[10];
	for (size_t i=0; i<len; ++i)
	{
		char ch = buf[i];
		if (ch >= 32  && ch <= 127)
			out += ch;
		else
		{
			sprintf(tmp, "\\\'%02x", ch & 0xff);
			out += tmp;
		}
	}

	// delete temp buf
	delete[] buf;
	buf = 0;

	return out;
}

static const char *_linkRTF =
	"{\\rtf1\\ansi\\deflang%d{\\fonttbl{\\f0\\fnil\\fcharset%d %s;}}\n"
	"{\\colortbl ;\\red0\\green0\\blue255;\\red%d\\green%d\\blue%d;}\n"
	"\\viewkind4\\uc1\\pard\\highlight2\\f0\\fs%d{\\field{\\*\\fldinst{HYPERLINK \"%s\"}}{\\fldrslt{\\cf1\\ul %s}}}\\cf0\\ulnone\n"
	"}\n";

static void ReplaceSelWithHyperlink(CRichEditCtrl& theCtrl,
	const CString& text, CString link)
{
	CHARFORMAT2 cf2;
	ReallyGetCharFormatSelection(theCtrl, cf2);

	DWORD bgColor = cf2.crBackColor;
	if (cf2.dwEffects & CFE_AUTOBACKCOLOR)
		bgColor = GetSysColor(COLOR_WINDOW);

	CStringA rtf;
	rtf.Format(_linkRTF, 
		cf2.lcid, cf2.bCharSet, encodeShiftJIS(cf2.szFaceName),
		GetRValue(bgColor), GetGValue(bgColor), GetBValue(bgColor),
		cf2.yHeight/10, escapeRTF(link), escapeRTF(text));
	SetRTFSelection(theCtrl, rtf);
}

static void ClearHyperlink(CRichEditCtrl& theCtrl,
	const CString& text)
{
	theCtrl.ReplaceSel(_T(""), TRUE);
	theCtrl.ReplaceSel(text, TRUE);

	// For some reason that ms only knows, the second line alone
	// does not work. EM_SETTEXTEX and EM_STREAMIN both also do
	// not work.
}

void CGuideView::OnFormatHyperlink()
{
	CRichEditCtrl& theCtrl = GetRichEditCtrl();

	// get selected text (note: there's bug in GetSelText()).
	CHARRANGE pos;
	theCtrl.GetSel(pos);
	CString selText;
	theCtrl.GetTextRange(pos.cpMin, pos.cpMax, selText);

	// get the selected node from the left view
	struct tree_node_t *node = 
		(struct tree_node_t *)theApp.GetLeftView()
			->SendMessage(WM_GUIDEUI, GM_GET_SELECTED_NODE);

	// show the link dialog
	CHyperLinkDialog aLinkDialog(selText, GetDocument()->m_pGuide, node);
	if (aLinkDialog.DoModal() == IDCANCEL)
		return;

	// set text with link or just text
	if (aLinkDialog.IsClearLink())
		ClearHyperlink(theCtrl, aLinkDialog.GetText());
	else
		ReplaceSelWithHyperlink(theCtrl, aLinkDialog.GetText(), aLinkDialog.GetLink());
}

void CGuideView::SwitchFocus()
{
	CView *leftView = theApp.GetLeftView();

	CFrameWnd *frameWnd = (CFrameWnd *)theApp.m_pMainWnd;
	ASSERT(frameWnd);
	frameWnd->SetActiveView(leftView);
}

CString CGuideView::GetCurPos()
{
	// resource strings:
	//	IDS_SBARFMT1	"(no text)"
	//	IDS_SBARFMT2	"Lines: %d, Chars: %d"
	//	IDS_SBARFMT3	"Line: %d of %d, Char: %d"
	//	IDS_SBARFMT4	"Sel: %d lines, %d chars"

	CRichEditCtrl& theCtrl = GetRichEditCtrl();
	CString result;

	// get line count
	int lineCount = theCtrl.GetLineCount();

	// if we don't have the focus just update the line count
	if (::GetFocus() != m_hWnd)
	{
		// get character count
		int charCount = GetTextLengthEx(GTL_PRECISE);

		// update status bar
		if (charCount == 0)
			result = RCSTR(IDS_SBARFMT1);
		else
			result.Format(RCSTR(IDS_SBARFMT2), lineCount, charCount);
	}
	else
	{
		// get the current pos
		CHARRANGE cr = { 0, 0 };
		theCtrl.GetSel(cr);

		// if there is no selection, show "line: n of n, char: n"
		if (cr.cpMin == cr.cpMax)
		{
			// get line index of current line
			int lineIndex = theCtrl.SendMessage(EM_EXLINEFROMCHAR, 0, cr.cpMin);
			// get index of first char in current line
			int firstChar = theCtrl.SendMessage(EM_LINEINDEX, -1);

			// format the string and update status bar
			result.Format(RCSTR(IDS_SBARFMT3), lineIndex+1, lineCount, cr.cpMin-firstChar+1);
		}
		else
		// if there is a selection, show "selection: n lines, n characters"
		{
			// get line index of first line
			int firstLineIndex = theCtrl.SendMessage(EM_EXLINEFROMCHAR, 0, cr.cpMin);
			// get line index of last line
			int lastLineIndex = theCtrl.SendMessage(EM_EXLINEFROMCHAR, 0, cr.cpMax);

			// format the string and update status bar
			result.Format(RCSTR(IDS_SBARFMT4), lastLineIndex-firstLineIndex+1, cr.cpMax-cr.cpMin);
		}
	}

	// at last, return it!
	return result;
}

void CGuideView::OnEnMsgfilter(NMHDR *pNMHDR, LRESULT *pResult)
{
	MSGFILTER *pMsgFilter = reinterpret_cast<MSGFILTER *>(pNMHDR);

	if (pMsgFilter->msg == WM_KEYDOWN)
	{
		if (pMsgFilter->wParam == VK_TAB && (::GetKeyState(VK_CONTROL) & 0x8000))
		{
			SwitchFocus();
			*pResult = 1;
			return;
		}
	}
	else if (pMsgFilter->msg == WM_KEYUP && pMsgFilter->wParam == VK_ESCAPE &&
		theApp.GetEscMinimize())
	{
		AfxGetMainWnd()->PostMessage(WM_SYSCOMMAND, SC_MINIMIZE);
	}

	*pResult = 0;
}

//-- BEGIN print functionality (v1.5) ----------------------------------------

// Overridden version of OnPrint(), the only difference being the use of
// MyPrintPage() rather than PrintPage(), since PrintPage() itself was
// not virtual.
void CGuideView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT(pInfo != NULL);
	ASSERT(pInfo->m_bContinuePrinting);

	UINT nPage = pInfo->m_nCurPage;
	ASSERT(nPage <= (UINT)m_aPageStart.GetSize());
	long nIndex = (long) m_aPageStart[nPage-1];

	// print as much as possible in the current page.
	nIndex = MyPrintPage(pDC, nIndex, 0xFFFFFFFF);

	GETTEXTLENGTHEX lex;
	lex.codepage = -1;
	lex.flags = GTL_PRECISE | GTL_NUMCHARS;
	long nLength = (long) ::SendMessage(m_pRTFConcatenator->GetWnd(),
		EM_GETTEXTLENGTHEX, (WPARAM)&lex, 0);

	if (nIndex >= nLength)
	{
		TRACE(traceAppMsg, 0, "End of Document\n");
		pInfo->SetMaxPage(nPage);
	}

	// update pagination information for page just printed
	if (nPage == (UINT)m_aPageStart.GetSize())
	{
		if (nIndex < nLength)
			m_aPageStart.Add(nIndex);
	}
	else
	{
		ASSERT(nPage+1 <= (UINT)m_aPageStart.GetSize());
		ASSERT(nIndex == (long)m_aPageStart[nPage+1-1]);
	}
}

// Our version of CRichEditView::PrintPage function (non-virtual, so
// can't override) with the only difference being the window to which
// the EM_FORMATRANGE is sent to.
long CGuideView::MyPrintPage(CDC* pDC, long nIndexStart, long nIndexStop)
	// worker function for laying out text in a rectangle.
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	FORMATRANGE fr;

	// offset by printing offset
	pDC->SetViewportOrg(-pDC->GetDeviceCaps(PHYSICALOFFSETX),
		-pDC->GetDeviceCaps(PHYSICALOFFSETY));
	// adjust DC because richedit doesn't do things like MFC
	if (::GetDeviceCaps(pDC->m_hDC, TECHNOLOGY) != DT_METAFILE && pDC->m_hAttribDC != NULL)
	{
		::ScaleWindowExtEx(pDC->m_hDC,
			::GetDeviceCaps(pDC->m_hDC, LOGPIXELSX),
			::GetDeviceCaps(pDC->m_hAttribDC, LOGPIXELSX),
			::GetDeviceCaps(pDC->m_hDC, LOGPIXELSY),
			::GetDeviceCaps(pDC->m_hAttribDC, LOGPIXELSY), NULL);
	}

	fr.hdcTarget = pDC->m_hAttribDC;
	fr.hdc = pDC->m_hDC;
	fr.rcPage = GetPageRect();
	fr.rc = GetPrintRect();

	fr.chrg.cpMin = nIndexStart;
	fr.chrg.cpMax = nIndexStop;

	// send the message to our window, not this->m_hWnd.
	ASSERT(m_pRTFConcatenator);
	return (long) ::SendMessage(m_pRTFConcatenator->GetWnd(), EM_FORMATRANGE, TRUE, (LPARAM)&fr);
}

// While beginning printing, create a hidden RTF window with the entire
// text and use that during printing.
void CGuideView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// commit current RTF
	theApp.GetLeftView()->SendMessage(WM_GUIDEUI, GM_COMMIT_RTF_NOW);

	// get wintree
	struct wintree_t *pWinTree = (struct wintree_t *)
		theApp.GetLeftView()->SendMessage(WM_GUIDEUI, GM_GET_WINTREE);

	// create temp window with all RTF
	ASSERT(m_pRTFConcatenator == 0);
	m_pRTFConcatenator = new RTFConcatenator();
	PrintTemplate pt;
	theApp.GetPrintTemplate(pt);
	m_pRTFConcatenator->SetTemplate(pt);

	if (thePrintItemInfo.GetPrintType() == PT_ENTIRE_DOCUMENT)
		m_pRTFConcatenator->CollectAll(pWinTree);
	else if (thePrintItemInfo.GetPrintType() == PT_SELECTED_AND_CHILDREN)
		m_pRTFConcatenator->CollectRecurse(pWinTree, thePrintItemInfo.GetSelectedItem());
	else // if (thePrintItemInfo.GetPrintType() == PT_ONLY_SELECTED)
		m_pRTFConcatenator->CollectOne(pWinTree, thePrintItemInfo.GetSelectedItem());

	// initialize page start vector
	ASSERT(m_aPageStart.GetSize() == 0);
	m_aPageStart.Add(0);
	ASSERT(m_aPageStart.GetSize() > 0);
	::SendMessage(m_pRTFConcatenator->GetWnd(), EM_FORMATRANGE, FALSE, 0);
}

// cleanup.
void CGuideView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Note that m_pRTFConcatenator might be null here. This happens if
	// print preview is invoked and there is no printer installed and the
	// user answers 'No' to the question that MFC asks ("do you want to 
	// install a printer?") -- basically when OnPreparePrinting() returns
	// FALSE, OnBeginPrinting() is not called in case of print preview).

	if (m_pRTFConcatenator)
	{
		::SendMessage(m_pRTFConcatenator->GetWnd(), EM_FORMATRANGE, FALSE, 0);
		m_aPageStart.RemoveAll();

		ASSERT(m_pRTFConcatenator);
		delete m_pRTFConcatenator;
		m_pRTFConcatenator = 0;
	}
}

// NOTE: the following method has been overriden from 
// CRichEditView::OnPrepareDC() to fix the problem of
// CRichEditView printing an extra, blank page at the
// end of a print cycle (only for print, not for preview).
void CGuideView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);
	ASSERT(pInfo != NULL);  // overriding OnPaint -- never get this.

	pDC->SetMapMode(MM_TEXT);

// -- The if condition below was the original code:
//	if (pInfo->m_nCurPage > (UINT)m_aPageStart.GetSize() &&
//		!PaginateTo(pDC, pInfo))
// -- The PaginateTo() call is omitted in our version.

	if (pInfo->m_nCurPage > (UINT)m_aPageStart.GetSize())
	{
		// can't paginate to that page, thus cannot print it.
		pInfo->m_bContinuePrinting = FALSE;
	}
	ASSERT_VALID(this);
}

//-- END print functionality (v1.5) ----------------------------------------

void CGuideView::OnFormatDecindent()
{
	PARAFORMAT2 pf = GetParaFormatSelection();
	pf.dwMask = PFM_STARTINDENT;
	if (pf.dxStartIndent <= 360)
		pf.dxStartIndent = 0;
	else
		pf.dxStartIndent -= 360;
	SetParaFormat(pf);
}

void CGuideView::OnFormatIncindent()
{
	PARAFORMAT2 pf = GetParaFormatSelection();
	pf.dwMask = PFM_STARTINDENT;
	pf.dxStartIndent += 360;
	SetParaFormat(pf);
}

void CGuideView::OnFileExport()
{
	// resource string:
	//	IDS_FILEEXPORT_FILT	"Rich Text Format (*.rtf)|*.rtf|All Files (*.*)|*.*||"

	// initial file name
	CString initialFile = GetDocument()->GetTitle() + _T(".rtf");

	// show the dialog box
	CExportFileDialog aSaveDialog(FALSE, PT_ENTIRE_DOCUMENT, _T("rtf"), initialFile, 
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, RCSTR(IDS_FILEEXPORT_FILT), this);
	if (aSaveDialog.DoModal() == IDCANCEL)
		return;

	// get the print type and selected item
	PrintType requestedPrintType = aSaveDialog.m_finalType;
	HTREEITEM hItem = TreeView_GetSelection(theApp.GetLeftWnd());
	ASSERT(hItem);

	// get the filename
	CString fileName = aSaveDialog.GetPathName();

	// remove read-only attribute
	_tchmod(fileName, _S_IREAD | _S_IWRITE);

	// commit current RTF
	theApp.GetLeftView()->SendMessage(WM_GUIDEUI, GM_COMMIT_RTF_NOW);

	// get wintree
	struct wintree_t *pWinTree = (struct wintree_t *)
		theApp.GetLeftView()->SendMessage(WM_GUIDEUI, GM_GET_WINTREE);

	// start a wait cursor
	CWaitCursor aWaitCursor;

	// get the RTF
	CStringA rtf;
	{
		RTFConcatenator aConcat;
		PrintTemplate pt;
		theApp.GetPrintTemplate(pt);
		aConcat.SetTemplate(pt);

		if (requestedPrintType == PT_ENTIRE_DOCUMENT)
			aConcat.CollectAll(pWinTree);
		else if (requestedPrintType == PT_SELECTED_AND_CHILDREN)
			aConcat.CollectRecurse(pWinTree, hItem);
		else
			aConcat.CollectOne(pWinTree, hItem);

		aConcat.GetRTF(rtf);
	}

	// write to file
	CString errMsg;
	FILE *fp = _tfopen(fileName, _T("wt"));
	if (fp)
	{
		if (fwrite((const char *)rtf, rtf.GetLength(), 1, fp) != 1)
			errMsg = __tcserror(NULL);
		fclose(fp);
	}
	else
	{
		errMsg = __tcserror(NULL);
	}

	// display error message if any error occured
	if (errMsg.GetLength() > 0)
	{
		CString msg;
		msg.FormatMessage(IDS_FILE_WRITE_ERROR, errMsg);
		AfxMessageBox(msg, MB_ICONSTOP);
	}
	// else display a status bar message
	else
	{
		theApp.SetStatusBarText(IDS_EXPORT_SUCCESS);
	}
}

void CGuideView::OnFilePrintPreviewWrapper()
{
	// Note: The following was copied from CView::OnFilePrintPreview(),
	// and only the first and third parameters of the DoPrintPreview()
	// call have been changed.
	// ----------------------------------------------------------------

	// In derived classes, implement special window handling here
	// Be sure to Unhook Frame Window close if hooked.

	// must not create this on the frame.  Must outlive this function
	CPrintPreviewState* pState = new CPrintPreviewState;

	// DoPrintPreview's return value does not necessarily indicate that
	// Print preview succeeded or failed, but rather what actions are necessary
	// at this point.  If DoPrintPreview returns TRUE, it means that
	// OnEndPrintPreview will be (or has already been) called and the
	// pState structure will be/has been deleted.
	// If DoPrintPreview returns FALSE, it means that OnEndPrintPreview
	// WILL NOT be called and that cleanup, including deleting pState
	// must be done here.

	if (!DoPrintPreview(IDD_PRINT_PREVIEW_BAR, this,
							RUNTIME_CLASS(CGuidePreview), pState))
	{
		// In derived classes, reverse special window handling here for
		// Preview failure case

		TRACE(traceAppMsg, 0, "Error: DoPrintPreview failed.\n");
		AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
		delete pState;      // preview failed to initialize, delete State now
	}
}

void CGuideView::OnEndPrintPreview(CDC* pDC, CPrintInfo* pInfo, POINT point, CPreviewView* pView)
{
	bool shouldRedo = false;
	PrintType newType = PT_ENTIRE_DOCUMENT;

	// invoke the preview again if the user changed the combo to some other view
	ASSERT(pView->IsKindOf(RUNTIME_CLASS(CGuidePreview)));
	if (pView->IsKindOf(RUNTIME_CLASS(CGuidePreview)))
	{
		CGuidePreview *pGuideView = (CGuidePreview *)pView;
		if (pGuideView->m_bShouldRedo)
		{
			shouldRedo = true;
			newType = pGuideView->m_RequestedType;
		}
	}

	// finish off the printing
	CRichEditView::OnEndPrintPreview(pDC, pInfo, point, pView);

	// cause a new preview
	if (shouldRedo)
	{
		this->PostMessage(WM_GUIDEUI, GM_PREVIEW_PREVIEW, (LPARAM)newType);
	}
}

// Handler for ID_FILE_PRINT, which occurs only when top-level menu or
// shortcut key is used. In this case, set the (initial) printType to 
// entire document. The user can change his mind during the print
// dialog, so set the selected item as the tree view's selected item.
void CGuideView::OnFilePrint()
{
	thePrintItemInfo.SetPrintType(PT_ENTIRE_DOCUMENT);
	thePrintItemInfo.SetSelectedItem(TreeView_GetSelection(theApp.GetLeftWnd()));

	CRichEditView::OnFilePrint();
}

// Handler for print from RMB menu of tree. In this case, set the
// (initial) printType to the selected item and children. Set the
// selected item to the one that came from the left view.
void CGuideView::OnTreeRmbPrint(HTREEITEM hItem)
{
	thePrintItemInfo.SetPrintType(PT_SELECTED_AND_CHILDREN);
	thePrintItemInfo.SetSelectedItem(hItem);

	CRichEditView::OnFilePrint();
}

// Handler for the print from the preview toolbar button. In this
// case, set the (initial) printType to the given one (which was
// the combo box entry). The selected item should have been set
// by now (it should have been set when the preview was invoked
// itself).
void CGuideView::OnPreviewPrint(PrintType printType)
{
	thePrintItemInfo.SetPrintType(printType);
	ASSERT(thePrintItemInfo.GetSelectedItem());

	CRichEditView::OnFilePrint();
}

// Handler for ID_FILE_PRINT_PREVIEW, which occurs only when top-level 
// menu or shortcut key is used. In this case, set the (initial) printType
// to the entire document. The user can change his mind during the preview,
// so set the selected item as the tree view's selected item.
void CGuideView::OnFilePrintPreview()
{
	thePrintItemInfo.SetPrintType(PT_ENTIRE_DOCUMENT);
	thePrintItemInfo.SetSelectedItem(TreeView_GetSelection(theApp.GetLeftWnd()));

	OnFilePrintPreviewWrapper();
}

// Handler for preview from RMB menu of tree. In this case, set the
// (initial) printType to the selected item and children. Set the
// selected item to the one that came from the left view.
void CGuideView::OnTreeRmbPreview(HTREEITEM hItem)
{
	thePrintItemInfo.SetPrintType(PT_SELECTED_AND_CHILDREN);
	thePrintItemInfo.SetSelectedItem(hItem);

	OnFilePrintPreviewWrapper();
}

// Handler for the preview from the preview view itself (this happens
// when the user changes the combo box entry). In this case, set the
// printType to the given one (the user's choice from the combo box).
// The selected item should have been already set during the invocation
// of the first preview itself (from the above handlers).
void CGuideView::OnPreviewPreview(PrintType printType)
{
	thePrintItemInfo.SetPrintType(printType);
	ASSERT(thePrintItemInfo.GetSelectedItem());

	OnFilePrintPreviewWrapper();
}

void CGuideView::OnEditInsertdate()
{
	GetRichEditCtrl().ReplaceSel(GetConfiguredDateString(), TRUE);
}

void CGuideView::OnEditInserttime()
{
	GetRichEditCtrl().ReplaceSel(GetConfiguredTimeString(), TRUE);
}

void CGuideView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bClearStatusBar)
	{
		theApp.SetStatusBarText(AFX_IDS_IDLEMESSAGE);
		m_bClearStatusBar = false;
	}

	CRichEditView::OnMouseMove(nFlags, point);
}

void CGuideView::OnEditPasteastext()
{
	SendMessage(EM_PASTESPECIAL, CF_TEXT);
}

void CGuideView::OnUpdateEditPasteastext(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(IsClipboardFormatAvailable(CF_TEXT));
}
