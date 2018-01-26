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

#ifndef WINTREE_H
#define WINTREE_H

#include <windows.h>
#include <libguide/config.h>
#include <libguide/guide.h>

#ifdef __cplusplus
extern "C" {
#endif

struct wintree_t;

LIBGUIDEAPI struct wintree_t *wintree_create(struct guide_t *guide,
		 HWND hwnd, int leaf_image_idx, int nonleaf_image_idx);
LIBGUIDEAPI void wintree_destroy(struct wintree_t *wintree);

LIBGUIDEAPI void wintree_populate(struct wintree_t *wintree);
LIBGUIDEAPI HTREEITEM wintree_add_toplevel(struct wintree_t *wintree, 
		const wchar_t *title, const char *data);
LIBGUIDEAPI HTREEITEM wintree_insert_at_current(struct wintree_t *wintree,
		HTREEITEM item, const wchar_t *title, const char *data);
LIBGUIDEAPI HTREEITEM wintree_insert_after_current(struct wintree_t *wintree,
		HTREEITEM item, const wchar_t *title, const char *data);
LIBGUIDEAPI HTREEITEM wintree_add_child(struct wintree_t *wintree,
		HTREEITEM item, const wchar_t *title, const char *data);
LIBGUIDEAPI HTREEITEM wintree_add_child_as_first(struct wintree_t *wintree,
		HTREEITEM item, const wchar_t *title, const char *data);
LIBGUIDEAPI void wintree_rename_item(struct wintree_t *wintree,
		HTREEITEM item, const wchar_t *new_name);
LIBGUIDEAPI void wintree_delete(struct wintree_t *wintree,
		HTREEITEM item);
LIBGUIDEAPI void wintree_set_text(struct wintree_t *wintree,
		HTREEITEM item, const char *text);
LIBGUIDEAPI const char *wintree_get_text(struct wintree_t *wintree,
		HTREEITEM item);

enum move_op_e { 
	WT_MV_BEFORE, WT_MV_AFTER, WT_MV_CHILD,
	WT_CP_BEFORE, WT_CP_AFTER, WT_CP_CHILD
};

LIBGUIDEAPI int wintree_can_move(struct wintree_t *wintree,
		HTREEITEM srcItem, HTREEITEM dstItem);

LIBGUIDEAPI HTREEITEM wintree_move_or_copy(struct wintree_t *wintree,
		HTREEITEM srcItem, HTREEITEM dstItem,
		enum move_op_e op);

typedef int (*wintree_traverser_fn_t)(HTREEITEM item, 
	struct tree_node_t *node, void *cargo);

LIBGUIDEAPI int wintree_traverse(struct wintree_t *wintree,
		wintree_traverser_fn_t tvr, void *cargo);

LIBGUIDEAPI int wintree_traverse_subtree(struct wintree_t *wintree,
		HTREEITEM item, wintree_traverser_fn_t tvr, void *cargo);

LIBGUIDEAPI struct tree_node_t *wintree_get_node_from_item(
		struct wintree_t *wintree, HTREEITEM item);
LIBGUIDEAPI HTREEITEM wintree_get_item_from_node(
		struct wintree_t *wintree, struct tree_node_t* node);

LIBGUIDEAPI struct tree_t *wintree_get_tree(struct wintree_t *wintree);
LIBGUIDEAPI struct guide_t *wintree_get_guide(struct wintree_t *wintree);

LIBGUIDEAPI void wintree_update_icon(struct wintree_t *wintree,
	HTREEITEM item);

#ifdef __cplusplus
}
#endif

#endif /* WINTREE_H */
