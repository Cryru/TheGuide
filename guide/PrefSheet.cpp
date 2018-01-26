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
#include "FontPref.h"
#include "ColorPref.h"
#include "StartupPref.h"
#include "PrintPref.h"
#include "DateTimePref.h"
#include "PortablePref.h"
#include ".\prefsheet.h"

// CPrefSheet

IMPLEMENT_DYNAMIC(CPrefSheet, CPropertySheet)
CPrefSheet::CPrefSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CPrefSheet::CPrefSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CPrefSheet::~CPrefSheet()
{
}

BEGIN_MESSAGE_MAP(CPrefSheet, CPropertySheet)
END_MESSAGE_MAP()

// CPrefSheet message handlers

INT_PTR CPrefSheet::DoModal()
{
	CFontPref aFontPrefPage;
	AddPage(&aFontPrefPage);

	CColorPref aColorPrefPage;
	AddPage(&aColorPrefPage);

	CDateTimePref aDateTimePrefPage;
	AddPage(&aDateTimePrefPage);

	CStartupPref aStartupPrefPage;
	AddPage(&aStartupPrefPage);

	CPrintPref aPrintPrefPage;
	AddPage(&aPrintPrefPage);

	CPortablePref aPortPrefPage;
	AddPage(&aPortPrefPage);

	return CPropertySheet::DoModal();
}
