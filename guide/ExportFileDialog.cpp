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
#include ".\exportfiledialog.h"

// CExportFileDialog

static LPOFNHOOKPROC oldHookProc;
static PrintType initialType;

static inline void AddString(HWND hCombo, UINT id)
{
	CString str;
	VERIFY(str.LoadString(id));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)str);
}

static UINT_PTR CALLBACK ExportHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	if (uiMsg == WM_INITDIALOG)
	{
		HWND combo = GetDlgItem(hdlg, IDC_EXPORT_WHAT);
		AddString(combo, IDS_PRINT_ALL);
		AddString(combo, IDS_PRINT_ONLYSEL);
		AddString(combo, IDS_PRINT_SELANDCHILDREN);

		int sel = 0;
		if (initialType == PT_ONLY_SELECTED)
			sel = 1;
		else if (initialType == PT_SELECTED_AND_CHILDREN)
			sel = 2;
		SendMessage(combo, CB_SETCURSEL, sel, 0);
	}

	return oldHookProc(hdlg, uiMsg, wParam, lParam);
}

IMPLEMENT_DYNAMIC(CExportFileDialog, CFileDialog)

CExportFileDialog::CExportFileDialog(BOOL bOpenFileDialog, PrintType initialType,
	LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
	DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd)
	: CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, 
		dwFlags|OFN_ENABLETEMPLATE|OFN_EXPLORER, lpszFilter, pParentWnd)
	, m_finalType(initialType)
{
	OPENFILENAME& ofn = GetOFN();

	ofn.lpTemplateName = MAKEINTRESOURCE(IDD_CUSTOM_EXPORT);
	oldHookProc = ofn.lpfnHook;    // save old hook
	ofn.lpfnHook = ExportHookProc;
	ofn.Flags |= OFN_ENABLESIZING; // gets turned off in CFileDialog ctor
}

BEGIN_MESSAGE_MAP(CExportFileDialog, CFileDialog)
	ON_CBN_SELCHANGE(IDC_EXPORT_WHAT, OnCbnSelchangeExportWhat)
END_MESSAGE_MAP()

// CExportFileDialog message handlers

void CExportFileDialog::OnCbnSelchangeExportWhat()
{
	CComboBox *combo = (CComboBox *)GetDlgItem(IDC_EXPORT_WHAT);
	ASSERT(combo);
	if (combo)
	{
		int curSel = combo->GetCurSel();
		if (curSel == 0)
			m_finalType = PT_ENTIRE_DOCUMENT;
		else if (curSel == 1)
			m_finalType = PT_ONLY_SELECTED;
		else if (curSel == 2)
			m_finalType = PT_SELECTED_AND_CHILDREN;
	}
}
