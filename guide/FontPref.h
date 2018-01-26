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

#include "guide.h"

// CFontPref dialog

class CFontPref : public CPropertyPage
{
	DECLARE_DYNAMIC(CFontPref)

public:
	CFontPref();
	virtual ~CFontPref();

// Dialog Data
	enum { IDD = IDD_PREF_FONTS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_nTextRadio;
	int m_nTreeRadio;
	afx_msg void OnBnClickedTreeFontChoose();
	afx_msg void OnBnClickedTextFontChoose();
	afx_msg void OnBnClickedRadioTreeFontSysdefault();
	afx_msg void OnBnClickedRadioTreeFontUser();
	afx_msg void OnBnClickedRadioTextFontSysdefault();
	afx_msg void OnBnClickedRadioTextFontUser();
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
private:
	PreferredFont m_oTextFont;
	PreferredFont m_oTreeFont;
};
