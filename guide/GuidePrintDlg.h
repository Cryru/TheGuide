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

// CGuidePrintDlg

class CGuidePrintDlg : public CPrintDialog
{
	DECLARE_DYNAMIC(CGuidePrintDlg)

public:
	CGuidePrintDlg(BOOL bPrintSetupOnly,
			// TRUE for Print Setup, FALSE for Print Dialog
			DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS
				| PD_HIDEPRINTTOFILE | PD_NOSELECTION,
			CWnd* pParentWnd = NULL);

protected:
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	INT_PTR DoModal();
	afx_msg void OnBnClickedPrintdlgEntireDocument();
	afx_msg void OnBnClickedPrintdlgOnlySelected();
	afx_msg void OnBnClickedPrintdlgSelandchildren();
};
