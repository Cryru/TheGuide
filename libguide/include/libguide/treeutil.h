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

#ifndef _TREEUTIL_H_
#define _TREEUTIL_H_

typedef void *(*tree_copy_data_fn_t)(void *src, void *cargo);

LIBGUIDEAPI struct tree_node_t *tree_copy_subtree_after(struct tree_node_t *src_node,
		struct tree_node_t *dst_node, tree_copy_data_fn_t copy_fn, void *cargo);
LIBGUIDEAPI struct tree_node_t *tree_copy_subtree_before(struct tree_node_t *src_node,
		struct tree_node_t *dst_node, tree_copy_data_fn_t copy_fn, void *cargo);
LIBGUIDEAPI struct tree_node_t *tree_copy_subtree_as_child(struct tree_node_t *src_node,
		struct tree_node_t *dst_node/*, struct tree_node_t *after*/, 
		tree_copy_data_fn_t copy_fn, void *cargo);

#endif /* _TREEUTIL_H_ */
