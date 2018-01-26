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
#include ".\guidepreview.h"

IMPLEMENT_DYNCREATE(CGuidePreview, CPreviewView)

CGuidePreview::CGuidePreview()
	: m_bShouldRedo(false)
	, m_RequestedType(PT_ENTIRE_DOCUMENT)
{
	// empty
}

static inline void AddString(CComboBox *pCombo, UINT id)
{
	CString str;
	VERIFY(str.LoadString(id));
	pCombo->AddString(str);
}

static int EnumToIndex(PrintType pt)
{
	// NOTE: keep this in sync with the AddString() and GetPrintType() calls!

	if (pt == PT_ONLY_SELECTED)
		return 1;
	else if (pt == PT_SELECTED_AND_CHILDREN)
		return 2;
	return 0;
}

PrintType CGuidePreview::GetPrintType()
{
	// NOTE: keep this in sync with the AddString() and EnumToIndex() calls!

	CComboBox *pCombo = (CComboBox *)m_pToolBar->GetDlgItem(IDC_PRINT_PREVIEW_WHAT);
	ASSERT(pCombo);
	if (pCombo)
	{
		switch (pCombo->GetCurSel())
		{
			case 0: return PT_ENTIRE_DOCUMENT;
			case 1: return PT_ONLY_SELECTED;
			case 2: return PT_SELECTED_AND_CHILDREN;
		}
	}

	/* NOT REACHED */
	return PT_ENTIRE_DOCUMENT;
}

void CGuidePreview::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate && pActivateView->m_hWnd == this->m_hWnd && pDeactiveView->m_hWnd != this->m_hWnd)
	{
		CComboBox *pCombo = (CComboBox *)m_pToolBar->GetDlgItem(IDC_PRINT_PREVIEW_WHAT);
		if (pCombo && pCombo->GetCount() == 0)
		{
			// NOTE: keep this in sync with EnumToIndex() !
			AddString(pCombo, IDS_PRINT_ALL);				// at index 0
			AddString(pCombo, IDS_PRINT_ONLYSEL);			// at index 1
			AddString(pCombo, IDS_PRINT_SELANDCHILDREN);	// at index 2

			// set the initial selection
			pCombo->SetCurSel(EnumToIndex(thePrintItemInfo.GetPrintType()));
		}

		this->SetFocus();
	}
}

BEGIN_MESSAGE_MAP(CGuidePreview, CPreviewView)
	ON_CBN_SELCHANGE(IDC_PRINT_PREVIEW_WHAT, OnSelChange)
	ON_BN_CLICKED(AFX_ID_PREVIEW_PRINT, OnBnClickedIdPreviewPrint)
END_MESSAGE_MAP()

void CGuidePreview::OnSelChange()
{
	m_RequestedType = GetPrintType();

	// close the preview
	PostMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE, 0);
	// ask for redo
	m_bShouldRedo = true;
}

void CGuidePreview::OnBnClickedIdPreviewPrint()
{
	// Note: refer also the original implementation CPreviewView::OnPreviewPrint().

	PrintType theType = GetPrintType();

	OnPreviewClose();
	theApp.GetRightView()->SendMessage(WM_GUIDEUI, GM_PREVIEW_PRINT, (LPARAM)theType);
}
