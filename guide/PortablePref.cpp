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
#include ".\portablepref.h"

// CPortablePref dialog

IMPLEMENT_DYNAMIC(CPortablePref, CPropertyPage)
CPortablePref::CPortablePref()
	: CPropertyPage(CPortablePref::IDD)
{
}

CPortablePref::~CPortablePref()
{
}

void CPortablePref::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPortablePref, CPropertyPage)
	ON_BN_CLICKED(IDC_RGY2INI, OnBnClickedRgy2ini)
	ON_BN_CLICKED(IDC_INI2RGY, OnBnClickedIni2rgy)
	ON_BN_CLICKED(IDC_PMODE, OnBnClickedPmode)
END_MESSAGE_MAP()

// CPortablePref message handlers

BOOL CPortablePref::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	SetRadioCheck(IDC_PMODE, theApp.GetPortableMode());

	return TRUE;
}

BOOL CPortablePref::OnApply()
{
	bool bChecked = IsRadioChecked(IDC_PMODE);
	theApp.SetPortableMode(bChecked);

	return CPropertyPage::OnApply();
}

void CPortablePref::OnBnClickedRgy2ini()
{
	if (AfxMessageBox(IDS_RGY2INI_CONFIRM, MB_YESNO) == IDYES)
		theApp.CopyFromRegistry();
}

void CPortablePref::OnBnClickedIni2rgy()
{
	if (AfxMessageBox(IDS_INI2RGY_CONFIRM, MB_YESNO) == IDYES)
		theApp.CopyFromIni();
}

void CPortablePref::OnBnClickedPmode()
{
	SetModified();
}
