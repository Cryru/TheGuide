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

#include <stdlib.h>
#include <assert.h>
#include <libguide/tree.h>
#include <libguide/treeutil.h>
#include <libguide/lut.h>

struct _tree_copy_info
{
	struct tree_node_t *src_root;
	struct tree_node_t *dst_root;
	struct lut_t *lut;
	tree_copy_data_fn_t copy_fn;
	void *cargo;
};

/* TODO: Change tree_add_child() semantics, to take `before' rather than
 * `after'.
 */
static struct tree_node_t* _tree_get_last_child(struct tree_node_t *parent)
{
	struct tree_node_t *t, *last = tree_get_first_child(parent);
	if (!last)
		return NULL;
	while ((t = tree_get_next_sibling(last)))
		last = t;
	return last;
}

static int _tree_copier(struct tree_node_t *node, void *cargo)
{
	struct _tree_copy_info *copy_info = (struct _tree_copy_info *)cargo;
	struct tree_node_t *parent = tree_get_parent(node), *new_parent = NULL, *new_node;
	void *new_data;

	/* don't try to insert root (it's done by caller) */
	if (node == copy_info->src_root)
		return 0;

	/* get new parent */
	if (parent == copy_info->src_root)
		new_parent = copy_info->dst_root;
	else
		lut_get(copy_info->lut, parent, &new_parent);
	assert(new_parent);

	/* get new data */
	new_data = tree_get_data(node);
	if (copy_info->copy_fn) 
		new_data = copy_info->copy_fn(new_data, copy_info->cargo);

	/* insert new node */
	new_node = 
		tree_add_child(new_parent, new_data, _tree_get_last_child(new_parent));

	/* remember: node -> new_node */
	lut_set(copy_info->lut, node, new_node);

	return 0;
}

struct tree_node_t *tree_copy_subtree_after(struct tree_node_t *src_node,
		struct tree_node_t *dst_node, tree_copy_data_fn_t copy_fn, void *cargo)
{
	void *new_data;
	struct tree_node_t *copy_root;

	assert(src_node);
	assert(dst_node);
	assert(tree_get_parent(src_node));	/* src cannot be root */
	assert(tree_get_parent(dst_node));	/* dst cannot be root */

	if (!src_node || !dst_node || 
		!tree_get_parent(src_node) || !tree_get_parent(dst_node))
		return NULL;

	/* insert src_node copy */
	new_data = tree_get_data(src_node);
	if (copy_fn)
		new_data = copy_fn(new_data, cargo);
	copy_root = tree_add_sibling_after(dst_node, new_data);
	assert(copy_root); /* TODO: if this operation fails, we need to cleanup
					      the newly-created data and then exit */

	/* copy children if present */
	if (tree_get_first_child(src_node)) {
		struct lut_t *lut = lut_create();
		struct _tree_copy_info copy_info = {
			src_node, copy_root, lut, copy_fn, cargo
		};

		tree_traverse_subtree_preorder(src_node, _tree_copier, &copy_info);
		lut_free(lut);
	}

	return copy_root;
}

/* TODO: refactor out common code in below fns. */
struct tree_node_t *tree_copy_subtree_before(struct tree_node_t *src_node,
		struct tree_node_t *dst_node, tree_copy_data_fn_t copy_fn, void *cargo)
{
	void *new_data;
	struct tree_node_t *copy_root;

	assert(src_node);
	assert(dst_node);
	assert(tree_get_parent(src_node));	/* src cannot be root */
	assert(tree_get_parent(dst_node));	/* dst cannot be root */

	if (!src_node || !dst_node || 
		!tree_get_parent(src_node) || !tree_get_parent(dst_node))
		return NULL;

	/* insert src_node copy */
	new_data = tree_get_data(src_node);
	if (copy_fn)
		new_data = copy_fn(new_data, cargo);
	copy_root = tree_add_sibling_before(dst_node, new_data);
	assert(copy_root); /* TODO: if this operation fails, we need to cleanup
					      the newly-created data and then exit */

	/* copy children if present */
	if (tree_get_first_child(src_node)) {
		struct lut_t *lut = lut_create();
		struct _tree_copy_info copy_info = {
			src_node, copy_root, lut, copy_fn, cargo
		};

		tree_traverse_subtree_preorder(src_node, _tree_copier, &copy_info);
		lut_free(lut);
	}

	return copy_root;
}

struct tree_node_t *tree_copy_subtree_as_child(struct tree_node_t *src_node,
		struct tree_node_t *dst_node/*, struct tree_node_t *after*/,
		tree_copy_data_fn_t copy_fn, void *cargo)
{
	void *new_data;
	struct tree_node_t *copy_root;

	assert(src_node);
	assert(dst_node);
	assert(tree_get_parent(src_node));	/* src cannot be root */
	assert(tree_get_parent(dst_node));	/* dst cannot be root */

	if (!src_node || !dst_node || 
		!tree_get_parent(src_node) || !tree_get_parent(dst_node))
		return NULL;

	/* insert src_node copy */
	new_data = tree_get_data(src_node);
	if (copy_fn)
		new_data = copy_fn(new_data, cargo);
	copy_root = tree_add_child(dst_node, new_data, NULL);
	/* TODO: the NULL can be accepted as `after' */
	assert(copy_root); /* TODO: if this operation fails, we need to cleanup
					      the newly-created data and then exit */

	/* copy children if present */
	if (tree_get_first_child(src_node)) {
		struct lut_t *lut = lut_create();
		struct _tree_copy_info copy_info = {
			src_node, copy_root, lut, copy_fn, cargo
		};

		tree_traverse_subtree_preorder(src_node, _tree_copier, &copy_info);
		lut_free(lut);
	}

	return copy_root;
}
