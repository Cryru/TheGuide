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

// CDateTimePref dialog

class CDateTimePref : public CPropertyPage
{
	DECLARE_DYNAMIC(CDateTimePref)
	void FormatAndSet(UINT ctrlId, UINT idsId, const CString& value);
	DateTimeFormat m_Format;

public:
	CDateTimePref();
	virtual ~CDateTimePref();

// Dialog Data
	enum { IDD = IDD_PREF_DATEFMT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedSetDatefmt();
	afx_msg void OnBnClickedSetTimefmt();
	afx_msg void OnAnyRadioButtonClicked();
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
};
