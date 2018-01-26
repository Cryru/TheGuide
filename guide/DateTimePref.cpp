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
#include "Utils.h"
#include ".\datetimepref.h"

///////////////////////////////////////////////////////////////////////////////
// CFormatDialog class

class CFormatDialog : public CDialog
{
public:
	// format function signature (use GetDateString() or GetTimeString())
	typedef CString (*formatFunctionType)(LPCTSTR format);

	// ctor
	CFormatDialog(UINT idd, formatFunctionType ffunc, LPCTSTR format);

	// the initial and final format string
	CString m_szFormat;

	// do some validation in OnOK()
	void OnOK();

// "private":
	afx_msg void OnEnChangeEditfmt();
	void DoDataExchange(CDataExchange* pDX);
	CString m_szSample;
	formatFunctionType m_cbFormatFunc;

// boilerplate MFC stuff
	DECLARE_DYNAMIC(CFormatDialog)
	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CFormatDialog, CDialog)

BEGIN_MESSAGE_MAP(CFormatDialog, CDialog)
	ON_EN_CHANGE(IDC_EDITFMT, OnEnChangeEditfmt)
END_MESSAGE_MAP()

CFormatDialog::CFormatDialog(UINT idd, 
	CFormatDialog::formatFunctionType ffunc, LPCTSTR format)
	: CDialog(idd)
	, m_cbFormatFunc(ffunc)
{
	m_szFormat = format;
	m_szSample = ffunc(format);
}

void CFormatDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITFMT, m_szFormat);
	DDX_Text(pDX, IDC_EDITSAMPLE, m_szSample);
}

void CFormatDialog::OnEnChangeEditfmt()
{
	UpdateData();
	m_szSample = m_cbFormatFunc(m_szFormat);
	UpdateData(FALSE);
}

void CFormatDialog::OnOK()
{
	if (m_szFormat.IsEmpty())
	{
		AfxMessageBox(IDS_NOBLANKFORMAT, MB_OK|MB_ICONERROR);
		return;
	}

	CDialog::OnOK();
}

///////////////////////////////////////////////////////////////////////////////
// CDateFormatDialog class

class CDateFormatDialog : public CFormatDialog
{
public:
	CDateFormatDialog(LPCTSTR format)
		: CFormatDialog(IDD_DATEFMT, GetDateString, format)
	{ }
};

///////////////////////////////////////////////////////////////////////////////
// CTimeFormatDialog class

class CTimeFormatDialog : public CFormatDialog
{
public:
	CTimeFormatDialog(LPCTSTR format)
		: CFormatDialog(IDD_TIMEFMT, GetTimeString, format)
	{ }
};

///////////////////////////////////////////////////////////////////////////////
// CDateTimePref dialog

IMPLEMENT_DYNAMIC(CDateTimePref, CPropertyPage)

CDateTimePref::CDateTimePref()
	: CPropertyPage(CDateTimePref::IDD)
{
}

CDateTimePref::~CDateTimePref()
{
}

void CDateTimePref::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDateTimePref, CPropertyPage)
	ON_BN_CLICKED(IDC_SET_DATEFMT, OnBnClickedSetDatefmt)
	ON_BN_CLICKED(IDC_SET_TIMEFMT, OnBnClickedSetTimefmt)
	ON_BN_CLICKED(IDC_RADIO_DATEFMT_SYSTEM, OnAnyRadioButtonClicked)
	ON_BN_CLICKED(IDC_RADIO_DATEFMT_CUSTOM, OnAnyRadioButtonClicked)
	ON_BN_CLICKED(IDC_RADIO_TIMEFMT_SYSTEM, OnAnyRadioButtonClicked)
	ON_BN_CLICKED(IDC_RADIO_TIMEFMT_CUSTOM, OnAnyRadioButtonClicked)
END_MESSAGE_MAP()

// CDateTimePref message handlers

void CDateTimePref::OnBnClickedSetDatefmt()
{
	// show the dialog
	CDateFormatDialog aDlg(m_Format.dateFormat);
	if (aDlg.DoModal() == IDCANCEL)
		return;

	// remember the value given by user
	m_Format.useSystemDateFormat = false;
	m_Format.dateFormat = aDlg.m_szFormat;

	// update radio button text
	FormatAndSet(IDC_RADIO_DATEFMT_CUSTOM, IDS_FMT_DATECUS, 
		GetDateString(m_Format.dateFormat));

	// update radio button
	if (IsRadioChecked(IDC_RADIO_DATEFMT_SYSTEM))
	{
		SetRadioCheck(IDC_RADIO_DATEFMT_SYSTEM, false);
		SetRadioCheck(IDC_RADIO_DATEFMT_CUSTOM, true);
		SetModified();
	}
}

void CDateTimePref::OnBnClickedSetTimefmt()
{
	// show the dialog
	CTimeFormatDialog aDlg(m_Format.timeFormat);
	if (aDlg.DoModal() == IDCANCEL)
		return;

	// remember the value given by user
	m_Format.useSystemTimeFormat = false;
	m_Format.timeFormat = aDlg.m_szFormat;

	// update radio button text
	FormatAndSet(IDC_RADIO_TIMEFMT_CUSTOM, IDS_FMT_TIMECUS, 
		GetTimeString(m_Format.timeFormat));

	// update radio button
	if (IsRadioChecked(IDC_RADIO_TIMEFMT_SYSTEM))
	{
		SetRadioCheck(IDC_RADIO_TIMEFMT_SYSTEM, false);
		SetRadioCheck(IDC_RADIO_TIMEFMT_CUSTOM, true);
		SetModified();
	}
}

void CDateTimePref::OnAnyRadioButtonClicked()
{
	SetModified();
}

void CDateTimePref::FormatAndSet(UINT ctrlId, UINT idsId, const CString& value)
{
	CString t;
	t.FormatMessage(idsId, value);
	GetDlgItem(ctrlId)->SetWindowText(t);
}

BOOL CDateTimePref::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Get the current values
	theApp.GetDateTimeFormat(m_Format);

	// set the radio buttons
	SetRadioCheck(IDC_RADIO_DATEFMT_SYSTEM, m_Format.useSystemDateFormat);
	SetRadioCheck(IDC_RADIO_DATEFMT_CUSTOM, ! m_Format.useSystemDateFormat);
	SetRadioCheck(IDC_RADIO_TIMEFMT_SYSTEM, m_Format.useSystemTimeFormat);
	SetRadioCheck(IDC_RADIO_TIMEFMT_CUSTOM, ! m_Format.useSystemTimeFormat);

	// set the system & custom values
	FormatAndSet(IDC_RADIO_DATEFMT_SYSTEM, IDS_FMT_DATESYS, GetDateString(NULL));
	FormatAndSet(IDC_RADIO_DATEFMT_CUSTOM, IDS_FMT_DATECUS, GetDateString(m_Format.dateFormat));
	FormatAndSet(IDC_RADIO_TIMEFMT_SYSTEM, IDS_FMT_TIMESYS, GetTimeString(NULL));
	FormatAndSet(IDC_RADIO_TIMEFMT_CUSTOM, IDS_FMT_TIMECUS, GetTimeString(m_Format.timeFormat));

	return TRUE;
}

BOOL CDateTimePref::OnApply()
{
	m_Format.useSystemDateFormat = IsRadioChecked(IDC_RADIO_DATEFMT_SYSTEM);
	m_Format.useSystemTimeFormat = IsRadioChecked(IDC_RADIO_TIMEFMT_SYSTEM);
	theApp.SetDateTimeFormat(m_Format);

	return CPropertyPage::OnApply();
}
