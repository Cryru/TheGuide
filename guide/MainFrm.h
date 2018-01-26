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

#include "afxwin.h"
#include "FormatBar.h"
#include "searchdlg.h"

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

private:
	CSplitterWnd m_wndSplitter;
	CFormatBar   m_wndFormatBar;
	CStatusBar	 m_wndStatusBar;
	CToolBar	 m_wndToolBar;
	CMenu		 m_TrayMenu;
	CSearchDlg	 m_SearchDlg;
	int			 m_initialOVR;

	BOOL CreateFormatBar();

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	void SetStatusBarText(LPCTSTR text);
	void SetStatusBarText(UINT id);
	virtual ~CMainFrame();

	DECLARE_MESSAGE_MAP()

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnTrayIconMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPopupRestore();
	afx_msg void OnPopupExit();
	afx_msg LRESULT OnGuideUI(WPARAM wParam, LPARAM lParam);
	afx_msg void OnViewPreferences();
	afx_msg void OnTreeSearch();
	afx_msg void OnUpdateChildCountIndicator(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLCIndicator(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditInsertdate(CCmdUI *pCmdUI);
};
