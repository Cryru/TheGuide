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

#ifndef TREE_H
#define TREE_H

#include <libguide/config.h>

#ifdef __cplusplus
extern "C" {
#endif

struct tree_node_t;
struct tree_t;

typedef void (*tree_node_cleanup_fn_t)(struct tree_node_t *, void *);
typedef int (*tree_traverser_fn_t)(struct tree_node_t *, void *);
typedef int (*tree_traverser_fn2_t)(struct tree_node_t *, void *, int);

LIBGUIDEAPI struct tree_node_t *tree_get_root(struct tree_t *tree);
LIBGUIDEAPI struct tree_node_t *tree_get_first_child(struct tree_node_t *parent);
LIBGUIDEAPI struct tree_node_t *tree_get_next_sibling(struct tree_node_t *node);
LIBGUIDEAPI struct tree_node_t *tree_get_prev_sibling(struct tree_node_t *node);
LIBGUIDEAPI struct tree_node_t *tree_get_parent(struct tree_node_t *node);
LIBGUIDEAPI void *tree_get_data(struct tree_node_t *node);
LIBGUIDEAPI void tree_set_data(struct tree_node_t *node, void *data);

LIBGUIDEAPI struct tree_t *tree_create();
LIBGUIDEAPI struct tree_t *tree_create_with_root(void *root_data);
LIBGUIDEAPI void tree_delete_subtree(struct tree_node_t *node, tree_node_cleanup_fn_t cleanup_fn,
		void *cargo);
LIBGUIDEAPI void tree_delete_tree(struct tree_t *tree, tree_node_cleanup_fn_t cleanup_fn,
		void *cargo);

LIBGUIDEAPI void tree_merge_tree(struct tree_node_t *node, struct tree_t* tree);

LIBGUIDEAPI struct tree_t *tree_split_subtree(struct tree_node_t *node);

LIBGUIDEAPI struct tree_node_t *tree_move_subtree_after(struct tree_node_t *src_node,
		struct tree_node_t *dst_node);
LIBGUIDEAPI struct tree_node_t *tree_move_subtree_before(struct tree_node_t *src_node,
		struct tree_node_t *dst_node);
LIBGUIDEAPI struct tree_node_t *tree_move_subtree_as_child(struct tree_node_t *src_node,
		struct tree_node_t *dst_node/*, struct tree_node_t *after*/);

LIBGUIDEAPI struct tree_node_t *tree_add_root(struct tree_t *tree, void *data);
LIBGUIDEAPI struct tree_node_t *tree_add_child(struct tree_node_t *parent, void *data, 
		struct tree_node_t *after);
LIBGUIDEAPI struct tree_node_t *tree_add_sibling_after(struct tree_node_t *node, void *data);
LIBGUIDEAPI struct tree_node_t *tree_add_sibling_before(struct tree_node_t *node, void *data);

LIBGUIDEAPI int tree_traverse_preorder(struct tree_t *tree, tree_traverser_fn_t tvr, void *cargo);
LIBGUIDEAPI int tree_traverse_preorder2(struct tree_t *tree, tree_traverser_fn2_t tvr, void *cargo);
LIBGUIDEAPI int tree_traverse_postorder(struct tree_t *tree, tree_traverser_fn_t tvr, void *cargo);
LIBGUIDEAPI int tree_traverse_subtree_preorder(struct tree_node_t *node, tree_traverser_fn_t tvr, void *cargo);
LIBGUIDEAPI int tree_traverse_subtree_preorder2(struct tree_node_t *node, tree_traverser_fn2_t tvr, void *cargo);
LIBGUIDEAPI int tree_traverse_subtree_postorder(struct tree_node_t *node, tree_traverser_fn_t tvr, void *cargo);

#ifdef __cplusplus
}
#endif

#endif // TREE_H
