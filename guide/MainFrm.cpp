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
#include "LeftView.h"
#include "GuideView.h"
#include "FormatBarDefs.h"
#include "PrefSheet.h"
#include ".\mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SYSCOMMAND()
	ON_MESSAGE(WM_GUIDE_TRAYICON, OnTrayIconMsg)
	ON_COMMAND(ID_TRAY_RESTORE, OnPopupRestore)
	ON_COMMAND(ID_TRAY_EXIT, OnPopupExit)
	ON_COMMAND(ID_VIEW_PREFERENCES, OnViewPreferences)
	ON_COMMAND(ID_TREE_SEARCH, OnTreeSearch)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CHILDCOUNT, OnUpdateChildCountIndicator)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_LC, OnUpdateLCIndicator)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INSERTDATE, OnUpdateEditInsertdate)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INSERTTIME, OnUpdateEditInsertdate)
	// Our own messages
	ON_MESSAGE(WM_GUIDEUI, OnGuideUI)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_LC,
	ID_INDICATOR_CHILDCOUNT,
};

// for saving/restoring state of control bars
static const TCHAR barStateSection[] = _T("barState");

CMainFrame::CMainFrame()
{
	m_initialOVR = (::GetKeyState(VK_INSERT) & 1);
}

CMainFrame::~CMainFrame()
{
}

static void MakeDropDown(CToolBar& ctrl, UINT id)
{
	int index = ctrl.CommandToIndex(id);
	DWORD dwStyle = ctrl.GetButtonStyle(index);
	dwStyle |= TBBS_DROPDOWN;
	ctrl.SetButtonStyle(index, dwStyle);
}

BOOL CMainFrame::CreateFormatBar()
{
	if (!m_wndFormatBar.CreateEx(this, TBSTYLE_FLAT, 
			WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
			CRect(0,0,0,0), ID_VIEW_FORMATBAR) ||
		!m_wndFormatBar.LoadToolBar(IDR_FORMAT_BAR))
	{
		TRACE0("Failed to create format toolbar\n");
		return -1;      // fail to create
	}

	m_wndFormatBar.SetButtons(formatBarButtons, formatBarButtonCount);
	m_wndFormatBar.SetButtonInfo(BUTTON_FONTNAME, 
		IDC_FONTNAME, TBBS_SEPARATOR, WIDTH_FONTNAME);
	m_wndFormatBar.SetButtonInfo(1, ID_SEPARATOR, TBBS_SEPARATOR, 12);
	m_wndFormatBar.SetButtonInfo(BUTTON_FONTSIZE, 
		IDC_FONTSIZE, TBBS_SEPARATOR, WIDTH_FONTSIZE);

	m_wndFormatBar.SendMessage(TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
	MakeDropDown(m_wndFormatBar, ID_FORMAT_NUMBERED);

	m_wndFormatBar.SetSizes(CSize(23,22), CSize(16,16));
	m_wndFormatBar.SetWindowText(RCSTR(IDS_FORMATBAR_NAME));
	m_wndFormatBar.PositionCombos();

	return TRUE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP |
			/*CBRS_GRIPPER | */CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!CreateFormatBar())
		return -1;

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(*indicators)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndFormatBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	DockControlBar(&m_wndFormatBar);

	//tool = (L -2, T 0, R 198, B 26)
	//format = (L 196, T 0, R 674, B 26)
	CRect rTool(-2, 0, 198, 26);
	CRect rFormat(196, 0, 674, 26);
	ClientToScreen(rTool);
	ClientToScreen(rFormat);

	DockControlBar(&m_wndToolBar, AFX_IDW_DOCKBAR_TOP, &rTool);
	DockControlBar(&m_wndFormatBar, AFX_IDW_DOCKBAR_TOP, &rFormat);

	// load the tray menu
	m_TrayMenu.LoadMenu(IDR_TRAYMENU);

	// v1.4: Restore control bar states
	LoadBarState(barStateSection);

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	// restore pane widths
	int treeWidth, textWidth;
	theApp.GetPaneWidths(treeWidth, textWidth);

	if (!m_wndSplitter.CreateView(0, 0, 
			RUNTIME_CLASS(CLeftView), CSize(treeWidth, 100), pContext) ||
		!m_wndSplitter.CreateView(0, 1, 
			RUNTIME_CLASS(CGuideView), CSize(textWidth, 100), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}

	return TRUE;
}

void CMainFrame::OnDestroy() 
{
/*
	CRect r;
	m_wndToolBar.GetWindowRect(&r); ScreenToClient(&r);
	afxDump << "tool = " << r << "\n";
	m_wndFormatBar.GetWindowRect(&r); ScreenToClient(&r);
	afxDump << "format = " << r << "\n";
*/
	// v1.6: BUG FIX! Do not get the sizes etc. if we're minimized!!
	if (! IsIconic())
	{
		// v1.4: Save control bar states
		SaveBarState(barStateSection);

		// get widths of panes and save it
		int treeCur, treeMin, textCur, textMin;
		m_wndSplitter.GetColumnInfo(0, treeCur, treeMin);
		m_wndSplitter.GetColumnInfo(1, textCur, textMin);
		theApp.SetPaneWidths(treeCur, textCur);

		// get location of main window and save it
		CRect r;
		GetWindowRect(&r);
		theApp.SetMainFrameSize(r.left, r.top, r.Width(), r.Height());
	}

	CFrameWnd::OnDestroy();
}

void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
	// if we're asked close
	if (nID == SC_CLOSE)
	{
		// ..and this is configured to mean minimize, then minimize instead,
		if (theApp.GetXMinimize())
			PostMessage(WM_SYSCOMMAND, SC_MINIMIZE);
		// ..else post an ID_APP_EXIT, so that all closes go via the method
		// CGuideApp::OnAppExit() only. This is required for the "auto save
		// on close" option.
		else
			PostMessage(WM_COMMAND, ID_APP_EXIT);
		return;
	}

	// do tray minimize if configured
	if (nID == SC_MINIMIZE && theApp.GetMinimizeToSystemTray())
	{
		CString title;
		GetWindowText(title);
		theApp.AddTrayIcon(title);
		ShowWindow(SW_HIDE);
		return;
	}

	// else do the normal stuff
	CFrameWnd::OnSysCommand(nID, lParam);
}

LRESULT CMainFrame::OnTrayIconMsg(WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_RBUTTONUP)
	{
	    CPoint pos;
	    GetCursorPos(&pos);
		SetForegroundWindow();
		CMenu *subMenu = m_TrayMenu.GetSubMenu(0);
		subMenu->SetDefaultItem(0, TRUE);
		subMenu->TrackPopupMenu(
			TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON,
			pos.x, pos.y, this);
		PostMessage(WM_NULL);
	}
	else if (lParam == WM_LBUTTONDBLCLK && theApp.GetSingleClickRestore() == false)
	{
		OnPopupRestore();
	}
	else if (lParam == WM_LBUTTONUP && theApp.GetSingleClickRestore() == true)
	{
		OnPopupRestore();
	}

	return 0;
}

void CMainFrame::OnPopupRestore()
{
	// show ourselves
	ShowWindow(SW_SHOW);

	// remove the icon
	theApp.RemoveTrayIcon();

	// reqd to bring ourselves to the front
	SetForegroundWindow();
}

void CMainFrame::OnPopupExit()
{
	SendMessage(WM_CLOSE);
}

BOOL CMainFrame::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR *pNMHdr = (NMHDR *)lParam;
	if (pNMHdr->code == TBN_DROPDOWN)
	{
		NMTOOLBAR *pNMToolbar = (NMTOOLBAR *)lParam;
		if (pNMToolbar->iItem == ID_FORMAT_NUMBERED)
			::SendMessage(theApp.GetRightWnd(), WM_GUIDEUI,
				GM_NUMBER_DROP_DOWN, 0);
	}

	return CFrameWnd::OnNotify(wParam, lParam, pResult);
}

void CMainFrame::OnViewPreferences()
{
	CPrefSheet aPrefSheet(RCSTR(IDS_PREFDLG_NAME));
	aPrefSheet.DoModal();
}

void CMainFrame::SetStatusBarText(LPCTSTR text)
{
	m_wndStatusBar.SetWindowText(text);
}

void CMainFrame::SetStatusBarText(UINT id)
{
	m_wndStatusBar.SetWindowText(RCSTR(id));
}

void CMainFrame::OnTreeSearch()
{
	if (m_SearchDlg.m_hWnd == NULL)
		m_SearchDlg.Create(IDD_SEARCH_DIALOG, this);
	m_SearchDlg.ShowWindow(SW_SHOW);
	m_SearchDlg.GetDlgItem(IDC_SWHAT)->SetFocus();
	m_SearchDlg.GetDlgItem(IDC_SWHAT)->SendMessage(EM_SETSEL, 0, -1);
}

LRESULT CMainFrame::OnGuideUI(WPARAM wParam, LPARAM lParam)
{
	if (wParam == GM_SHOW_SEARCH)
	{
		OnTreeSearch();
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// restore the size and location of main window
	int x, y, w, h;
	if (theApp.GetMainFrameSize(x, y, w, h))
	{
		cs.x = x;
		cs.y = y;
		cs.cx = w;
		cs.cy = h;
	}

	return CFrameWnd::PreCreateWindow(cs);
}

void CMainFrame::OnUpdateChildCountIndicator(CCmdUI* pCmdUI)
{
	BOOL isEnabled = theApp.GetRightView()->IsWindowEnabled();

	pCmdUI->Enable(isEnabled);
	if (isEnabled)
	{
		CView *pLeftView = theApp.GetLeftView();
		LRESULT childCount = pLeftView->SendMessage(WM_GUIDEUI, GM_GET_CHILD_COUNT);
		if (childCount == 0)
		{
			pCmdUI->SetText(RCSTR(IDS_CHILDCOUNT_0));
		}
		else if (childCount == 1)
		{
			pCmdUI->SetText(RCSTR(IDS_CHILDCOUNT_1));
		}
		else
		{
			CString str;
			str.Format(RCSTR(IDS_CHILDCOUNT_MANY), childCount);
			pCmdUI->SetText(str);
		}
	}

	// update title also (for * if modified)
	OnUpdateFrameTitle(TRUE);
}

void CMainFrame::OnUpdateLCIndicator(CCmdUI* pCmdUI)
{
	BOOL isEnabled = theApp.GetRightView()->IsWindowEnabled();

	pCmdUI->Enable(isEnabled);
	if (isEnabled)
	{
		CString result;
		theApp.GetRightView()->SendMessage(WM_GUIDEUI, GM_GET_LC, (LPARAM)&result);
		pCmdUI->SetText(result);
	}
}

void CMainFrame::OnUpdateEditInsertdate(CCmdUI *pCmdUI)
{
	CView *leftView = theApp.GetLeftView();
	CView *rightView = theApp.GetRightView();
	HWND hFocusWnd = ::GetFocus();

	BOOL bEnable =
		leftView && rightView && rightView->IsWindowEnabled() &&
			(rightView->m_hWnd == hFocusWnd || TreeView_GetEditControl(leftView->m_hWnd));

	pCmdUI->Enable(bEnable);
}

// copied from winfrm.cpp CFrameWnd::OnUpdateFrameTitle(BOOL).
void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	if ((GetStyle() & FWS_ADDTOTITLE) == 0)
		return;     // leave it alone!
/*
#ifndef _AFX_NO_OLE_SUPPORT
	// allow hook to set the title (used for OLE support)
	if (m_pNotifyHook != NULL && m_pNotifyHook->OnUpdateFrameTitle())
		return;
#endif
*/
	CDocument* pDocument = GetActiveDocument();
	if (bAddToTitle && pDocument != NULL)
	{
//		UpdateFrameTitleForDocument(pDocument->GetTitle());
		CString title = pDocument->GetTitle();
		if (pDocument->IsModified())
			title += _T('*');
		UpdateFrameTitleForDocument(title);
	}
	else
		UpdateFrameTitleForDocument(NULL);
}
