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

#include "FlatButton.h"

// CFgColorDialog dialog

class CFgColorDialog : public CDialog
{
	DECLARE_DYNAMIC(CFgColorDialog)
	CToolTipCtrl m_toolTip;
	CColorButton m_buttons[40];
	CFlatButton m_buttonAuto;
	CFlatButton m_buttonMore;
	COLORREF m_clrInitial, m_clrFinal;
	WPARAM m_wParam;

public:
	CFgColorDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFgColorDialog();

// Dialog Data
	enum { IDD = IDD_FGCOLOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	void SetInitialColor(COLORREF c);
	void SetWParam(WPARAM wParam) { m_wParam = wParam; }
	COLORREF GetFinalColor() const { return m_clrFinal; }

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedFgcolorAuto();
	afx_msg void OnBnClickedFgcolorMore();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnFgColor(UINT nID);

	virtual void OnSelectColor(COLORREF selectedColor) = 0;
};
