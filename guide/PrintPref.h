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

#include "hslistbox.h"

// CPrintPref dialog

class CPrintPref : public CPropertyPage
{
private:
	// I'm that sort of a chap..
	DECLARE_DYNAMIC(CPrintPref)

	// icons for the buttons
	HANDLE m_hIconLeft;
	HANDLE m_hIconCenter;
	HANDLE m_hIconRight;
	HANDLE m_hIconBold;
	HANDLE m_hIconItalic;
	HANDLE m_hIconUnderline;

	// the template we'll modify
	PrintTemplate m_Template;

public:
	// ctor/dtor
	CPrintPref();
	~CPrintPref();

	// DDX/DDV
	CHSListBox m_hsListBox;
	CComboBox  m_fontNames;
	CComboBox  m_fontSizes;
	void DoDataExchange(CDataExchange* pDX);

	BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedPfxnum();
	afx_msg void OnBnClickedBeginonnew();
	afx_msg void OnBnClickedJusleft();
	afx_msg void OnBnClickedJuscenter();
	afx_msg void OnBnClickedJusright();
	afx_msg void OnBnClickedHcolor();
	afx_msg void OnBnClickedResetall();
	afx_msg void OnLbnSelchangeHdglist();
	afx_msg void OnCbnSelchangePfontnames();
	afx_msg void OnCbnSelchangePfontsizes();
	afx_msg void OnBnClickedPfmtbold();
	afx_msg void OnBnClickedPfmtitalic();
	afx_msg void OnBnClickedPfmtunderline();
	virtual BOOL OnApply();
};
