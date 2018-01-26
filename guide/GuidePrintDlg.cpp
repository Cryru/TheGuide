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
#include ".\guideprintdlg.h"

// CGuidePrintDlg

IMPLEMENT_DYNAMIC(CGuidePrintDlg, CPrintDialog)

CGuidePrintDlg::CGuidePrintDlg(BOOL bPrintSetupOnly, DWORD dwFlags, CWnd* pParentWnd) :
	CPrintDialog(bPrintSetupOnly, dwFlags, pParentWnd)
{
	// empty
}

BEGIN_MESSAGE_MAP(CGuidePrintDlg, CPrintDialog)
	ON_BN_CLICKED(IDC_PRINTDLG_ENTIRE_DOCUMENT, OnBnClickedPrintdlgEntireDocument)
	ON_BN_CLICKED(IDC_PRINTDLG_ONLY_SELECTED, OnBnClickedPrintdlgOnlySelected)
	ON_BN_CLICKED(IDC_PRINTDLG_SELANDCHILDREN, OnBnClickedPrintdlgSelandchildren)
END_MESSAGE_MAP()

// CGuidePrintDlg message handlers

BOOL CGuidePrintDlg::OnInitDialog()
{
	CPrintDialog::OnInitDialog();

	PrintType printType = thePrintItemInfo.GetPrintType();
	UINT id = 0;

	switch (printType)
	{
		case PT_ENTIRE_DOCUMENT:		id = IDC_PRINTDLG_ENTIRE_DOCUMENT; break;
		case PT_ONLY_SELECTED:			id = IDC_PRINTDLG_ONLY_SELECTED; break;
		case PT_SELECTED_AND_CHILDREN:	id = IDC_PRINTDLG_SELANDCHILDREN; break;
	}

	if (id)
		GetDlgItem(id)->SendMessage(BM_SETCHECK, BST_CHECKED);

	return TRUE;
}

UINT_PTR CALLBACK PrintHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	if (uiMsg == WM_INITDIALOG)
	{
		HWND hWnd = 0;
		if (thePrintItemInfo.GetPrintType() == PT_ENTIRE_DOCUMENT)
			hWnd = GetDlgItem(hdlg, IDC_PRINTDLG_ENTIRE_DOCUMENT);
		else if (thePrintItemInfo.GetPrintType() == PT_ONLY_SELECTED)
			hWnd = GetDlgItem(hdlg, IDC_PRINTDLG_ONLY_SELECTED);
		else
			hWnd = GetDlgItem(hdlg, IDC_PRINTDLG_SELANDCHILDREN);

		::SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
	}

	return 0;
}

INT_PTR CGuidePrintDlg::DoModal()
{
	m_pd.Flags |= PD_ENABLEPRINTHOOK;
	m_pd.lpfnPrintHook = PrintHookProc;

	return CPrintDialog::DoModal();
}

void CGuidePrintDlg::OnBnClickedPrintdlgEntireDocument()
{
	thePrintItemInfo.SetPrintType(PT_ENTIRE_DOCUMENT);
}

void CGuidePrintDlg::OnBnClickedPrintdlgOnlySelected()
{
	thePrintItemInfo.SetPrintType(PT_ONLY_SELECTED);
}

void CGuidePrintDlg::OnBnClickedPrintdlgSelandchildren()
{
	thePrintItemInfo.SetPrintType(PT_SELECTED_AND_CHILDREN);
}
