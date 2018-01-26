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
#include ".\startuppref.h"

// CStartupPref dialog

IMPLEMENT_DYNAMIC(CStartupPref, CPropertyPage)

BEGIN_MESSAGE_MAP(CStartupPref, CPropertyPage)
	ON_BN_CLICKED(IDC_RADIO_SHOW_EMPTY, OnAnyButtonClicked)
	ON_BN_CLICKED(IDC_RADIO_LOAD_LAST, OnAnyButtonClicked)
	ON_BN_CLICKED(IDC_CHECK_SYSTRAY, OnMinToTrayButtonClicked)
	ON_BN_CLICKED(IDC_CHECK_ESC_MIN, OnAnyButtonClicked)
	ON_BN_CLICKED(IDC_CHECK_X_MIN, OnAnyButtonClicked)
	ON_BN_CLICKED(IDC_CHECK_SAVE_ON_CLOSE, OnAnyButtonClicked)
	ON_BN_CLICKED(IDC_CHECK_AUTOSAVE, OnBnClickedCheckAutosave)
	ON_BN_CLICKED(IDC_CHECK_BACKUP, OnAnyButtonClicked)
	ON_BN_CLICKED(IDC_CHECK_ICONS, OnAnyButtonClicked)
	ON_BN_CLICKED(IDC_CHECK_CBOXES, OnAnyButtonClicked)
END_MESSAGE_MAP()

CStartupPref::CStartupPref()
	: CPropertyPage(CStartupPref::IDD)
	, m_nRadio(0)
	, m_nMinutes(0)
{
}

CStartupPref::~CStartupPref()
{
}

void CStartupPref::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_SHOW_EMPTY, m_nRadio);
	DDX_Text(pDX, IDC_AUTOSAVE_MINUTES, m_nMinutes);
	DDV_MinMaxUInt(pDX, m_nMinutes, 0, 99);
}

BOOL CStartupPref::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// startup option
	StartupOption opt;
	theApp.GetStartupOption(opt);
	m_nRadio = int(opt);

	// system tray
	bool sysTray = theApp.GetMinimizeToSystemTray();
	SetRadioCheck(IDC_CHECK_SYSTRAY, sysTray);
	GetDlgItem(IDC_CHECK_SINGLECLICK)->EnableWindow(sysTray);
	SetRadioCheck(IDC_CHECK_SINGLECLICK, theApp.GetSingleClickRestore());

	// esc minimizes
	SetRadioCheck(IDC_CHECK_ESC_MIN, theApp.GetEscMinimize());

	// x minimizes
	SetRadioCheck(IDC_CHECK_X_MIN, theApp.GetXMinimize());

	// auto save on close
	SetRadioCheck(IDC_CHECK_SAVE_ON_CLOSE, theApp.GetSaveOnClose());

	// auto save and auto save interval
	bool autoSave = theApp.GetAutoSave();
	SetRadioCheck(IDC_CHECK_AUTOSAVE, autoSave);
	unsigned minutes = theApp.GetAutosaveInterval();
	m_nMinutes = minutes;
	GetDlgItem(IDC_AUTOSAVE_MINUTES)->EnableWindow(autoSave);

	// auto backup
	SetRadioCheck(IDC_CHECK_BACKUP, theApp.GetAutoBackup());

	// tree decoration
	TreeDecoration deco = theApp.GetTreeDecoration();
	SetRadioCheck(IDC_CHECK_ICONS, SHOW_ICONS(deco));
	SetRadioCheck(IDC_CHECK_CBOXES, SHOW_CHECKBOXES(deco));

	UpdateData(FALSE);
	return TRUE;
}

BOOL CStartupPref::OnApply()
{
	if (!UpdateData())
		return FALSE;

	// startup option
	ASSERT(m_nRadio == 0 || m_nRadio == 1);
	theApp.SetStartupOption(StartupOption(m_nRadio));

	// system tray
	theApp.SetMinimizeToSystemTray(IsRadioChecked(IDC_CHECK_SYSTRAY));
	theApp.SetSingleClickRestore(IsRadioChecked(IDC_CHECK_SINGLECLICK));

	// esc minimizes
	theApp.SetEscMinimize(IsRadioChecked(IDC_CHECK_ESC_MIN));

	// x minimizes
	theApp.SetXMinimize(IsRadioChecked(IDC_CHECK_X_MIN));

	// auto save on close
	theApp.SetSaveOnClose(IsRadioChecked(IDC_CHECK_SAVE_ON_CLOSE));

	// auto save and auto save interval
	theApp.SetAutoSave(IsRadioChecked(IDC_CHECK_AUTOSAVE));
	theApp.SetAutosaveInterval(m_nMinutes);
	// notify left view
	theApp.GetLeftView()->SendMessage(WM_GUIDEUI, GM_APPLY_AUTOSAVE);

	// auto backup
	theApp.SetAutoBackup(IsRadioChecked(IDC_CHECK_BACKUP));

	// tree decoration
	int newDeco = 0;
	newDeco |= IsRadioChecked(IDC_CHECK_ICONS)  ? TD_ICONS : 0;
	newDeco |= IsRadioChecked(IDC_CHECK_CBOXES) ? TD_CHECKBOXES : 0;
	theApp.SetTreeDecoration((TreeDecoration)newDeco);

	return CPropertyPage::OnApply();
}

void CStartupPref::OnAnyButtonClicked()
{
	SetModified();
}

void CStartupPref::OnMinToTrayButtonClicked()
{
	SetModified();

	GetDlgItem(IDC_CHECK_SINGLECLICK)->EnableWindow(IsRadioChecked(IDC_CHECK_SYSTRAY));
}

void CStartupPref::OnBnClickedCheckAutosave()
{
	SetModified();

	GetDlgItem(IDC_AUTOSAVE_MINUTES)->EnableWindow(IsRadioChecked(IDC_CHECK_AUTOSAVE));
}
