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
#include "afxwin.h"

struct wintree_t;

class CGuideDoc;
class CLeftViewColorDialog;
class CIconWnd;

enum DragOperation
{
	DO_NONE, DO_BEFORE, DO_AFTER, DO_CHILD
};

class CLeftView : public CTreeView
{
protected: // create from serialization only
	CLeftView();
	DECLARE_DYNCREATE(CLeftView)

// Attributes
private:
	// rmb support
	HTREEITEM m_RClickItem;
	// BEGIN drag-drop support
	HTREEITEM m_hDragItem, m_hDropItem;
	bool m_bDragging;
	CMenu m_DropMenu;
	// END drag-drop support
	struct wintree_t* m_pWinTree;
	// shortcut keys for tree control
	HACCEL m_hAccel;
	// font to use for the tree
	CFont m_oFont;
	// RMB menu
	CMenu m_RMBMenu;
	// no. of children for current node
	int m_nChildCount;
	// current drag operation
	DragOperation m_dragOp;
	// is this a ctrl+drag?
	bool m_bIsCtrlDrag;
	// is this a rmb drag?
	bool m_bIsRMBDrag;
	// icon window
	CIconWnd *m_pIconWnd;

// Overrides
public:
	BOOL PreCreateWindow(CREATESTRUCT& cs);
	BOOL OnPreparePrinting(CPrintInfo* pInfo);
	void OnInitialUpdate();
	void OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	BOOL PreTranslateMessage(MSG* pMsg);
	void OnDestroy();

// Implementation
public:
	~CLeftView();

private:
	CGuideDoc* GetDocument();
	void StopDrag();
	void ChangeSelection(HTREEITEM oldItem, HTREEITEM newItem);
	bool DeleteItem(HTREEITEM hItem);
	void AddChild(HTREEITEM hParent);
	void InsertAbove(HTREEITEM hItem);
	void InsertBelow(HTREEITEM hItem);
	void EnableIfSelected(CCmdUI *pCmdUI);
	void ApplyFont();
	void ApplyColor();
	void SetDragOperation(DragOperation op, HTREEITEM dropItem);
	void StartAutoSaveTimer();
	void CommitNow();
	enum { DLG_FOREGROUND_COLOR, DLG_BACKGROUND_COLOR };
	void ProcessPopupColor(unsigned whichDlg, HTREEITEM hItem);
	void SaveNodeData(HTREEITEM item);
	void LoadNodeData(HTREEITEM item);

// Generated message map functions
protected:
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeAddChild();
	afx_msg void OnTreeRename();
	afx_msg void OnTreeDelete();
	// rmb menu on tree items
	afx_msg void OnRMBAddChild();
	afx_msg void OnRMBAddTop();
	afx_msg void OnRMBRename();
	afx_msg void OnRMBDelete();
	afx_msg void OnRMBAddPageAbove();
	afx_msg void OnRMBAddPageBelow();
	void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	// tree commands via accels
	void OnTreeDelSelected();
	void OnTreeEscKey();
	void OnTreeMoveUp();
	void OnTreeMoveDown();
	void OnTreeShiftFocus();
	void OnTreeInsert();
	void OnTreeInsertAfter();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	// messages from other parts of the application
	LRESULT OnGuideUI(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnRmbSearch();
	afx_msg void OnRmbMovedown();
	afx_msg void OnRmbMoveup();
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnFileExport();
	afx_msg void OnRmbPrint();
	afx_msg void OnRmbPrintpreview();
	afx_msg void OnDropCopy();
	afx_msg void OnDropMove();
	afx_msg void OnTvnBeginrdrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTreeExpandall();
	afx_msg void OnTreeCollapseall();
	afx_msg void OnEditInsertdate();
	afx_msg void OnEditInserttime();
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTreeNodeForeColor();
	afx_msg void OnTreeNodeBackcolor();
	afx_msg void OnRmbNodeForeColor();
	afx_msg void OnRmbNodeBackcolor();
	afx_msg void OnTvnItemexpanded(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRmbNodeicon();
};

inline CGuideDoc* CLeftView::GetDocument()
   { return reinterpret_cast<CGuideDoc*>(m_pDocument); }
