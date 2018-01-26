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


/* ----------------------------------------------------------------- */
/* includes */

#include <assert.h>
#include <windows.h>
#include <commctrl.h>
#include <libguide/wintree.h>
#include <libguide/treeutil.h>
#include <libguide/lut.h>


/* ----------------------------------------------------------------- */
/* defines */

#define LOWEST_ICON_INDEX		(0)		/* XXX */
#define HIGHEST_ICON_INDEX		(40)	/* XXX */


/* ----------------------------------------------------------------- */
/* data structures */

struct wintree_t
{
	struct guide_t *guide;
	struct tree_t *tree;
	HWND hwnd;
	int leaf_image_idx;
	int nonleaf_image_idx;
};


/* ----------------------------------------------------------------- */
/* helper methods */

static int _wintree_get_icon_index(struct wintree_t *wintree,
	struct tree_node_t *node)
{
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);

	int iImage = (int) data->icon;
	if (iImage < LOWEST_ICON_INDEX || iImage > HIGHEST_ICON_INDEX)
		iImage = 0;

	if (iImage == 0)		/* default */
	{
		return tree_get_first_child(node) ? 
				wintree->nonleaf_image_idx : wintree->leaf_image_idx;
	}
	/* else */
	return iImage;
}

static HTREEITEM _wintree_insert_item(struct wintree_t *wintree,
	struct tree_node_t *node, HTREEITEM parent,
	HTREEITEM insert_after)
{
	TVINSERTSTRUCT tv;
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);

	memset(&tv, 0, sizeof(tv));
	tv.hInsertAfter = insert_after;
	tv.hParent = parent;
	tv.item.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM|TVIF_STATE;
	tv.item.pszText = data->title;
	tv.item.lParam = (LPARAM) node;
	tv.item.iImage = _wintree_get_icon_index(wintree, node);
	tv.item.iSelectedImage = tv.item.iImage;
	tv.item.state = data->tc_state;
	tv.item.stateMask = TVIS_STATEIMAGEMASK;

	return TreeView_InsertItem(wintree->hwnd, &tv);
}

struct _wintree_insert_t
{
	struct wintree_t *wintree;
	struct lut_t *lut;
	struct tree_node_t *tree_root;
	HTREEITEM ui_root;
};

static int _wintree_inserter(struct tree_node_t *node, void *cargo)
{
	HTREEITEM hItem, hParentItem;
	struct _wintree_insert_t *ins = (struct _wintree_insert_t *)cargo;
	struct tree_node_t *parent = tree_get_parent(node);
	struct guide_nodedata_t *data = (struct guide_nodedata_t *)
		tree_get_data(node);
	struct tree_node_t *fchild = tree_get_first_child(node);

	/* don't try to insert root into ui, just return successfully */
	if (node == ins->tree_root)
		return 0;

	/* get parent HTREEITEM */
	if (parent != ins->tree_root)
		lut_get(ins->lut, parent, &hParentItem);
	else
		/* we're inserting an immediate child of the root */
		hParentItem = ins->ui_root;

	/* insert it */
	hItem = _wintree_insert_item(ins->wintree, node, hParentItem, TVI_LAST);

	/* remember it */
	lut_set(ins->lut, node, hItem);

	return 0;
}

/**
 * Populate a tree control. Given a subtree starting at `node'
 * corresponding to a UI subtree starting at `ui_root', this
 * fn. populates the UI subtree with the contents of the
 * in-memory tree.
 *
 * The `ui_root' node should have added by the caller, and 
 * should correspond to the `node' node. Note that in our
 * case, when node == tree_get_root(tree), then 
 * ui_root == NULL (since we don't show the actual root in
 * the UI); which means that you can populate the entire tree
 * when `node' is the root node and `ui_root' == NULL.
 *
 * If `node' has no children, this fn. is a no-op.
 */
static int _wintree_insert_tree(struct wintree_t *wintree,
	struct tree_node_t *node, HTREEITEM ui_root)
{
	int ret = -1;
	assert(wintree);
	assert(node);

	if (tree_get_first_child(node))
	{
		struct lut_t *lut = lut_create();
		struct _wintree_insert_t ins = { wintree, lut, node, ui_root };

		ret = tree_traverse_subtree_preorder(node, _wintree_inserter, &ins);
		lut_free(lut);
	}

	return ret;
}

/**
 * A pre-order wintree traversal callback to expand nodes as indicated
 * by node->data->state.
 */
static int _wintree_expander(HTREEITEM item, struct tree_node_t *node, 
	void *cargo)
{
	struct guide_nodedata_t *data = (struct guide_nodedata_t *)
		tree_get_data(node);

	if (guide_nodedata_get_expanded(data))
	{
		TreeView_Expand((HWND)cargo, item, TVE_EXPAND);
	}
	/* assume that the node is collapsed (since this function
	 * is called only just after tree creation as of now (v1.6).
	 * if not, do a TreeView_Expand(TVE_COLLAPSE). */

	return 0;
}

static struct tree_node_t *_wintree_get_last_child(struct tree_node_t *parent)
{
	struct tree_node_t *node = tree_get_first_child(parent);
	if (node)
	{
		while (tree_get_next_sibling(node) != NULL)
			node = tree_get_next_sibling(node);
	}
	return node;
}


/* ----------------------------------------------------------------- */
/* wintree methods */

struct wintree_t *wintree_create(struct guide_t *guide,
		 HWND hwnd, int leaf_image_idx, int nonleaf_image_idx)
{
	struct wintree_t *wintree;

	wintree = (struct wintree_t *)malloc(sizeof(struct wintree_t));
	assert(wintree);
	if (!wintree) return NULL;

	wintree->guide = guide;
	wintree->tree = guide_get_tree(guide);
	wintree->hwnd = hwnd;
	wintree->leaf_image_idx = leaf_image_idx;
	wintree->nonleaf_image_idx = nonleaf_image_idx;

	return wintree;
}

void wintree_destroy(struct wintree_t *wintree)
{
	assert(wintree);
	assert(wintree->hwnd);

	TreeView_DeleteAllItems(wintree->hwnd);

#ifdef _DEBUG
	memset(wintree, 0xab, sizeof(struct wintree_t));
#endif
	free(wintree);
}

void wintree_populate(struct wintree_t *wintree)
{
	/* Delete all items first. */
	/* Apparently doing a TreeView_DeleteAllItems seems to do something
	 * bad to the image list. Restoring it after the deletion works. */
	HIMAGELIST himgl = TreeView_GetImageList(wintree->hwnd, TVSIL_NORMAL);
	TreeView_DeleteAllItems(wintree->hwnd);
	TreeView_SetImageList(wintree->hwnd, himgl, TVSIL_NORMAL);

	/* actually insert */
	_wintree_insert_tree(wintree, tree_get_root(wintree->tree), NULL);

	/* expand the nodes where necessary */
	wintree_traverse(wintree, _wintree_expander, (void *)(wintree->hwnd));
}

HTREEITEM wintree_add_toplevel(struct wintree_t *wintree, 
		const wchar_t *title, const char *data)
{
	struct tree_node_t *root = tree_get_root(wintree->tree);
	struct tree_node_t *node;
	struct tree_node_t *new_node;
	struct guide_nodedata_t *new_data;

	/* make data for the new node */
	new_data = guide_nodedata_create_with_data(wintree->guide, title, data);

	/* add as child under root */
	assert(root);
	node = _wintree_get_last_child(root);
	new_node = guide_add_child(wintree->guide, root, new_data, node);

	/* insert it */
	return _wintree_insert_item(wintree, new_node, TVI_ROOT, TVI_LAST);
}

struct tree_node_t *wintree_get_node_from_item(
	struct wintree_t *wintree, HTREEITEM item)
{
	TVITEM tvi;

	memset(&tvi, 0, sizeof(tvi));
	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.hItem = item;
	TreeView_GetItem(wintree->hwnd, &tvi);
	return (struct tree_node_t *)tvi.lParam;
}

void wintree_update_icon(struct wintree_t *wintree,
	HTREEITEM item)
{
	struct tree_node_t *node = 
		wintree_get_node_from_item(wintree, item);
	int image_idx = _wintree_get_icon_index(wintree, node);

	TVITEM tvItem;
	memset(&tvItem, 0, sizeof(tvItem));
	tvItem.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvItem.hItem = item;
	tvItem.iImage = image_idx;
	tvItem.iSelectedImage = image_idx;
	TreeView_SetItem(wintree->hwnd, &tvItem);
}

HTREEITEM wintree_insert_at_current(struct wintree_t *wintree,
	HTREEITEM item, const wchar_t *title, const char *data)
{
	struct tree_node_t *node = 
		wintree_get_node_from_item(wintree, item);
	struct tree_node_t *new_node;
	struct guide_nodedata_t *new_data;
	HTREEITEM hParentItem, hPrevItem;

	/* get parent node */
	hParentItem = TreeView_GetParent(wintree->hwnd, item);
	/* note: hParentItem may be NULL */

	/* get prev node */
	hPrevItem = TreeView_GetPrevSibling(wintree->hwnd, item);
	/* note: hPrevNode may be NULL */

	/* make data for the new node */
	new_data = guide_nodedata_create_with_data(wintree->guide, title, data);

	/* add as sibling before ("at") current node */
	new_node = guide_add_sibling_before(wintree->guide, node, new_data);

	/* actually insert */
	return _wintree_insert_item(wintree, new_node, hParentItem, 
		hPrevItem ? hPrevItem : TVI_FIRST);
}

HTREEITEM wintree_insert_after_current(struct wintree_t *wintree,
	HTREEITEM item, const wchar_t *title, const char *data)
{
	struct tree_node_t *node = 
		wintree_get_node_from_item(wintree, item);
	struct tree_node_t *new_node;
	struct guide_nodedata_t *new_data;
	HTREEITEM hParentItem;

	/* get parent node */
	hParentItem = TreeView_GetParent(wintree->hwnd, item);
	/* note: hParentItem may be NULL */

	/* make data for the new node */
	new_data = guide_nodedata_create_with_data(wintree->guide, title, data);

	/* add as sibling after current node */
	new_node = guide_add_sibling_after(wintree->guide, node, new_data);

	/* actually insert */
	return _wintree_insert_item(wintree, new_node, hParentItem, 
		item);
}

HTREEITEM wintree_add_child(struct wintree_t *wintree,
		HTREEITEM item, const wchar_t *title, const char *data)
{
	struct tree_node_t *parent_node = 
		wintree_get_node_from_item(wintree, item);
	struct tree_node_t *node, *new_node;
	struct guide_nodedata_t *new_data;
	HTREEITEM hItem;

	/* make data for the new node */
	new_data = guide_nodedata_create_with_data(wintree->guide, title, data);

	/* add as child under root */
	node = _wintree_get_last_child(parent_node);
	new_node = guide_add_child(wintree->guide, parent_node, new_data, node);

	/* insert item into UI */
	hItem = _wintree_insert_item(wintree, new_node, item, TVI_LAST);

	/* set new parent's icon to non-leaf */
	wintree_update_icon(wintree, item);

	return hItem;
}

HTREEITEM wintree_add_child_as_first(struct wintree_t *wintree,
	HTREEITEM item, const wchar_t *title, const char *data)
{
	struct tree_node_t *parent_node = 
		wintree_get_node_from_item(wintree, item);
	struct tree_node_t *new_node;
	struct guide_nodedata_t *new_data;
	HTREEITEM hItem;

	/* make data for the new node */
	new_data = guide_nodedata_create_with_data(wintree->guide, title, data);

	/* add as child under root */
	new_node = guide_add_child(wintree->guide, parent_node, new_data, NULL);

	/* insert item into UI */
	hItem = _wintree_insert_item(wintree, new_node, item, TVI_FIRST);

	/* set new parent's icon to non-leaf */
	wintree_update_icon(wintree, item);

	return hItem;
}

void wintree_rename_item(struct wintree_t *wintree,
		HTREEITEM item, const wchar_t *new_name)
{
	struct tree_node_t *node = wintree_get_node_from_item(wintree, item);
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);
	guide_nodedata_set_title(data, new_name);
}

void wintree_delete(struct wintree_t *wintree,
		HTREEITEM item)
{
	/* get node */
	struct tree_node_t *node = wintree_get_node_from_item(wintree, item);

	/* get parent item */
	struct tree_node_t *parent = tree_get_parent(node);
	HTREEITEM hParentItem;
	if (parent)
		hParentItem = TreeView_GetParent(wintree->hwnd, item);

	/* delete from data structure */
	guide_delete_subtree(wintree->guide, node);

	/* delete from ui */
	TreeView_DeleteItem(wintree->hwnd, item);

	/* in ui tree, set parent node's icon to leaf if
	   it has no more children */
	/* bug fix 2.0b1+: `hParentItem', not `parent' */
	if (hParentItem && tree_get_first_child(parent) == NULL)
		wintree_update_icon(wintree, hParentItem);
}

static int _wintree_child_searcher(struct tree_node_t *node, void *cargo)
{
	if (node == (struct tree_node_t *)cargo)
		return 1; /* found */
	return 0; /* continue searching */
}

/**
 * Check if a subtree rooted at 'srcItem' can be moved/copied to
 * after/before/as child of 'dstItem'. This is possible if the
 * trees from root->srcItem and root->dstItem are disjoint except
 * for root.
 */
int wintree_can_move(struct wintree_t *wintree,
		HTREEITEM srcItem, HTREEITEM dstItem)
{
	assert(srcItem);
	assert(dstItem);

	while (dstItem)
	{
		if (dstItem == srcItem) return 0;
		dstItem = TreeView_GetParent(wintree->hwnd, dstItem);
	}
	return 1;
}

#ifdef _DEBUG
static void _wintree_check_subtree(struct wintree_t *wintree,
	HTREEITEM item, struct tree_node_t *node_from_tree)
{
	struct tree_node_t *child_from_tree = tree_get_first_child(node_from_tree);
	struct tree_node_t *node_from_ui;

	HTREEITEM child = TreeView_GetChild(wintree->hwnd, item);
	while (child)
	{
		_wintree_check_subtree(wintree, child, child_from_tree);
		child = TreeView_GetNextSibling(wintree->hwnd, child);
		child_from_tree = tree_get_next_sibling(child_from_tree);
	}

	node_from_ui = wintree_get_node_from_item(wintree, item);
	if (node_from_ui != node_from_tree)
	{
		MessageBox(wintree->hwnd, L"Inconsitency detected.", L"The Guide", 
			MB_ICONSTOP|MB_OK);
	}
}

static void _wintree_check(struct wintree_t *wintree)
{
	HWND hwnd;
	HTREEITEM root_item;
	struct tree_node_t *node_from_ui, *node_from_tree;

	hwnd = wintree->hwnd;
	root_item = TreeView_GetRoot(hwnd);
	node_from_tree = tree_get_root(wintree->tree);
	node_from_tree = tree_get_first_child(node_from_tree);
	while (root_item) 
	{
		_wintree_check_subtree(wintree, root_item, node_from_tree);
		node_from_ui = wintree_get_node_from_item(wintree, root_item);
		if (node_from_ui != node_from_tree)
		{
			MessageBox(hwnd, L"Inconsitency detected.", L"The Guide", 
				MB_ICONSTOP|MB_OK);
		}
		root_item = TreeView_GetNextSibling(hwnd, root_item);
		node_from_tree = tree_get_next_sibling(node_from_tree);
	}
}
#endif

static void *clone_cb(void *src, void *cargo)
{
	struct guide_t *guide = (struct guide_t *)cargo;
	struct guide_nodedata_t *nodedata = (struct guide_nodedata_t *)src;
	return guide_nodedata_clone(nodedata, guide);
}

static HTREEITEM wintree_copy_before(struct wintree_t *wintree,
		HTREEITEM srcItem, HTREEITEM dstItem)
{
	struct tree_node_t *src_node = 
		wintree_get_node_from_item(wintree, srcItem);
	struct tree_node_t *dst_node = 
		wintree_get_node_from_item(wintree, dstItem);
	struct tree_node_t *new_src_node;
	HTREEITEM dst_parent_item, dst_prev_item, new_src_item;

	/* get parent item */
	dst_parent_item = TreeView_GetParent(wintree->hwnd, dstItem);
	dst_prev_item = TreeView_GetPrevSibling(wintree->hwnd, dstItem);
	/* note: dst_prev_item might be NULL */

	/* update data structure */
	new_src_node = 
		tree_copy_subtree_before(src_node, dst_node, clone_cb, wintree->guide);

	/* add src item to dst item's parent */
	/* if no prev sibling existed for dst_node, be sure to insert as first
	 * child of dst_node's parent */
	new_src_item = _wintree_insert_item(wintree, new_src_node, dst_parent_item,
		dst_prev_item ? dst_prev_item : TVI_FIRST);

	/* then recursively insert the rest with 'new_src_item' as the root */
	_wintree_insert_tree(wintree, new_src_node, new_src_item);

	return new_src_item;
}

static HTREEITEM wintree_move_before(struct wintree_t *wintree,
		HTREEITEM srcItem, HTREEITEM dstItem)
{
	struct tree_node_t *src_node = 
		wintree_get_node_from_item(wintree, srcItem);
	struct tree_node_t *dst_node = 
		wintree_get_node_from_item(wintree, dstItem);
	struct tree_node_t *old_src_node_parent = 
		tree_get_parent(src_node);
	HTREEITEM old_src_parent_item, dst_parent_item;
	HTREEITEM dst_prev_item, new_src_item;

	/* get parent items */
	old_src_parent_item = TreeView_GetParent(wintree->hwnd, srcItem);
	dst_parent_item = TreeView_GetParent(wintree->hwnd, dstItem);
	dst_prev_item = TreeView_GetPrevSibling(wintree->hwnd, dstItem);
	/* note, dst_prev_item might be NULL */

	/* update data structure */
	tree_move_subtree_before(src_node, dst_node);

	/* delete src from ui */
	TreeView_DeleteItem(wintree->hwnd, srcItem);

	/* in ui tree, set src node's parent's icon to leaf if 
	   it has no more children */
	if (old_src_node_parent && !tree_get_first_child(old_src_node_parent))
		wintree_update_icon(wintree, old_src_parent_item);

	/* add src item & children to dst item's parent */
	/* first insert src item after 'dst_prev_item' */
	/* if no prev sibling existed for dst_node, be sure to insert as first
	 * child of dst_node's parent */
	new_src_item = _wintree_insert_item(wintree, src_node, dst_parent_item,
		dst_prev_item ? dst_prev_item : TVI_FIRST);

	/* then recursively insert the rest with 'new_src_item' as the root */
	_wintree_insert_tree(wintree, src_node, new_src_item);

	return new_src_item;
}

static HTREEITEM wintree_copy_after(struct wintree_t *wintree,
		HTREEITEM srcItem, HTREEITEM dstItem)
{
	struct tree_node_t *src_node = 
		wintree_get_node_from_item(wintree, srcItem);
	struct tree_node_t *dst_node = 
		wintree_get_node_from_item(wintree, dstItem);
	struct tree_node_t *new_src_node;
	HTREEITEM dst_parent_item, new_src_item;

	/* get parent item */
	dst_parent_item = TreeView_GetParent(wintree->hwnd, dstItem);

	/* update data structure */
	new_src_node = 
		tree_copy_subtree_after(src_node, dst_node, clone_cb, wintree->guide);

	/* insert the copied node after 'dstItem' */
	new_src_item = _wintree_insert_item(wintree, new_src_node, dst_parent_item,
		dstItem);

	/* then recursively insert the rest with 'new_src_item' as the root */
	_wintree_insert_tree(wintree, new_src_node, new_src_item);

	return new_src_item;
}

static HTREEITEM wintree_move_after(struct wintree_t *wintree,
		HTREEITEM srcItem, HTREEITEM dstItem)
{
	struct tree_node_t *src_node = 
		wintree_get_node_from_item(wintree, srcItem);
	struct tree_node_t *dst_node = 
		wintree_get_node_from_item(wintree, dstItem);
	struct tree_node_t *old_src_node_parent = 
		tree_get_parent(src_node);
	HTREEITEM old_src_parent_item, dst_parent_item, new_src_item;

	/* get parent items */
	old_src_parent_item = TreeView_GetParent(wintree->hwnd, srcItem);
	dst_parent_item = TreeView_GetParent(wintree->hwnd, dstItem);

	/* update data structure */
	tree_move_subtree_after(src_node, dst_node);

	/* delete src from ui */
	TreeView_DeleteItem(wintree->hwnd, srcItem);

	/* in ui tree, set src node's parent's icon to leaf if 
	   it has no more children */
	if (old_src_node_parent && !tree_get_first_child(old_src_node_parent))
		wintree_update_icon(wintree, old_src_parent_item);

	/* add src item & children to dst item's parent */
	/* first insert src item after 'dstItem' */
	new_src_item = _wintree_insert_item(wintree, src_node, dst_parent_item,
		dstItem);

	/* then recursively insert the rest with 'new_src_item' as the root */
	_wintree_insert_tree(wintree, src_node, new_src_item);

	return new_src_item;
}

static HTREEITEM wintree_copy_child(struct wintree_t *wintree,
		HTREEITEM srcItem, HTREEITEM dstItem)
{
	struct tree_node_t *src_node = 
		wintree_get_node_from_item(wintree, srcItem);
	struct tree_node_t *dst_node = 
		wintree_get_node_from_item(wintree, dstItem);
	struct tree_node_t *new_src_node;
	HTREEITEM dst_parent_item, new_src_item;

	/* get parent item */
	dst_parent_item = TreeView_GetParent(wintree->hwnd, dstItem);

	/* update data structure */
	new_src_node = 
		tree_copy_subtree_as_child(src_node, dst_node, clone_cb, wintree->guide);

	/* set new dst_node's icon to non-leaf */
	wintree_update_icon(wintree, dstItem);

	/* insert the copied node after 'dstItem' */
	new_src_item = _wintree_insert_item(wintree, new_src_node, dstItem,
		TVI_FIRST);

	/* then recursively insert the rest with 'new_src_item' as the root */
	_wintree_insert_tree(wintree, new_src_node, new_src_item);

	return new_src_item;
}

static HTREEITEM wintree_move_child(struct wintree_t *wintree,
		HTREEITEM srcItem, HTREEITEM dstItem)
{
	struct tree_node_t *src_node = 
		wintree_get_node_from_item(wintree, srcItem);
	struct tree_node_t *dst_node = 
		wintree_get_node_from_item(wintree, dstItem);
	struct tree_node_t *old_src_parent_node = tree_get_parent(src_node);
	HTREEITEM old_src_parent_item, new_src_item ;

	/* get old src's parent item (it's icon might change) */
	old_src_parent_item = TreeView_GetParent(wintree->hwnd, srcItem);
	/* note: old_src_parent_item might be NULL */

	/* update data structure */
	tree_move_subtree_as_child(src_node, dst_node);

	/* delete src from ui */
	TreeView_DeleteItem(wintree->hwnd, srcItem);

	/* in ui tree, set dst_node's icon to non-leaf */
	wintree_update_icon(wintree, dstItem);

	/* if original src_node's parent now has no more children, then
	 * set it's icon to leaf */
	if (old_src_parent_item && !tree_get_first_child(old_src_parent_node))
		wintree_update_icon(wintree, old_src_parent_item);

	/* insert source item in ui */
	new_src_item = 
		_wintree_insert_item(wintree, src_node, dstItem, TVI_FIRST);

	/* then recursively insert the rest with 'new_src_item' as the root */
	_wintree_insert_tree(wintree, src_node, new_src_item);

	return new_src_item;
}

HTREEITEM wintree_move_or_copy(struct wintree_t *wintree,
		HTREEITEM srcItem, HTREEITEM dstItem,
		enum move_op_e op)
{
	HTREEITEM out = NULL;

	switch (op) {
		case WT_MV_BEFORE: 
			out = wintree_move_before(wintree, srcItem, dstItem);
			break;
		case WT_MV_AFTER:
			out = wintree_move_after(wintree, srcItem, dstItem);
			break;
		case WT_MV_CHILD:
			out = wintree_move_child(wintree, srcItem, dstItem);
			break;
		case WT_CP_BEFORE: 
			out = wintree_copy_before(wintree, srcItem, dstItem);
			break;
		case WT_CP_AFTER:
			out = wintree_copy_after(wintree, srcItem, dstItem);
			break;
		case WT_CP_CHILD:
			out = wintree_copy_child(wintree, srcItem, dstItem);
			break;
		default:
			assert(0);
			return NULL;
	}

#ifdef _DEBUG
	_wintree_check(wintree);
#endif
	return out;
}

void wintree_set_text(struct wintree_t *wintree,
		HTREEITEM item, const char *text)
{
	struct tree_node_t *node = wintree_get_node_from_item(wintree, item);
	struct guide_nodedata_t *data = (struct guide_nodedata_t *)tree_get_data(node);

	guide_nodedata_set_text(data, text);
}

const char *wintree_get_text(struct wintree_t *wintree,
		HTREEITEM item)
{
	struct tree_node_t *node = wintree_get_node_from_item(wintree, item);
	struct guide_nodedata_t *data = (struct guide_nodedata_t *)tree_get_data(node);
	return data->text;
}

int wintree_traverse_subtree(struct wintree_t *wintree, 
		HTREEITEM item, wintree_traverser_fn_t tvr, void *cargo)
{
	struct tree_node_t *node = wintree_get_node_from_item(wintree, item);
	HWND hwnd = wintree->hwnd;
	int ret;

	/* visit self */
	ret = tvr(item, node, cargo);
	if (ret) return ret;

	/* visit children */
	item = TreeView_GetChild(hwnd, item);
	while (item)
	{
		ret = wintree_traverse_subtree(wintree, item, tvr, cargo);
		if (ret) return ret;

		item = TreeView_GetNextSibling(hwnd, item);
	}

	return 0;
}

int wintree_traverse(struct wintree_t *wintree,
		wintree_traverser_fn_t tvr, void *cargo)
{
	HWND hwnd;
	HTREEITEM item;
	int ret;
	assert(wintree);
	assert(tvr);

	/* for each top-level item in tree control, process */
	hwnd = wintree->hwnd;
	/* visit the first one */
	item = TreeView_GetRoot(hwnd);

	/* (bug fix 2.0dev1+) the tree may not have any nodes at all */
	if (!item)
		return 0;

	/* ok, go on, visit the first one. */
	ret = wintree_traverse_subtree(wintree, item, tvr, cargo);
	if (ret) return ret;

	/* visit the remaining */
	while ((item = TreeView_GetNextSibling(hwnd, item)))
	{
		ret = wintree_traverse_subtree(wintree, item, tvr, cargo);
		if (ret) return ret;
	}

	return 0;
}

struct tree_t *wintree_get_tree(struct wintree_t *wintree)
{
	return wintree->tree;
}

struct guide_t *wintree_get_guide(struct wintree_t *wintree)
{
	return wintree->guide;
}

static int _wintree_search_node(HTREEITEM item, struct tree_node_t *node, 
	void *cargo)
{
	if (node == (struct tree_node_t *)cargo)
		return (int) item;
	return 0;
}

HTREEITEM wintree_get_item_from_node(struct wintree_t *wintree, 
	struct tree_node_t* node)
{
	/* A bad way to do it! we should be using a map, really. */

	return (HTREEITEM) wintree_traverse(wintree, _wintree_search_node, node);
}
