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
#include "coloredstatic.h"

// CColorPref dialog

class CColorPref : public CPropertyPage
{
	DECLARE_DYNAMIC(CColorPref)

public:
	CColorPref();
	virtual ~CColorPref();

// Dialog Data
	enum { IDD = IDD_PREF_COLORS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	int m_nTreeRadio;
	int m_nTextRadio;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	afx_msg void OnBnClickedRadioTextColorSysdefault();
	afx_msg void OnBnClickedRadioTextColorUser();
	afx_msg void OnBnClickedRadioTreeColorSysdefault();
	afx_msg void OnBnClickedRadioTreeColorUser();
	afx_msg void OnBnClickedTextBack();
	afx_msg void OnBnClickedTextFore();
	afx_msg void OnBnClickedTreeBack();
	afx_msg void OnBnClickedTreeFore();
	CColoredStatic m_oTreeSample;
	CColoredStatic m_oTextSample;
};
