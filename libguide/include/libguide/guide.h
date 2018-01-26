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

#ifndef GUIDE_H
#define GUIDE_H

#include <libguide/config.h>
#include <libguide/tree.h>

#ifdef __cplusplus
extern "C" {
#endif

struct guide_t;

/*-----------------------------------------------------------------------------------------------*/

/**
 * The contents and properties of a node.
 */
struct guide_nodedata_t
{
	/** The title (node name), in unicode format. Should not be modified. */
	wchar_t *title;

	/** The text (node contents), in UTF-8 format. Should not be modified. */
	char *text;

	/** 
	 * The state of the node. See the enum guide_nodedata_state_e for
	 * possible values.
	 */
	uint32 state;

	/** A number representing the icon for the node. 0 = default. */
	uint32 icon;

	/** Index of the first visible line in the text pane. */
	uint32 first_line;

	/**
	 * The text color of the node in the tree pane. A value of (uint32)(-1)
	 * means default. Else cast to COLORREF and use.
	 */
	uint32 color;

	/**
	 * The bg color of the node in the tree pane. A value of (uint32)(-1)
	 * means default. Else cast to COLORREF and use.
	 */
	uint32 bgcolor;

	/** The unique identifier of the node. */
	uint32 uid;

	/** The state value (TVITEM::state) of the node. */
	uint32 tc_state;
};

/**
 * How to interpret the 'state' field in guide_nodedata_t. This field is a 
 * bitmask, with the various bits representing the properties listed here.
 */
enum guide_nodedata_state_e
{
	NS_EXPANDED		= 0x0001		/**< if set, node should be expanded */
};

/* operations on the node data structure */

LIBGUIDEAPI struct guide_nodedata_t *guide_nodedata_create(struct guide_t *guide);
LIBGUIDEAPI struct guide_nodedata_t *guide_nodedata_create_with_data(struct guide_t *guide,
	const wchar_t *title, const char *text);
LIBGUIDEAPI struct guide_nodedata_t *guide_nodedata_clone(struct guide_nodedata_t *src, struct guide_t *guide);
LIBGUIDEAPI void guide_nodedata_destroy(struct guide_nodedata_t *data);

LIBGUIDEAPI void guide_nodedata_set_title(struct guide_nodedata_t *data, const wchar_t *title);
LIBGUIDEAPI void guide_nodedata_set_text(struct guide_nodedata_t *data, const char *text);
LIBGUIDEAPI void guide_nodedata_set_textn(struct guide_nodedata_t *data, const char *text, size_t n);

#define guide_nodedata_set_expanded(data, is_expanded)  \
	((is_expanded) ? ((data)->state |= NS_EXPANDED) : ((data)->state &= ~NS_EXPANDED))
#define guide_nodedata_get_expanded(data)  ((data)->state & NS_EXPANDED)


/*-----------------------------------------------------------------------------------------------*/

/**
 * The guide data structure. From v2.0, the guide is no longer just a plain
 * tree, but has some more properties. The tree itself, with it's nodes having
 * data as pointers to heap-allocated guide_nodedata_t structures, is contained
 * within this structure.
 */
struct guide_t
{
	/** The tree. */
	struct tree_t *tree;

	/** A counter used internally, to create node UIDs. */
	uint32 _counter;

	/** Lookup table of uid -> node pointer. (not serialized). */
	struct lut_t *_uidtbl;

	/**
	 * The selected node. This is serialized, as well as translated
	 * to the new pointer upon load.
	 */
	struct tree_node_t *sel_node;
};

/* operations on the guide itself */

/** Create a new, empty guide. */
LIBGUIDEAPI struct guide_t *guide_create();
/** Load a guide from a file. */
LIBGUIDEAPI struct guide_t *guide_load(const wchar_t *filename, unsigned *os_errcode,
	uint32 *format);
/** Store guide into disk. */
LIBGUIDEAPI int guide_store(const wchar_t *filename, struct guide_t *gde);
/** Destroy the guide object. Do not use the pointer after this call. */
LIBGUIDEAPI void guide_destroy(struct guide_t *gde);
/** Delete a subtree. Do not use tree_delete_subtree() directly. */
LIBGUIDEAPI void guide_delete_subtree(struct guide_t *guide, struct tree_node_t *node);
/** Get the tree contained in the guide. */
#define guide_get_tree(gde)		((gde)->tree)

LIBGUIDEAPI struct tree_t *guide_create_with_root(struct guide_t *guide, struct guide_nodedata_t *data);
LIBGUIDEAPI struct tree_node_t *guide_add_child(struct guide_t *guide, struct tree_node_t *parent,
	struct guide_nodedata_t *data, struct tree_node_t *after);
LIBGUIDEAPI struct tree_node_t *guide_add_sibling_after(struct guide_t *guide, 
	struct tree_node_t *node, struct guide_nodedata_t *data);
LIBGUIDEAPI struct tree_node_t *guide_add_sibling_before(struct guide_t *guide, 
	struct tree_node_t *node, struct guide_nodedata_t *data);

// note: uid will be >0.
LIBGUIDEAPI struct tree_node_t *guide_get_node_by_uid(struct guide_t *guide, uint32 uid);

/*-----------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif // GUIDE_H
