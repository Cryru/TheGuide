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
#include ".\keyboardhelp.h"

// CKeyboardHelp dialog

IMPLEMENT_DYNAMIC(CKeyboardHelp, CDialog)

CKeyboardHelp::CKeyboardHelp(CWnd* pParent /*=NULL*/)
	: CDialog(CKeyboardHelp::IDD, pParent)
{
}

CKeyboardHelp::~CKeyboardHelp()
{
}

void CKeyboardHelp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_list1);
	DDX_Control(pDX, IDC_LIST2, m_list2);
}

BEGIN_MESSAGE_MAP(CKeyboardHelp, CDialog)
END_MESSAGE_MAP()

// CKeyboardHelp message handlers

#define BEGIN_LIST(ctrl) { CListCtrl* cp = &ctrl; int idx = 0;
#define ADD_ITEM(t1,t2) cp->InsertItem(idx,_T(t1)); cp->SetItemText(idx++,1,_T(t2));
#define END_LIST() }

BOOL CKeyboardHelp::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list1.InsertColumn(0, L"Key", LVCFMT_LEFT, 100);
	m_list1.InsertColumn(1, L"Description", LVCFMT_LEFT, 450);
	m_list1.SetExtendedStyle(LVS_EX_FULLROWSELECT|/*LVS_EX_LABELTIP*/0x4000);
	BEGIN_LIST(m_list1)
		ADD_ITEM("Shift+Up",	 "Move current page up")
		ADD_ITEM("Shift+Down",	 "Move current page down")
		ADD_ITEM("Insert",		 "Add page above current page")
		ADD_ITEM("Shift+Insert", "Add page below current page")
		ADD_ITEM("Ctrl+Enter",  "Add top-level page")
		ADD_ITEM("+",			 "Expand subtree by one level")
		ADD_ITEM("*",			 "Expand subtree entirely")
		ADD_ITEM("-",			 "Collapse subtree")
		ADD_ITEM("Enter",		 "Add child page")
		ADD_ITEM("Delete",		 "Delete current page and its children")
		ADD_ITEM("Space",		 "Rename current page")
		ADD_ITEM("F2",			 "Rename current page")
		ADD_ITEM("Tab",			 "Shift focus to text pane")
		ADD_ITEM("Ctrl+Tab",	 "Shift focus to text pane")
		ADD_ITEM("Arrow Keys",	 "Move around in the tree")
		ADD_ITEM("Ctrl+Arrow Keys", "Scroll the view")
	END_LIST()

	m_list2.InsertColumn(0, L"Key", LVCFMT_LEFT, 100);
	m_list2.InsertColumn(1, L"Description", LVCFMT_LEFT, 450);
	m_list2.SetExtendedStyle(LVS_EX_FULLROWSELECT|/*LVS_EX_LABELTIP*/0x4000);
	BEGIN_LIST(m_list2)
		ADD_ITEM("Ctrl+Tab",	"Shift focus to tree pane")
		ADD_ITEM("Ctrl+L",		"Left align paragraph")
		ADD_ITEM("Ctrl+R",		"Right align paragraph")
		ADD_ITEM("Ctrl+E",		"Center align paragraph")
		ADD_ITEM("Ctrl+1",		"Single line spacing")
		ADD_ITEM("Ctrl+5",		"1.5 line spacing")
		ADD_ITEM("Ctrl+2",		"Double line spacing")
		ADD_ITEM("Ctrl+Shift+L","Change bullet style")
		ADD_ITEM("Ctrl+`+'",    "Subscript")
		ADD_ITEM("Ctrl+Shift+`+'", "Superscript")
		ADD_ITEM("Ctrl+B",		"Bold")
		ADD_ITEM("Ctrl+I",		"Italics")
		ADD_ITEM("Ctrl+U",		"Underline")
		ADD_ITEM("Ctrl+K",		"Strikethrough")
		ADD_ITEM("Ctrl+Shift+H","Hyperlink")
		ADD_ITEM("Ctrl+Shift+M","Decrease paragraph indent")
		ADD_ITEM("Ctrl+M",		"Increase paragraph indent")
		ADD_ITEM("Ctrl+Delete",	"Delete till end of word")
		ADD_ITEM("Ctrl+Left",	"Move cursor before previous word")
		ADD_ITEM("Ctrl+Right",	"Move cursor after next word")
		ADD_ITEM("Ctrl+Shift+Left",	"Select word to left of cursor")
		ADD_ITEM("Ctrl+Shift+Right","Select word to right of cursor")
		ADD_ITEM("Ctrl+Shift+Up","Select till start of paragraph")
		ADD_ITEM("Ctrl+Shift+Down","Select till end of paragraph")
		ADD_ITEM("Ctrl+C",		"Copy")
		ADD_ITEM("Ctrl+V",		"Paste")
		ADD_ITEM("Ctrl+X",		"Cut")
		ADD_ITEM("Ctrl+Z",		"Undo")
		ADD_ITEM("Ctrl+Y",		"Redo")
	END_LIST()

	return TRUE;
}
