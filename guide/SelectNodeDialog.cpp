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
#include <libguide/wintree.h>
#include "Guide.h"
#include "IconWnd.h"
#include ".\selectnodedialog.h"

IMPLEMENT_DYNAMIC(SelectNodeDialog, CDialog)

SelectNodeDialog::SelectNodeDialog(struct guide_t *guide, struct tree_node_t *node)
	: CDialog(IDD_SELECTNODE)
	, m_pGuide(guide)
	, m_pNode(node)
{
	ASSERT(guide);
	ASSERT(node);
}

SelectNodeDialog::~SelectNodeDialog()
{
	ASSERT(m_pWinTree);
	wintree_destroy(m_pWinTree);
	m_pWinTree = 0;
}

BEGIN_MESSAGE_MAP(SelectNodeDialog, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE, OnNMDblclkTree)
END_MESSAGE_MAP()

// SelectNodeDialog message handlers

BOOL SelectNodeDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	HWND hTree = ::GetDlgItem(m_hWnd, IDC_TREE);

	// set image list
	CImageList& imageList = theIcons.GetImageList();
	TreeView_SetImageList(hTree, imageList.m_hImageList, TVSIL_NORMAL);

	// create a wintree
	m_pWinTree = wintree_create(m_pGuide, hTree, theIcons.GetLeafIdx(), theIcons.GetNonLeafIdx());

	// populate the tree
	wintree_populate(m_pWinTree);

	// select whatever our creator asked us to select
	HTREEITEM hItem = wintree_get_item_from_node(m_pWinTree, m_pNode);
	ASSERT(hItem);
	TreeView_Select(hTree, hItem, TVGN_CARET);

	return TRUE;
}

void SelectNodeDialog::OnNMDblclkTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnOK();

	*pResult = 0;
}

void SelectNodeDialog::OnOK()
{
	HWND hTree = ::GetDlgItem(m_hWnd, IDC_TREE);

	// get selected item
	HTREEITEM hItem = TreeView_GetSelection(hTree);
	ASSERT(hItem);

	// get node for this item
	m_pNode = wintree_get_node_from_item(m_pWinTree, hItem);
	ASSERT(m_pNode);

	CDialog::OnOK();
}
