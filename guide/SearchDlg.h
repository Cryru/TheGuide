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

#include <afxtempl.h>	// for CArray
#include <afxcmn.h>

struct tree_node_t;

// some info about each result (match) we found
struct ResultInfo
{
	CHARRANGE range;
	HTREEITEM item;
};

class CSearchDlg : public CDialog
{
	DECLARE_DYNAMIC(CSearchDlg)

public:
	CSearchDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSearchDlg();

private:
	CListCtrl m_result;		// list control for results
	HWND m_hRichEditWnd;	// hidden rich edit window
	CString m_szText;		// text to search for
	bool m_bMatchCase;		// match case?
	bool m_bOnlyTitle;		// only in titles?
	// holds more info about each match
	CArray<ResultInfo, ResultInfo&> m_arrResultInfo;

	HWND m_hSizeBox;

	// callback for wintree_traverse
	static int SearchCallback(HTREEITEM item, 
		struct tree_node_t *node, void *cargo);

	// you know what these do
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	// message handlers
	afx_msg void OnBnClickedSclose();
	afx_msg void OnBnClickedSfind();
	afx_msg void OnDblclkResult(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	// yeah, we do a message map, whaddya think?
	DECLARE_MESSAGE_MAP()

	// some vars reqd for resizing the dlg
	long min_w, min_h;
	int listctrl_border_w, listctrl_border_h;
};
