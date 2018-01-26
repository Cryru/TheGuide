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
#include "GuideDoc.h"
#include "FgColorDialog.h"
#include "iconwnd.h"
#include ".\leftview.h"

#include "utils.h"

#include <libguide/wintree.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////
// some constants

#define HOVER_EXPAND_TIMER_ID		1
#define HOVER_EXPAND_TIMER_DELAY	750 /* ms */

#define AUTO_SAVE_TIMER_ID			2

///////////////////////////////////////////////////////////////////////
// globals (loaded once)

static HICON g_hCur_NoDrop;
static HICON g_hCur_Arrow;
static HICON g_hCur_Copy;

///////////////////////////////////////////////////////////////////////
// static helper functions

// ignore (64-bit compatibility) warnings related to conversion of
// pointer to DWORD and vice versa.
#pragma warning( push )
#pragma warning( disable : 4311 ; disable : 4312 )

static DWORD __stdcall streamOutLengthCalcCallback(
	DWORD dwCookie, LPBYTE pbBuff,
    LONG cb, LONG *pcb)
{
	size_t *lenp = reinterpret_cast<size_t *>(dwCookie);
	if (cb > 0)
	{
		*lenp += cb;
		return 0;
	}

	return -1;
}

struct streamOutParams
{
	char *start;
	size_t offset;

	streamOutParams(char *start_)
		: start(start_), offset(0)
	{ }
};

static DWORD __stdcall streamOutCallback(
	DWORD dwCookie, LPBYTE pbBuff,
    LONG cb, LONG *pcb)
{
	streamOutParams *params = reinterpret_cast<streamOutParams *>(dwCookie);
	*pcb = cb;

	if (cb > 0)
	{
		memcpy(params->start + params->offset, pbBuff, cb);
		params->offset += (size_t) cb;
		return 0;
	}

	return -1;
}

struct streamInParams
{
	const CStringA& rtf;
	size_t used;

	streamInParams(const CStringA& rtf_)
		: rtf(rtf_), used(0)
	{ }
};

static DWORD __stdcall streamInCallback(
	DWORD dwCookie, LPBYTE pbBuff,
    LONG cb, LONG *pcb)
{
	streamInParams *params = reinterpret_cast<streamInParams *>(dwCookie);

	if (cb > 0)
	{
		size_t remain = (size_t)params->rtf.GetLength() - params->used;
		size_t n = __min(remain, (size_t)cb);
		memcpy(pbBuff, (const char *)(params->rtf) + params->used, n);
		params->used += n;
		*pcb = (LONG)n;
		return 0;
	}

	*pcb = 0;
	return -1;
}

CStringA GetRTF(HWND hwnd)
{
	// first calculate the no. of bytes required
	// (it's faster this way)
	size_t reqdBytes = 0;

	EDITSTREAM es;
	es.dwCookie = reinterpret_cast<DWORD>(&reqdBytes);
	es.dwError = 0;
	es.pfnCallback = streamOutLengthCalcCallback;
	::SendMessage(hwnd, EM_STREAMOUT, (WPARAM)SF_RTF, (LPARAM)&es);

	// now allocate sufficient memory and the stream it out
	// into that
	CStringA out;
	char *outp = out.GetBuffer((int)reqdBytes + 1);

	streamOutParams params(outp);
	es.dwCookie = reinterpret_cast<DWORD>(&params);
	es.dwError = 0;
	es.pfnCallback = streamOutCallback;

	::SendMessage(hwnd, EM_STREAMOUT, (WPARAM)SF_RTF, (LPARAM)&es);

	// finalize CString
	outp[reqdBytes] = 0;
	out.ReleaseBuffer((int)reqdBytes);

	return out;
}

static void FixSomeStuff(HWND hwnd)
{
	// clear CFE_LINK and CFE_PROTECTED effects
	CHARFORMAT2 cf2;
	memset(&cf2, 0, sizeof(cf2));
	cf2.cbSize = sizeof(cf2);
	cf2.dwMask = CFM_LINK | CFM_PROTECTED;
	::SendMessage(hwnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf2);

	// clear numbering effects
	PARAFORMAT2 pf2;
	memset(&pf2, 0, sizeof(pf2));
	// bug fix 2.0b1+: set pf2.cbSize field
	pf2.cbSize = sizeof(pf2);
	pf2.wNumbering = 0;
	pf2.wNumberingStyle = 0;
	pf2.wNumberingStart = 0;
	pf2.dxOffset = 0;
	pf2.dxStartIndent = 0;
	pf2.dwMask = PFM_NUMBERINGSTART | PFM_NUMBERINGSTYLE | 
			PFM_NUMBERING | PFM_STARTINDENT | PFM_OFFSET;
	::SendMessage(hwnd, EM_SETPARAFORMAT, SCF_ALL, (LPARAM)&pf2);
}

void SetRTF(HWND hwnd, const CStringA& rtf, bool fixStyle)
{
	// The next line is required for working around some issues
	// (bugs?): the protect, link and numbering and turned on
	// by default it they were currently on. To fix this, turn
	// them off before streaming in the new text.
	if (fixStyle)
	{
		FixSomeStuff(hwnd);
	}

	streamInParams params(rtf);
	EDITSTREAM es;
	es.dwCookie = reinterpret_cast<DWORD>(&params);
	es.dwError = 0;
	es.pfnCallback = streamInCallback;

	::SendMessage(hwnd, EM_STREAMIN, (WPARAM)SF_RTF, (LPARAM)&es);
}

void SetRTFSelection(HWND hwnd, const CStringA& rtf)
{
	streamInParams params(rtf);
	EDITSTREAM es;
	es.dwCookie = reinterpret_cast<DWORD>(&params);
	es.dwError = 0;
	es.pfnCallback = streamInCallback;

	::SendMessage(hwnd, EM_STREAMIN, (WPARAM)SF_RTF|SFF_SELECTION, (LPARAM)&es);
}

// restore warning state
#pragma warning( pop )

///////////////////////////////////////////////////////////////////////
// CLeftView implementation

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
	// RMB menu command handlers
	ON_COMMAND(ID_RMB_ADDTOP, OnRMBAddTop)
	ON_COMMAND(ID_RMB_ADDCHILD, OnRMBAddChild)
	ON_COMMAND(ID_RMB_RENAME, OnRMBRename)
	ON_COMMAND(ID_RMB_DELETE, OnRMBDelete)
	ON_COMMAND(ID_RMB_ADDPAGEABOVE, OnRMBAddPageAbove)
	ON_COMMAND(ID_RMB_ADDPAGEBELOW, OnRMBAddPageBelow)
	ON_COMMAND(ID_RMB_SEARCH, OnRmbSearch)
	// Tree menu command handlers
	ON_COMMAND(ID_TREE_ADDTOP, OnRMBAddTop)
	ON_COMMAND(ID_TREE_ADDCHILDPAGE, OnTreeAddChild)
	ON_COMMAND(ID_TREE_RENAMEPAGE, OnTreeRename)
	ON_COMMAND(ID_TREE_DELETEPAGE, OnTreeDelete)
	ON_COMMAND(ID_TREE_ADDPAGEABOVE, OnTreeInsert)
	ON_COMMAND(ID_TREE_ADDPAGEBELOW, OnTreeInsertAfter)
	// Tree menu update handlers
	ON_UPDATE_COMMAND_UI(ID_TREE_ADDCHILDPAGE, EnableIfSelected)
	ON_UPDATE_COMMAND_UI(ID_TREE_ADDPAGEABOVE, EnableIfSelected)
	ON_UPDATE_COMMAND_UI(ID_TREE_ADDPAGEBELOW, EnableIfSelected)
	ON_UPDATE_COMMAND_UI(ID_TREE_RENAMEPAGE, EnableIfSelected)
	ON_UPDATE_COMMAND_UI(ID_TREE_DELETEPAGE, EnableIfSelected)
	ON_UPDATE_COMMAND_UI(ID_TREE_SEARCH, EnableIfSelected)
	// Tree commands invoked via accelarators
	ON_COMMAND(ID_TREE_DELETESELECTED, OnTreeDelSelected)
	ON_COMMAND(ID_TREE_ESCKEY, OnTreeEscKey)
	ON_COMMAND(ID_TREE_MOVEUP, OnTreeMoveUp)
	ON_COMMAND(ID_TREE_MOVEDOWN, OnTreeMoveDown)
	ON_COMMAND(ID_TREE_SHIFTFOCUS, OnTreeShiftFocus)
	ON_COMMAND(ID_TREE_INSERT, OnTreeInsert)
	ON_COMMAND(ID_TREE_INSERTAFTER, OnTreeInsertAfter)
	// Our own messages
	ON_MESSAGE(WM_GUIDEUI, OnGuideUI)
	// For cleanup
	ON_WM_DESTROY()
	// Tree control notifications
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnTvnBegindrag)
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(ID_TREE_MOVEUP, EnableIfSelected)
	ON_UPDATE_COMMAND_UI(ID_TREE_MOVEDOWN, EnableIfSelected)
	ON_COMMAND(ID_RMB_MOVEDOWN, OnRmbMovedown)
	ON_COMMAND(ID_RMB_MOVEUP, OnRmbMoveup)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrint)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, EnableIfSelected)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, EnableIfSelected)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT, EnableIfSelected)
	ON_COMMAND(ID_RMB_PRINT, OnRmbPrint)
	ON_COMMAND(ID_RMB_PRINTPREVIEW, OnRmbPrintpreview)
	ON_COMMAND(ID_DROP_COPY, OnDropCopy)
	ON_COMMAND(ID_DROP_MOVE, OnDropMove)
	ON_NOTIFY_REFLECT(TVN_BEGINRDRAG, OnTvnBeginrdrag)
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_TREE_EXPANDALL, OnTreeExpandall)
	ON_UPDATE_COMMAND_UI(ID_TREE_EXPANDALL, EnableIfSelected)
	ON_COMMAND(ID_TREE_COLLAPSEALL, OnTreeCollapseall)
	ON_UPDATE_COMMAND_UI(ID_TREE_COLLAPSEALL, EnableIfSelected)
	ON_COMMAND(ID_EDIT_INSERTDATE, OnEditInsertdate)
	ON_COMMAND(ID_EDIT_INSERTTIME, OnEditInserttime)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw)
	// Node foreground/background colors
	ON_COMMAND(ID_RMB_COLOR, OnRmbNodeForeColor)
	ON_COMMAND(ID_RMB_BGCOLOR, OnRmbNodeBackcolor)
	ON_COMMAND(ID_TREE_COLOR, OnTreeNodeForeColor)
	ON_UPDATE_COMMAND_UI(ID_TREE_COLOR, EnableIfSelected)
	ON_COMMAND(ID_TREE_BGCOLOR, OnTreeNodeBackcolor)
	ON_UPDATE_COMMAND_UI(ID_TREE_BGCOLOR, EnableIfSelected)
	//
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnTvnItemexpanded)
	ON_COMMAND(ID_RMB_NODEICON, OnRmbNodeicon)
	ON_COMMAND(ID_TREE_NODEICON, OnRmbNodeicon)
	ON_UPDATE_COMMAND_UI(ID_TREE_NODEICON, EnableIfSelected)
END_MESSAGE_MAP()

CLeftView::CLeftView()
	: m_RClickItem(0), m_hDragItem(0)
	, m_hDropItem(0)
	, m_bDragging(0), m_pWinTree(0)
	, m_hAccel(0), m_nChildCount(0)
	, m_dragOp(DO_NONE), m_bIsCtrlDrag(false)
	, m_bIsRMBDrag(false)
	, m_pIconWnd(0)
{
	m_RMBMenu.LoadMenu(IDR_TREE_MENU);
	m_DropMenu.LoadMenu(IDR_DROP_MENU);

	g_hCur_NoDrop = theApp.LoadCursor(IDC_NODROP);
	g_hCur_Arrow = ::LoadCursor(NULL, IDC_ARROW);
	g_hCur_Copy = theApp.LoadCursor(IDC_POINTER_COPY);
}

CLeftView::~CLeftView()
{
	m_RMBMenu.DestroyMenu();
	m_DropMenu.DestroyMenu();

	::DestroyCursor(g_hCur_NoDrop);
	::DestroyCursor(g_hCur_Arrow);
	::DestroyCursor(g_hCur_Copy);
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	TreeDecoration deco = theApp.GetTreeDecoration();
	DWORD checkbox = (SHOW_CHECKBOXES(deco)) ? TVS_CHECKBOXES : 0;

	cs.style |= TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT|
				TVS_SHOWSELALWAYS|TVS_EDITLABELS|checkbox;
	return CTreeView::PreCreateWindow(cs);
}

BOOL CLeftView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

static HIMAGELIST GetStateImageList()
{
	static HIMAGELIST himl;
	if (himl)
		return himl;

	himl = ImageList_Create(16, 16, ILC_COLOR24|ILC_MASK, 3, 0);
	HBITMAP hbmp = (HBITMAP) LoadImage(AfxGetInstanceHandle(), 
		MAKEINTRESOURCE(IDB_STATE_ICONS), IMAGE_BITMAP, 0, 0, 0);
	ImageList_AddMasked(himl, hbmp, RGB(255,255,0));
	return himl;
}

void CLeftView::OnInitialUpdate()
{
	CTreeCtrl& theCtrl = GetTreeCtrl();
	CTreeView::OnInitialUpdate();

	// set image lists: image [note: this pref change needs app restart,
	// see MSDN for TVS_CHECKBOXES style]
	static TreeDecoration deco = theApp.GetTreeDecoration();
	if (SHOW_ICONS(deco))
		theCtrl.SetImageList(&theIcons.GetImageList(), TVSIL_NORMAL);
	else
		theCtrl.SetImageList(NULL, TVSIL_NORMAL);

	// do we want icons? checkboxes? both?
	if (SHOW_CHECKBOXES(deco))
	{
		// set window style if we're using TVS_CHECKBOXES (reqd by MSDN)
		SetWindowLong(m_hWnd, GWL_STYLE, TVS_CHECKBOXES | GetWindowLong(m_hWnd, GWL_STYLE));

		// set image lists: state
		theCtrl.SendMessage(TVM_SETIMAGELIST, TVSIL_STATE, (LPARAM) GetStateImageList());
	}
	else
	{
		// set window style if we're using TVS_CHECKBOXES (reqd by MSDN)
		DWORD style = GetWindowLong(m_hWnd, GWL_STYLE);
		style &= ~(TVS_CHECKBOXES);
		SetWindowLong(m_hWnd, GWL_STYLE, style);
	}

	// load accelarator table
	if (m_hAccel == NULL)
	{
		m_hAccel = LoadAccelerators(AfxGetInstanceHandle(), 
			MAKEINTRESOURCE(IDR_TREE_ACCEL));
		ASSERT(m_hAccel);
	}

	// create wintree
	ASSERT(!m_pWinTree); // ui tree should be empty
	struct guide_t *guide = GetDocument()->m_pGuide;
	ASSERT(guide); // doc tree should have been created
	m_pWinTree = wintree_create(guide, theCtrl, theIcons.GetLeafIdx(), theIcons.GetNonLeafIdx());
	ASSERT(m_pWinTree);
	wintree_populate(m_pWinTree);

	// get the selected node
	struct tree_node_t *sel_node = guide->sel_node;
	// if no node selected (v1 file format), use the root node
	HTREEITEM hSelItem =
		sel_node ? 
			wintree_get_item_from_node(m_pWinTree, sel_node) :
			theCtrl.GetRootItem();
	if (hSelItem)
	{
		// restore selection
		ChangeSelection(NULL, hSelItem);
		theCtrl.Select(hSelItem, TVGN_CARET);
	}

	// register our handle with the app
	theApp.SetLeftWnd(m_hWnd);
	theApp.SetLeftView(this);

	// Enable/disable right view depending on item count
	BOOL bEnable = theCtrl.GetCount() > 0;
	::EnableWindow(theApp.GetRightWnd(), bEnable);
	if (!bEnable)
		// make ourselves as the active view if the right one is disabled
		((CFrameWnd *)AfxGetMainWnd())->SetActiveView(this);

	// Clear the RichEdit control's modified flag, and also the
	// document's modified flag.
	GetDocument()->SetModifiedFlag(FALSE);
	::SendMessage(theApp.GetRightWnd(), EM_SETMODIFY, FALSE, 0);
	// Note: the right wnd's OnInitialUpdate() gets called first.

	// apply the configured color and font
	ApplyColor();
	ApplyFont();

	// start autosave timer
	StartAutoSaveTimer();
}

void CLeftView::OnDestroy() 
{
	// base class destroy
	CTreeView::OnDestroy();

	// accelarator table
	VERIFY(DestroyAcceleratorTable(m_hAccel));

	// destroy old font, if any
	if (m_oFont.GetSafeHandle() != NULL)
		m_oFont.DeleteObject();
	ASSERT(m_oFont.GetSafeHandle() == NULL);
}

void CLeftView::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM hOld = pNMTreeView->itemOld.hItem;
	HTREEITEM hNew = pNMTreeView->itemNew.hItem;

	ChangeSelection(hOld, hNew);

	*pResult = 0;
}

static CPoint GetClickPosition()
{
	DWORD aPos = GetMessagePos();
	int x = aPos & 0xffff;
	int y = (aPos & 0xffff0000) >> 16;
	if (aPos & 0x80000000)
		y |= 0xffff0000;
	return CPoint(x, y);
}

static HTREEITEM GetItemFromPoint(CTreeCtrl& tree, const CPoint& point)
{
	CPoint aMappedPoint(point);
	UINT aFlags = 0;
	::MapWindowPoints(HWND_DESKTOP, tree, &aMappedPoint, 1);
	return tree.HitTest(aMappedPoint, &aFlags);
}

static void enableMenuItem(CMenu *pMenu, UINT id, bool enable)
{
	UINT opts = enable ? (MF_ENABLED) : (MF_DISABLED | MF_GRAYED);
	opts |= MF_BYCOMMAND;

	pMenu->EnableMenuItem(id, opts);
}

void CLeftView::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// if drag is in progress, do nothing
	if (m_bDragging)
	{
		*pResult = 0;
		return;
	}

	// get the sub menu
	CMenu *aTreeMenu = m_RMBMenu.GetSubMenu(0);

	// get the tree item at the clicked position
	CPoint aPoint = GetClickPosition();
	HTREEITEM hItem = GetItemFromPoint(GetTreeCtrl(), aPoint);
	// if that is not available (clicked below last item),
	// use the selected item
	if (!hItem)
		hItem = GetTreeCtrl().GetSelectedItem();

	// save it for later use
	m_RClickItem = hItem;

	// disable some menu entries based on selection
	bool enable1 = hItem != NULL;
	enableMenuItem(aTreeMenu, ID_RMB_ADDCHILD,		enable1);
	enableMenuItem(aTreeMenu, ID_RMB_ADDPAGEABOVE,	enable1);
	enableMenuItem(aTreeMenu, ID_RMB_ADDPAGEBELOW,	enable1);
	enableMenuItem(aTreeMenu, ID_RMB_RENAME,		enable1);
	enableMenuItem(aTreeMenu, ID_RMB_DELETE,		enable1);
	enableMenuItem(aTreeMenu, ID_RMB_MOVEUP,		enable1);
	enableMenuItem(aTreeMenu, ID_RMB_MOVEDOWN,		enable1);
	enableMenuItem(aTreeMenu, ID_RMB_PRINT,			enable1);
	enableMenuItem(aTreeMenu, ID_RMB_PRINTPREVIEW,	enable1);
	enableMenuItem(aTreeMenu, ID_RMB_COLOR,			enable1);
	enableMenuItem(aTreeMenu, ID_RMB_BGCOLOR,		enable1);
	enableMenuItem(aTreeMenu, ID_RMB_NODEICON,		enable1);

	// disable some menu entries based on any item present in tree
	bool enable2 = GetTreeCtrl().GetRootItem() != NULL;
	enableMenuItem(aTreeMenu, ID_RMB_SEARCH, enable2);

	// show the menu
	aTreeMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, 
		aPoint.x, aPoint.y, this);

	// success
	*pResult = 0;
}

void CLeftView::AddChild(HTREEITEM hParent)
{
	HTREEITEM hItem = wintree_add_child(
		m_pWinTree, hParent, RCSTR(IDS_PAGE_NEWCHILD), theEmptyRTFText);
	ASSERT(hItem);
	CTreeCtrl& tree = GetTreeCtrl();
	// BUG FIX: the selection changes from current selected item
	// to hItem not from hParent to hItem
//	ChangeSelection(tree.GetSelectedItem(), hItem);
	tree.Select(hItem, TVGN_CARET);
	theApp.GetRightView()->SendMessage(WM_GUIDEUI, GM_NEW_PAGE);
	tree.EditLabel(hItem);
	GetDocument()->SetModifiedFlag();
}

void CLeftView::OnRMBAddChild()
{
	AddChild(m_RClickItem);
	m_RClickItem = NULL;
}

void CLeftView::OnRMBAddTop()
{
	HTREEITEM hItem = wintree_add_toplevel(m_pWinTree, RCSTR(IDS_PAGE_NEWTOP),
		theEmptyRTFText);
	ASSERT(hItem);
	CTreeCtrl& tree = GetTreeCtrl();
	tree.Select(hItem, TVGN_CARET);
	theApp.GetRightView()->SendMessage(WM_GUIDEUI, GM_NEW_PAGE);
	tree.EditLabel(hItem);

	GetDocument()->SetModifiedFlag();
}

void CLeftView::OnRMBRename()
{
	if (m_RClickItem != NULL)
	{
		// via RMB menu
		GetTreeCtrl().EditLabel(m_RClickItem);
		m_RClickItem = NULL;
	}
	else
	{
		// via <F2>
		CTreeCtrl& tree = GetTreeCtrl();
		HTREEITEM selItem = tree.GetSelectedItem();
		if (selItem)
			tree.EditLabel(selItem);
	}
}

bool CLeftView::DeleteItem(HTREEITEM hItem)
{
	if (AfxMessageBox(IDS_CONFIRM_DELETE, 
			MB_YESNO, MB_ICONEXCLAMATION) == IDYES)
	{
		// delete from wintree
		wintree_delete(m_pWinTree, hItem);

		// set document as modified
		GetDocument()->SetModifiedFlag();

		// BUG FIX: Update right view's text when deletion happens
		// (note: the GetSelectedItem() might return NULL if we
		// deleted the only node of the tree)
		ChangeSelection(NULL, GetTreeCtrl().GetSelectedItem());

		return true;
	}

	return false;
}

void CLeftView::OnRMBDelete()
{
	DeleteItem(m_RClickItem);
	m_RClickItem = NULL;
}

static int ChildCounter(struct tree_node_t *node, void *cargo)
{
	int *pCount = (int *)cargo;
	(*pCount)++;

	return 0;
}

static int GetChildCount(struct wintree_t *pWinTree, HTREEITEM item)
{
	ASSERT(pWinTree);
	ASSERT(item);

	// get tree node
	struct tree_node_t *node = wintree_get_node_from_item(pWinTree, item);
	ASSERT(node);

	// traverse subtree
	int count = 0;
	tree_traverse_subtree_preorder(node, ChildCounter, &count);

	// exclude self in count
	return count - 1;
}

// extract data from the right view and save it into the guide
void CLeftView::SaveNodeData(HTREEITEM item)
{
	ASSERT(item);

	// get the rtf window
	CView *pRtfView = theApp.GetRightView();
	// get the node
	struct tree_node_t *node = wintree_get_node_from_item(m_pWinTree, item);
	ASSERT(node);
	// get the node data
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);
	ASSERT(data);

	// (1) store the rtf
	CStringA rtf = GetRTF(pRtfView->m_hWnd);
	wintree_set_text(m_pWinTree, item, rtf);

	// (2) store the index of the first visible line
	unsigned lineIdx = (unsigned)pRtfView->SendMessage(EM_GETFIRSTVISIBLELINE);
	data->first_line = lineIdx;
}

// take data from the node and put it into the right view
void CLeftView::LoadNodeData(HTREEITEM item)
{
	ASSERT(item);

	// get the rtf window
	CView *pRtfView = theApp.GetRightView();
	// get the node
	struct tree_node_t *node = wintree_get_node_from_item(m_pWinTree, item);
	ASSERT(node);
	// get the node data
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);
	ASSERT(data);

	// (1) restore the rtf
	const char *newText = wintree_get_text(m_pWinTree, item);
	SetRTF(pRtfView->m_hWnd, newText);

	// (2) restore the index of the first visible line
	pRtfView->SendMessage(EM_LINESCROLL, 0, data->first_line);

	// (*) remember current selection within the guide itself
	wintree_get_guide(m_pWinTree)->sel_node = node;
}

void CLeftView::ChangeSelection(HTREEITEM oldItem, HTREEITEM newItem)
{
	// Remember the document's modified flag before we do the
	// modification.
	BOOL bMod = GetDocument()->IsModified();

	// rtf window
	HWND hWnd = theApp.GetRightWnd();

	// v1.6: things might take a while if we've (big) images on 
	// pages. Show a wait cursor.
	CWaitCursor aWaitCursor;

	// get current text and put it in old item's node
	if (oldItem)
		SaveNodeData(oldItem);

	// get text from new item's node and put it in window
	if (newItem)
		LoadNodeData(newItem);

	// calculate child count
	m_nChildCount = newItem ? GetChildCount(m_pWinTree, newItem) : 0;

	// Enable/disable right view depending on item count
	BOOL bEnable = GetTreeCtrl().GetCount() > 0;
	if (bEnable == FALSE)
	{
		// if we're going to disable it, put the standard text 
		SetRTF(hWnd, theStartupRTFText);
	}
	::EnableWindow(hWnd, bEnable);

	// Reset modification state.
	GetDocument()->SetModifiedFlag(bMod);
}

void CLeftView::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	LPCTSTR szText = pTVDispInfo->item.pszText;
	HTREEITEM hItem = pTVDispInfo->item.hItem;
	LPARAM lParam = pTVDispInfo->item.lParam;

	*pResult = 0;
	if (szText != NULL)
	{
		if (szText[0] == 0)
		{
			AfxMessageBox(IDS_PAGENAME_NOEMPTY, MB_ICONEXCLAMATION);
			GetTreeCtrl().EditLabel(hItem);
		}
		else
		{
			wintree_rename_item(m_pWinTree, hItem, szText);
			*pResult = 1;
			GetDocument()->SetModifiedFlag();
		}
	}
}

struct _TitleSearchParams
{
	LPCTSTR title;
	HTREEITEM result;
};

static int _TitleSearcher(HTREEITEM item, struct tree_node_t *node, void *cargo)
{
	_TitleSearchParams *params = (_TitleSearchParams *)cargo;
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);

	// ignore leading and trailing spaces/newlines, and also ignore
	// case during comparison.
	CString betterTitle = data->title;
	betterTitle.Trim(L" \t\r\n");
	if (betterTitle.CompareNoCase(params->title) == 0)
	{
		params->result = item;
		return 1;
	}

	return 0;
}

static HTREEITEM FindNode(struct wintree_t *wintree, LPCTSTR title)
{
	// slightly pre-process the link so that we accomodate the
	// poor users' inclusion of surrounding spaces/newlines
	// within the link.
	CString betterTitle = title;
	betterTitle.Trim(L" \t\r\n");

	// actually search
	_TitleSearchParams params = { (LPCTSTR)betterTitle, NULL };
	wintree_traverse(wintree, _TitleSearcher, &params);

	// result is stored back into params.result by the c/b
	return params.result;
}

static int _state_setter(HTREEITEM item, struct tree_node_t *node, void *cargo)
{
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);
	CTreeCtrl *treeCtrl = (CTreeCtrl *)cargo;

	TVITEM tvitem;
	memset(&tvitem, 0, sizeof(tvitem));
	tvitem.hItem = item;
	tvitem.mask = TVIF_HANDLE;
	tvitem.stateMask = TVIS_STATEIMAGEMASK;
	treeCtrl->GetItem(&tvitem);

	data->tc_state = tvitem.state;

	return 0;
}

void CLeftView::CommitNow()
{
	// save RTF
	HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
	if (hItem)
	{
		CStringA text = GetRTF(theApp.GetRightWnd());
		wintree_set_text(m_pWinTree, hItem, text);
	}

	// save checked state
	wintree_traverse(m_pWinTree, _state_setter, &GetTreeCtrl());
}

LRESULT CLeftView::OnGuideUI(WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	switch (wParam)
	{
		case GM_COMMIT_RTF_NOW:
		{
			CommitNow();
		}
		break;

		case GM_ABOUT_TO_DELETE:
		{
			if (m_pWinTree)
			{
				// bug fix: m_pWinTree can be null when previous
				// document failed to open (MRU file was deleted
				// for e.g.)
				wintree_destroy(m_pWinTree);
				m_pWinTree = 0;
			}
		}
		break;

		case GM_APPLY_FONT:
		{
			ApplyFont();
		}
		break;

		case GM_APPLY_COLOR:
		{
			ApplyColor();
		}
		break;

		case GM_SELECT_NODE:
		{
			// search entire tree for node with the given (in LPARAM)
			// text for its title
			HTREEITEM item = FindNode(m_pWinTree, (LPCTSTR)lParam);
			if (item)
			{
				GetTreeCtrl().SelectItem(item);
				result = 1;
			}
		}
		break;

		case GM_SELECT_NODE_BY_UID:
		{
			struct tree_node_t *node = 
				guide_get_node_by_uid(GetDocument()->m_pGuide, (uint32)lParam);
			// ASSERT(node);
			// node can be NULL if the target was deleted for e.g.
			if (node)
			{
				HTREEITEM hItem = wintree_get_item_from_node(m_pWinTree, node);
				ASSERT(hItem);
				GetTreeCtrl().SelectItem(hItem);
				result = 1;
			}
		}
		break;

		case GM_GET_WINTREE:
		{
			result = (LRESULT) m_pWinTree; // not 64-bit safe?
		}
		break;

		case GM_GET_CHILD_COUNT:
		{
			result = (LRESULT) m_nChildCount;
		}
		break;

		case GM_APPLY_AUTOSAVE:
		{
			StartAutoSaveTimer();
		}
		break;

		case GM_SET_NODE_FGCOLOR:
		case GM_SET_NODE_BGCOLOR:
		{
			// get the node
			SetNodeColorInput *input = (SetNodeColorInput *)lParam;
			struct tree_node_t *node = wintree_get_node_from_item(m_pWinTree, input->hItem);
			struct guide_nodedata_t *data = 
				(struct guide_nodedata_t *)tree_get_data(node);

			// set the color into the data structure
			if (wParam == GM_SET_NODE_FGCOLOR)
				data->color = input->color;
			else
				data->bgcolor = input->color;

			// repaint the item
			CRect r;
			GetTreeCtrl().GetItemRect(input->hItem, &r, FALSE);
			InvalidateRect(&r, FALSE);
		}
		break;

		case GM_GET_SELECTED_NODE:
		{
			HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
			result = (LRESULT) wintree_get_node_from_item(m_pWinTree, hItem);
		}
		break;

		case GM_ICONWND_DESTROYED:
		{
			m_pIconWnd = 0;
		}
		break;

		case GM_SET_NODE_ICON:
		{
			HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
			if (hItem == NULL)
				break;

			// get node and data
			struct tree_node_t *node = wintree_get_node_from_item(m_pWinTree, hItem);
			struct guide_nodedata_t *data = 
				(struct guide_nodedata_t *)tree_get_data(node);
			// update data
			data->icon = (uint32) lParam;
			// update view
			wintree_update_icon(m_pWinTree, hItem);
			// mark document as modified
			GetDocument()->SetModifiedFlag(TRUE);
		}
		break;
	}

	return result;
}

void CLeftView::OnTreeAddChild()
{
	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM selItem = tree.GetSelectedItem();
	if (selItem)
		AddChild(selItem);
}

void CLeftView::OnTreeRename()
{
	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM selItem = tree.GetSelectedItem();
	if (selItem)
		tree.EditLabel(selItem);
}

void CLeftView::OnTreeDelete()
{
	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM selItem = tree.GetSelectedItem();
	if (selItem)
		DeleteItem(selItem);
}

void CLeftView::SetDragOperation(DragOperation op, HTREEITEM dropItem)
{
	// get the tree control
	CTreeCtrl& ctrl = GetTreeCtrl();

	// remember the values
	m_dragOp = op;
	m_hDropItem = dropItem;

	// if no operation, set the evil cursor, a status bar text and return
	if (op == DO_NONE)
	{
		m_hDropItem = NULL;
		SetCursor(g_hCur_NoDrop);
		theApp.SetStatusBarText(IDS_NODROP);
		return;
	}

	// process insert mark
	if (op != DO_NONE)
		ctrl.SetInsertMark(NULL);
	if (op == DO_BEFORE || op == DO_AFTER)
		ctrl.SetInsertMark(dropItem, op == DO_AFTER);

	// set timer if required
	if (dropItem && ctrl.GetChildItem(dropItem))
		SetTimer(HOVER_EXPAND_TIMER_ID, HOVER_EXPAND_TIMER_DELAY, 0);

	// set the drop target
	ctrl.SelectDropTarget(dropItem);

	// check if the user is trying to copy a node beneath itself
	HTREEITEM node = dropItem;
	while (node && node != m_hDragItem)
		node = ctrl.GetParentItem(node);
	if (node == m_hDragItem)
	{
		m_dragOp = DO_NONE;
		m_hDropItem = NULL;
		theApp.SetStatusBarText(IDS_NOBENEATH);
		SetCursor(g_hCur_NoDrop);
		return;
	}

	// OK to show a normal cursor
	SetCursor(m_bIsCtrlDrag ? g_hCur_Copy : g_hCur_Arrow);

	// status bar stuff.
	// 1. get the source and dest strings
	CString srcText, dstText;
	if (m_hDragItem) srcText = ctrl.GetItemText(m_hDragItem);
	if (m_hDropItem) dstText = ctrl.GetItemText(m_hDropItem);
	// 2. get the operation string
	static const UINT opDesc[] = {
//		_T("*"), _T("before"), _T("after"), _T("as the first child of")
		0, IDS_DRAG_SBAR1, IDS_DRAG_SBAR2, IDS_DRAG_SBAR3
	};
	// 3. get the children indicator
	CString c;
	if (m_hDragItem && ctrl.GetChildItem(m_hDragItem) != NULL)
		c = RCSTR(IDS_DRAG_SBAR4);
	// 4. get the move/copy/move-or-copy text
	CString verb;
	if (m_bIsRMBDrag)
		verb = RCSTR(IDS_DRAG_SBAR5);	// "Move/copy"
	else if (m_bIsCtrlDrag)
		verb = RCSTR(IDS_DRAG_SBAR6);	// "Copy"
	else
		verb = RCSTR(IDS_DRAG_SBAR7);	// "Move"
	// 4. form the text
	CString t;
	t.Format(L"%s \"%s\"%s %s \"%s\"", verb, srcText, c, RCSTR(opDesc[op]), 
		dstText);
	// 5. set it
	theApp.SetStatusBarText(t);
}

void CLeftView::OnMouseMove(UINT nFlags, CPoint point)
{
	// we use mouse move event only during drag-drop
	if (!m_bDragging)
		return;

	// get the tree control
	CTreeCtrl& ctrl = GetTreeCtrl();

	// kill any running hover expand timer
	KillTimer(HOVER_EXPAND_TIMER_ID);

	// do the hittest
	UINT flags = 0;
	HTREEITEM hItem = ctrl.HitTest(point, &flags);
	bool onItem = (flags & 
			(TVHT_ONITEMBUTTON|TVHT_ONITEMICON|TVHT_ONITEMINDENT|
			 TVHT_ONITEMLABEL|TVHT_ONITEMRIGHT|TVHT_ONITEMSTATEICON)) != 0;

	// see what hit..
	if (onItem)
	{
		// get item rect and calculate delta
		CRect r;
		ctrl.GetItemRect(hItem, &r, FALSE);
		int delta = r.Height() / 3;

		if ( (point.y - r.top) < delta )
		{
			// before
			SetDragOperation(DO_BEFORE, hItem);
		}
		else if ( (r.bottom - point.y) < delta )
		{
			// after (or child)
			HTREEITEM nextVisible = ctrl.GetNextVisibleItem(hItem);
			if (nextVisible && nextVisible == ctrl.GetChildItem(hItem))
				SetDragOperation(DO_CHILD, hItem);
			else
				SetDragOperation(DO_AFTER, hItem);
		}
		else
		{
			// first child
			SetDragOperation(DO_CHILD, hItem);
		}
	}
	else if (flags & TVHT_ABOVE)
	{
		ctrl.SendMessage(WM_VSCROLL, SB_LINEUP);
		SetDragOperation(DO_NONE, NULL);
	}
	else if (flags & TVHT_BELOW)
	{
		ctrl.SendMessage(WM_VSCROLL, SB_LINEDOWN);
		SetDragOperation(DO_NONE, NULL);
	}
	else if (flags & TVHT_TOLEFT)
	{
		ctrl.SendMessage(WM_HSCROLL, SB_LINELEFT);
		SetDragOperation(DO_NONE, NULL);
	}
	else if (flags & TVHT_TORIGHT)
	{
		ctrl.SendMessage(WM_HSCROLL, SB_LINERIGHT);
		SetDragOperation(DO_NONE, NULL);
	}
	// if TVHT_NOWHERE, don't do anything

	CTreeView::OnMouseMove(nFlags, point);
}

void CLeftView::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	// commit current text if required
	CommitNow();

	// get click point in client-coordinates
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	// set our state to "drag progress"
	m_bDragging = true;
	m_hDragItem = pNMTreeView->itemNew.hItem;
	m_hDropItem = NULL;
	m_dragOp = DO_NONE;
	m_bIsCtrlDrag = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	m_bIsRMBDrag = false;

	CTreeCtrl& tCtrl = GetTreeCtrl();
	tCtrl.SelectItem(m_hDragItem);

	// drawing stuff
	tCtrl.SelectDropTarget(NULL);
	SetCapture();

	*pResult = 0;
}

void CLeftView::OnTvnBeginrdrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	// commit current text if required
	CommitNow();

	// get click point in client-coordinates
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	// set our state to "drag progress"
	m_bDragging = true;
	m_hDragItem = pNMTreeView->itemNew.hItem;
	m_hDropItem = NULL;
	m_dragOp = DO_NONE;
	m_bIsCtrlDrag = false;
	m_bIsRMBDrag = true;

	CTreeCtrl& tCtrl = GetTreeCtrl();
	tCtrl.SelectItem(m_hDragItem);

	// drawing stuff
	tCtrl.SelectDropTarget(NULL);
	SetCapture();

	*pResult = 0;
}

void CLeftView::StopDrag()
{
	ASSERT(m_bDragging);

	// reset drawing stuff
	CTreeCtrl& tCtrl = GetTreeCtrl();
	tCtrl.SelectDropTarget(NULL);
	tCtrl.SetInsertMark(NULL);
	ReleaseCapture();

	// stop the "hover expand" timer
	KillTimer(HOVER_EXPAND_TIMER_ID);

	// reset state
	m_bDragging = false;
}

void CLeftView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		// stop the drag
		StopDrag();

		if (m_dragOp != DO_NONE)
		{
			if (m_bIsCtrlDrag)
				OnDropCopy();
			else
				OnDropMove();
		}
	}

	CTreeView::OnLButtonUp(nFlags, point);
}

void CLeftView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		// stop the drag
		StopDrag();

		if (m_dragOp != DO_NONE)
		{
			// show menu
			ClientToScreen(&point);
			m_DropMenu.GetSubMenu(0)->TrackPopupMenu(
				TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this);
		}
	}

	CTreeView::OnRButtonUp(nFlags, point);
}

void CLeftView::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == HOVER_EXPAND_TIMER_ID)
	{
		GetTreeCtrl().Expand(m_hDropItem, TVE_EXPAND);
	}
	else if (nIDEvent == AUTO_SAVE_TIMER_ID)
	{
		theApp.SaveIfRequired();
	}

	CTreeView::OnTimer(nIDEvent);
}

BOOL CLeftView::PreTranslateMessage(MSG* pMsg)
{
	WPARAM wParam = pMsg->wParam;
	CEdit *pEdit = GetTreeCtrl().GetEditControl();

	// treat edit control message separately
	if (pEdit)
	{
		// handle these keys: esc, ctrl+x, ctrl+v, ctrl+c
		if (pMsg->message == WM_KEYDOWN)
		{
			// is ctrl pressed?
			SHORT bCtrl = GetKeyState(VK_CONTROL) & 0x8000;

			switch (wParam)
			{
			case VK_ESCAPE: OnTreeEscKey(); return TRUE;
			case 'C': if (bCtrl) { pEdit->SendMessage(WM_COPY); return TRUE; } break;
			case 'X': if (bCtrl) { pEdit->SendMessage(WM_CUT); return TRUE; } break;
			case 'V': if (bCtrl) { pEdit->SendMessage(WM_PASTE); return TRUE; } break;
			}
		}

		// else just do the default thing
		return CTreeView::PreTranslateMessage(pMsg);
	}

	// if normal message (not while editing), see if these are shortcuts
	if (TranslateAccelerator(GetSafeHwnd(), m_hAccel, pMsg))
		return TRUE;

	// else just do the default thing
	return CTreeView::PreTranslateMessage(pMsg);
}

void CLeftView::OnTreeDelSelected()
{
	HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
	if (hItem)
		DeleteItem(hItem);
}

void CLeftView::OnTreeEscKey()
{
	CTreeCtrl& tCtrl = GetTreeCtrl();

	if (m_bDragging)
	{
		StopDrag();
		theApp.SetStatusBarText(AFX_IDS_IDLEMESSAGE);
	}
	else if (tCtrl.GetEditControl())
	{
		TreeView_EndEditLabelNow(GetSafeHwnd(), TRUE);
	}
	else if (theApp.GetEscMinimize())
	{
		AfxGetMainWnd()->PostMessage(WM_SYSCOMMAND, SC_MINIMIZE);
	}
}

void CLeftView::OnTreeMoveUp()
{
	CTreeCtrl& tCtrl = GetTreeCtrl();

	// commit changes
	CommitNow();

	// get selected item
	HTREEITEM hItem = tCtrl.GetSelectedItem();
	if (!hItem) return;

	// get item above it
	HTREEITEM hPrevItem = tCtrl.GetPrevVisibleItem(hItem);
	if (!hPrevItem) return;

	// move if we can
	if (wintree_can_move(m_pWinTree, hItem, hPrevItem))
	{
		HTREEITEM newItem = 
			wintree_move_or_copy(m_pWinTree, hItem, hPrevItem, WT_MV_BEFORE);
		GetDocument()->SetModifiedFlag();
		tCtrl.SelectItem(newItem);
	}
}

void CLeftView::OnTreeMoveDown()
{
	CTreeCtrl& tCtrl = GetTreeCtrl();

	// commit changes
	CommitNow();

	// get selected item
	HTREEITEM hItem = tCtrl.GetSelectedItem();
	if (!hItem) return;

	// get item below it
	HTREEITEM hNextItem = tCtrl.GetNextVisibleItem(hItem);
	if (!hNextItem) return;

	// usually we plan to move the src item after the next item
	enum move_op_e opn = WT_MV_AFTER;
	HTREEITEM hDstItem = hNextItem;

	// but if the below item is in expanded state, we want to add
	// `item' before the first item after the `below item'.
	// if you don't believe me comment out the following lines
	// and see for yourself!
	HTREEITEM childItem = tCtrl.GetChildItem(hNextItem);
	if (childItem != NULL &&
		(tCtrl.GetItemState(hNextItem, TVIS_EXPANDED) & TVIS_EXPANDED))
	{
		hDstItem = childItem;
		opn = WT_MV_BEFORE;
	}

	// move if we can
	if (wintree_can_move(m_pWinTree, hItem, hDstItem))
	{
		HTREEITEM newItem = 
			wintree_move_or_copy(m_pWinTree, hItem, hDstItem, opn);
		GetDocument()->SetModifiedFlag();
		tCtrl.SelectItem(newItem);
	}
}

void CLeftView::OnTreeShiftFocus()
{
	CView *rightView = theApp.GetRightView();

	if (rightView->IsWindowEnabled())
	{
		CFrameWnd *frameWnd = (CFrameWnd *)theApp.m_pMainWnd;
		ASSERT(frameWnd);
		frameWnd->SetActiveView(rightView);
	}
}

void CLeftView::InsertAbove(HTREEITEM hItem)
{
	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM hNewItem = wintree_insert_at_current(m_pWinTree, hItem,
		RCSTR(IDS_PAGE_NEWTOP), theEmptyRTFText);
	ASSERT(hNewItem);
	tree.Select(hNewItem, TVGN_CARET);
	theApp.GetRightView()->SendMessage(WM_GUIDEUI, GM_NEW_PAGE);
	tree.EditLabel(hNewItem);
	GetDocument()->SetModifiedFlag();
}

void CLeftView::InsertBelow(HTREEITEM hItem)
{
	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM hNewItem = wintree_insert_after_current(m_pWinTree, hItem,
		RCSTR(IDS_PAGE_NEWTOP), theEmptyRTFText);
	ASSERT(hNewItem);
	tree.Select(hNewItem, TVGN_CARET);
	theApp.GetRightView()->SendMessage(WM_GUIDEUI, GM_NEW_PAGE);
	tree.EditLabel(hNewItem);
	GetDocument()->SetModifiedFlag();
}

void CLeftView::OnTreeInsert()
{
	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM hCurItem = tree.GetSelectedItem();
	if (hCurItem)
		InsertAbove(hCurItem);
}

void CLeftView::OnTreeInsertAfter()
{
	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM hCurItem = tree.GetSelectedItem();
	if (hCurItem)
		InsertBelow(hCurItem);
}

void CLeftView::OnRMBAddPageAbove()
{
	InsertAbove(m_RClickItem);
	m_RClickItem = NULL;
}

void CLeftView::OnRMBAddPageBelow()
{
	InsertBelow(m_RClickItem);
	m_RClickItem = NULL;
}

void CLeftView::EnableIfSelected(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetTreeCtrl().GetSelectedItem() != NULL);
}

void CLeftView::ApplyColor()
{
	CTreeCtrl& theCtrl = GetTreeCtrl();

	// Set the colors if required
	bool sysDefault = false;
	COLORREF foreColor, backColor;
	theApp.GetTreeColor(sysDefault, foreColor, backColor);
	if (sysDefault)
	{
		theCtrl.SetBkColor(-1);
		theCtrl.SetTextColor(-1);
	}
	else
	{
		theCtrl.SetBkColor(backColor);
		theCtrl.SetTextColor(foreColor);
	}
}

void CLeftView::ApplyFont()
{
	CTreeCtrl& theCtrl = GetTreeCtrl();

	// Set the font if required
	bool sysDefault = true;
	PreferredFont treeFont;
	theApp.GetTreeFont(sysDefault, treeFont);
	if (sysDefault)
	{
		theCtrl.SetFont(NULL);
	}
	else
	{
		// destroy old font, if any
		if (m_oFont.GetSafeHandle() != NULL)
			m_oFont.DeleteObject();
		ASSERT(m_oFont.GetSafeHandle() == NULL);

		// calculate height
		HDC hScreenDC = ::GetDC(NULL);
		LONG lfHeight = 
			-MulDiv(treeFont.height, GetDeviceCaps(hScreenDC, LOGPIXELSY), 72);
		::ReleaseDC(NULL, hScreenDC);

		// fill out a logfont structure
		LOGFONT logFont;
		memset(&logFont, 0, sizeof(logFont));
		logFont.lfHeight = lfHeight;
		logFont.lfItalic = treeFont.italic;
		logFont.lfWeight = treeFont.bold ? FW_BOLD : 0;
		wcscpy(logFont.lfFaceName, treeFont.faceName);

		// create font object
		m_oFont.CreateFontIndirect(&logFont);

		theCtrl.SetFont(&m_oFont);
	}
}

void CLeftView::OnRmbSearch()
{
	theApp.m_pMainWnd->SendMessage(WM_GUIDEUI, GM_SHOW_SEARCH);
}

void CLeftView::OnRmbMovedown()
{
	OnTreeMoveDown();
}

void CLeftView::OnRmbMoveup()
{
	OnTreeMoveUp();
}

static void PostMessageToRightView(UINT msg, WPARAM wParam, LPARAM lParam,
								   bool makeActiveFirst = false)
{
	CView *rightView = theApp.GetRightView();

	ASSERT(rightView->IsWindowEnabled());
	if (rightView->IsWindowEnabled())
	{
		CFrameWnd *frameWnd = (CFrameWnd *)theApp.m_pMainWnd;
		ASSERT(frameWnd);
		if (makeActiveFirst)
			frameWnd->SetActiveView(rightView);

		rightView->PostMessage(msg, wParam, lParam);
	}
}

void CLeftView::OnFilePrint()
{
	// Forward the request to the right view if it is enabled.
	// Note that it should be enabled, since this menu item
	// itself should have been enabled only when the right
	// view is enabled.

	PostMessageToRightView(WM_COMMAND, ID_FILE_PRINT, 0, true);
}

void CLeftView::OnFilePrintPreview()
{
	// Forward the request to the right view if it is enabled.
	// Note that it should be enabled, since this menu item
	// itself should have been enabled only when the right
	// view is enabled.

	PostMessageToRightView(WM_COMMAND, ID_FILE_PRINT_PREVIEW, 0, true);
}

void CLeftView::OnFileExport()
{
	// Forward the request to the right view if it is enabled.
	// Note that it should be enabled, since this menu item
	// itself should have been enabled only when the right
	// view is enabled.

	thePrintItemInfo.SetPrintType(PT_ENTIRE_DOCUMENT);
	thePrintItemInfo.SetSelectedItem(0);

	PostMessageToRightView(WM_COMMAND, ID_FILE_EXPORT, 0);
}

void CLeftView::OnRmbPrint()
{
	// Forward the request to the right view if it is enabled.
	// Note that it should be enabled, since this menu item
	// itself should have been enabled only when the right
	// view is enabled.

	PostMessageToRightView(WM_GUIDEUI, GM_TREERMB_PRINT, (LPARAM)m_RClickItem);
	m_RClickItem = NULL;
}

void CLeftView::OnRmbPrintpreview()
{
	// Forward the request to the right view if it is enabled.
	// Note that it should be enabled, since this menu item
	// itself should have been enabled only when the right
	// view is enabled.

	PostMessageToRightView(WM_GUIDEUI, GM_TREERMB_PREVIEW, (LPARAM)m_RClickItem);
	m_RClickItem = NULL;
}

void CLeftView::OnDropCopy()
{
	ASSERT(m_hDragItem != NULL && m_hDropItem != NULL && m_dragOp != DO_NONE);
	if (m_hDragItem == NULL || m_hDropItem == NULL || m_dragOp == DO_NONE)
		return;

	move_op_e op = WT_CP_AFTER;
	switch (m_dragOp)
	{
		case DO_AFTER: op = WT_CP_AFTER; break;
		case DO_BEFORE: op = WT_CP_BEFORE; break;
		case DO_CHILD: op = WT_CP_CHILD; break;
	}
	HTREEITEM newItem = wintree_move_or_copy(m_pWinTree, m_hDragItem, m_hDropItem, op);
	GetTreeCtrl().SelectItem(newItem);

	GetDocument()->SetModifiedFlag();

	theApp.SetStatusBarText(AFX_IDS_IDLEMESSAGE);
}

void CLeftView::OnDropMove()
{
	ASSERT(m_hDragItem != NULL && m_hDropItem != NULL && m_dragOp != DO_NONE);
	if (m_hDragItem == NULL || m_hDropItem == NULL || m_dragOp == DO_NONE)
		return;

	move_op_e op = WT_MV_AFTER;
	switch (m_dragOp)
	{
		case DO_AFTER: op = WT_MV_AFTER; break;
		case DO_BEFORE: op = WT_MV_BEFORE; break;
		case DO_CHILD: op = WT_MV_CHILD; break;
	}
	HTREEITEM newItem = wintree_move_or_copy(m_pWinTree, m_hDragItem, m_hDropItem, op);
	GetTreeCtrl().SelectItem(newItem);

	GetDocument()->SetModifiedFlag();

	theApp.SetStatusBarText(AFX_IDS_IDLEMESSAGE);
}

static int _tvr_expand(HTREEITEM item, struct tree_node_t *node, void *cargo)
{
	TreeView_Expand((HWND)cargo, item, TVE_EXPAND);
	return 0;
}

void CLeftView::OnTreeExpandall()
{
	wintree_traverse(m_pWinTree, _tvr_expand, m_hWnd);
}

static int _tvr_collapse(HTREEITEM item, struct tree_node_t *node, void *cargo)
{
	TreeView_Expand((HWND)cargo, item, TVE_COLLAPSE);
	return 0;
}

void CLeftView::OnTreeCollapseall()
{
	// BUG FIX: check if the current node is a top-level node.
	// If not, find out the top-most node of the current node,
	// select it and then collapse all. If this is not done,
	// the text remains as is but the selection changes to
	// the top-most node of the current node after a collapse
	// all.

	CTreeCtrl& theCtrl = GetTreeCtrl();
	HTREEITEM hItem = theCtrl.GetSelectedItem(), hParent = NULL;
	ASSERT(hItem);
	while ((hParent = theCtrl.GetNextItem(hItem, TVGN_PARENT)) != NULL)
		hItem = hParent;
	theCtrl.SelectItem(hItem);

	wintree_traverse(m_pWinTree, _tvr_collapse, m_hWnd);
}

void CLeftView::OnEditInsertdate()
{
	CEdit *pEdit = GetTreeCtrl().GetEditControl();
	if (pEdit)
	{
		pEdit->ReplaceSel(GetConfiguredDateString(), TRUE);
	}
}

void CLeftView::OnEditInserttime()
{
	CEdit *pEdit = GetTreeCtrl().GetEditControl();
	if (pEdit)
	{
		pEdit->ReplaceSel(GetConfiguredTimeString(), TRUE);
	}
}

void CLeftView::StartAutoSaveTimer()
{
	KillTimer(AUTO_SAVE_TIMER_ID);

	if (theApp.GetAutoSave())
	{
		unsigned minutes = theApp.GetAutosaveInterval();
		if (minutes > 0)
			// bug fix 2.0b1+: 60*1000, not 60*100 !!
			SetTimer(AUTO_SAVE_TIMER_ID, minutes * 60 * 1000 /*ms*/, NULL);
	}
}

void CLeftView::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVCUSTOMDRAW pNMTVCD = reinterpret_cast<LPNMTVCUSTOMDRAW>(pNMHDR);

	// by default, do default
	*pResult = CDRF_DODEFAULT;

	switch (pNMTVCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		if (! pNMTVCD->nmcd.lItemlParam)  // with a manifest and the 'royale' theme, a
			break;						// "spurious" ITEMPREPAINT is received (?!)

		// get the node
		struct tree_node_t *node = (struct tree_node_t *)(pNMTVCD->nmcd.lItemlParam);
		struct guide_nodedata_t *data = 
			(struct guide_nodedata_t *)tree_get_data(node);

		// set the colors
		if (data->color != (uint32)-1)
		{
			pNMTVCD->clrText = data->color;
		}
		if (data->bgcolor != (uint32)-1)
		{
			pNMTVCD->clrTextBk = data->bgcolor;
		}

		// do the default thing itself
		break;
	}
}

class CLeftViewColorDialog : public CFgColorDialog
{
	HTREEITEM m_hItem;
	WPARAM m_wParam;

public:
	CLeftViewColorDialog(WPARAM wParam) : m_wParam(wParam) { }
	void SetItem(HTREEITEM hItem) { m_hItem = hItem; }
	void OnSelectColor(COLORREF selectedColor)
	{
		SetNodeColorInput input = { m_hItem, selectedColor };
		theApp.GetLeftView()->SendMessage(WM_GUIDEUI, m_wParam, (LPARAM)&input);
	}
};

void CLeftView::ProcessPopupColor(unsigned whichDlg, HTREEITEM hItem)
{
	static CLeftViewColorDialog fg(GM_SET_NODE_FGCOLOR);
	static CLeftViewColorDialog bg(GM_SET_NODE_BGCOLOR);
	CLeftViewColorDialog &dialog = whichDlg == DLG_FOREGROUND_COLOR ? fg : bg;

	// create the dialog if required
	if (dialog.GetSafeHwnd() == NULL)
		dialog.Create(CFgColorDialog::IDD, this);

	// position the dialog
	CRect dr;
	dialog.GetWindowRect(&dr);
	MSG *pMsg = AfxGetCurrentMessage();
	dialog.MoveWindow(pMsg->pt.x - dr.Width()/2, pMsg->pt.y - dr.Height()/2, 
		dr.Width(), dr.Height(), FALSE);

	// set the initial color
	struct tree_node_t *node = wintree_get_node_from_item(m_pWinTree, hItem);
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);
	dialog.SetInitialColor(data->color);

	// set which item to work on
	dialog.SetItem(hItem);

	// set the focus and show the window
	dialog.SetFocus();
	dialog.ShowWindow(SW_SHOW);
}

void CLeftView::OnTreeNodeForeColor()
{
	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM selItem = tree.GetSelectedItem();
	if (selItem)
		ProcessPopupColor(DLG_FOREGROUND_COLOR, selItem);
}

void CLeftView::OnTreeNodeBackcolor()
{
	CTreeCtrl& tree = GetTreeCtrl();
	HTREEITEM selItem = tree.GetSelectedItem();
	if (selItem)
		ProcessPopupColor(DLG_BACKGROUND_COLOR, selItem);
}

void CLeftView::OnRmbNodeForeColor()
{
	ASSERT(m_RClickItem);
	if (m_RClickItem)
		ProcessPopupColor(DLG_FOREGROUND_COLOR, m_RClickItem);
	m_RClickItem = 0;
}

void CLeftView::OnRmbNodeBackcolor()
{
	ASSERT(m_RClickItem);
	if (m_RClickItem)
		ProcessPopupColor(DLG_BACKGROUND_COLOR, m_RClickItem);
	m_RClickItem = 0;
}

void CLeftView::OnTvnItemexpanded(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (pNMTreeView->action == TVE_EXPAND)
	{
		struct tree_node_t *node = wintree_get_node_from_item(m_pWinTree, pNMTreeView->itemNew.hItem);
		struct guide_nodedata_t *data = 
			(struct guide_nodedata_t *)tree_get_data(node);

		guide_nodedata_set_expanded(data, 1);
	}
	else if (pNMTreeView->action == TVE_COLLAPSE)
	{
		struct tree_node_t *node = wintree_get_node_from_item(m_pWinTree, pNMTreeView->itemNew.hItem);
		struct guide_nodedata_t *data = 
			(struct guide_nodedata_t *)tree_get_data(node);

		guide_nodedata_set_expanded(data, 0);
	}

	*pResult = 0;
}

void CLeftView::OnRmbNodeicon()
{
	if (!m_pIconWnd)
	{
		DWORD pos = GetMessagePos();
		int x = (int)(short)LOWORD(pos); //GET_X_LPARAM(pos);
		int y = (int)(short)HIWORD(pos); //GET_Y_LPARAM(pos);

		m_pIconWnd = new CIconWnd();
		m_pIconWnd->Create(this, x, y);
	}
}
