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

#include "stdafx.h"
#include <afxpriv.h>

// CGuidePreview

enum PrintType;

class CGuidePreview : public CPreviewView
{
	DECLARE_DYNCREATE(CGuidePreview)
	PrintType GetPrintType();

public:
	bool m_bShouldRedo;
	PrintType m_RequestedType;

	CGuidePreview();

	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	afx_msg void OnSelChange();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedIdPreviewPrint();
};


