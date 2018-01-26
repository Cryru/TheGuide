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

struct tree_node_t
{
	void *data;
	struct tree_node_t *prev;
	struct tree_node_t *next;
	struct tree_node_t *parent;
	struct tree_node_t *first_child;
};

struct tree_t
{
	struct tree_node_t *root;
};

struct tree_node_t *tree_get_root(struct tree_t *tree)
{
	assert(tree);
	return tree->root;
}

struct tree_node_t *tree_get_first_child(struct tree_node_t *parent)
{
	assert(parent);
	return parent->first_child;
}

struct tree_node_t *tree_get_next_sibling(struct tree_node_t *node)
{
	assert(node);
	return node->next;
}

struct tree_node_t *tree_get_prev_sibling(struct tree_node_t *node)
{
	assert(node);
	return node->prev;
}

struct tree_node_t *tree_get_parent(struct tree_node_t *node)
{
	assert(node);
	return node->parent;
}

void *tree_get_data(struct tree_node_t *node)
{
	assert(node);
	return node->data;
}

void tree_set_data(struct tree_node_t *node, void *data)
{
	assert(node);
	node->data = data;
}

struct tree_t *tree_create()
{
	struct tree_t *t;

	t = (struct tree_t *)malloc(sizeof(struct tree_t));
	assert(t);
	if (!t) return NULL;
	t->root = NULL;

	return t;
}

struct tree_t *tree_create_with_root(void *root_data)
{
	struct tree_t *t;
	struct tree_node_t *r;

	t = (struct tree_t *)malloc(sizeof(struct tree_t));
	r = (struct tree_node_t *)malloc(sizeof(struct tree_node_t));
	assert(t);
	assert(r);

	r->data = root_data;
	r->prev = r->next = r->parent = r->first_child = NULL;

	t->root = r;

	return t;
}

struct tree_node_t *tree_add_root(struct tree_t *tree, void *data)
{
	struct tree_node_t *root;
	assert(tree);
	assert(tree->root == NULL);

	if (tree->root) return NULL;

	root = (struct tree_node_t *)malloc(sizeof(struct tree_node_t));
	assert(root);
	root->data = data;
	root->prev = root->next = root->parent = root->first_child = NULL;
	tree->root = root;

	return root;
}

/* TODO: Change tree_add_child() semantics, to take `before' rather than
 * `after'?
 */

struct tree_node_t *tree_add_child(struct tree_node_t *parent, void *data, 
		struct tree_node_t *after)
{
	struct tree_node_t *last;
	struct tree_node_t *new_child;

	assert(parent);

	new_child = (struct tree_node_t *)malloc(sizeof(struct tree_node_t));
	assert(new_child);
	new_child->data = data;
	new_child->parent = parent;
	new_child->first_child = NULL;

	/* BUG FIX v1.0+: the `after' flag, when passed as NULL, should
	 * have created a new node as the *first* child */
	if (after) {
		last = parent->first_child;
		if (last) {
			while (last != after && last->next)
				last = last->next;
			new_child->next = last->next;
			new_child->prev = last;
			if (last->next) {
				last->next->prev = new_child;
			}
			last->next = new_child;
		} else {
			new_child->prev = new_child->next = NULL;
			parent->first_child = new_child;
		}
	} else {
		/* `after' was NULL, insert `new_child' as first child of `parent' */
		new_child->prev = NULL;
		new_child->next = parent->first_child;
		if (parent->first_child)
			parent->first_child->prev = new_child;
		parent->first_child = new_child;
	}

	return new_child;
}

struct tree_node_t *tree_add_sibling_after(struct tree_node_t *node, void *data)
{
	struct tree_node_t *new_node;
	assert(node);
	assert(node->parent != NULL);
	
	if (node->parent == NULL) return NULL; /* don't add roots */

	new_node = (struct tree_node_t *)malloc(sizeof(struct tree_node_t));
	assert(new_node);
	new_node->data = data;
	new_node->prev = node;
	new_node->next = node->next;
	new_node->parent = node->parent;
	new_node->first_child = NULL;
	if (node->next)
		node->next->prev = new_node;
	node->next = new_node;

	return new_node;
}

struct tree_node_t *tree_add_sibling_before(struct tree_node_t *node, void *data)
{
	struct tree_node_t *new_node;
	assert(node);
	assert(node->parent != NULL);

	if (node->parent == NULL) return NULL; /* don't add roots */

	new_node = (struct tree_node_t *)malloc(sizeof(struct tree_node_t));
	assert(new_node);
	/* BUG FIX v1.0+: `next' and `first_child' of `new_node' was not set
	 * properly */
	new_node->parent = node->parent;
	new_node->first_child = NULL;
	new_node->data = data;
	new_node->next = node;
	new_node->prev = node->prev;
	if (node->prev)
		node->prev->next = new_node;
	node->prev = new_node;
	if (node->parent->first_child == node)
		node->parent->first_child = new_node;

	return new_node;
}

void tree_merge_tree(struct tree_node_t *node, struct tree_t* tree)
{
	(void)node;
	(void)tree;
	/* TODO */
}

struct _tree_deleter_cargo_t
{
	void *original_cargo;
	tree_node_cleanup_fn_t cleanup_fn;
};

static int _tree_delete_traverser(struct tree_node_t *node, void *cargo)
{
	struct _tree_deleter_cargo_t *c = (struct _tree_deleter_cargo_t *)cargo;
	assert(c);
	if (c->cleanup_fn)
		c->cleanup_fn(node, c->original_cargo);
	free(node);
	return 0;
}

void tree_delete_subtree(struct tree_node_t *node, tree_node_cleanup_fn_t cleanup_fn, void *cargo)
{
	struct _tree_deleter_cargo_t c = { cargo, cleanup_fn };
	/* remember links */
	struct tree_node_t *parent = node->parent, *prev = node->prev, *next = node->next;

	assert(cleanup_fn);

	/* free self and children */
	tree_traverse_subtree_postorder(node, _tree_delete_traverser, (void *)&c);

	/* fixup link: parent's first child */
	if (parent && parent->first_child == node)
	{
		parent->first_child = next;
	}
	/* fixup link: previous node's next link */
	if (prev) prev->next = next;
	/* fixup link: next node's prev link */
	if (next) next->prev = prev;
}

void tree_delete_tree(struct tree_t *tree, tree_node_cleanup_fn_t cleanup_fn, void *cargo)
{
	assert(tree);
	if (tree->root)
		tree_delete_subtree(tree->root, cleanup_fn, cargo);
	free(tree);	
}

struct tree_t *tree_split_subtree(struct tree_node_t *node)
{
/*
	# The following "old" code creates a new tree with more than
	# one root, which is disallowed now. Till we fix this, the
	# code is "disabled".

	struct tree_t *new_tree = 
		(struct tree_t *)malloc(sizeof(struct tree_t));
	assert(new_tree);
	assert(node);
	new_tree->root = node;
	if (node->prev)
		node->prev->next = NULL;
	node->prev = NULL;
	if (node->parent && node->parent->first_child == node)
		node->parent->first_child = NULL;
	do {
		node->parent = NULL;
		node = node->next;
	} while (node);

	return new_tree;
*/
	return NULL;
}

struct tree_node_t *tree_move_subtree_after(struct tree_node_t *src_node,
	struct tree_node_t *dst_node)
{
	assert(src_node);
	assert(dst_node);
	assert(src_node->parent != NULL); /* src cannot be root */
	assert(dst_node->parent != NULL); /* dst cannot be root */

	if (src_node->parent == NULL || dst_node->parent == NULL)
		return NULL; /* src, dst cannot be root */

	/* detach src_node */
	if (src_node->prev)
		src_node->prev->next = src_node->next;
	if (src_node->next)
		src_node->next->prev = src_node->prev;
	if (src_node->parent->first_child == src_node)
		src_node->parent->first_child = src_node->next;

	/* modify src_node's links (prev, next, parent) */
	src_node->prev = dst_node;
	src_node->next = dst_node->next;
	src_node->parent = dst_node->parent;
	/* modify dst_node's next's prev */
	if (dst_node->next)
		dst_node->next->prev = src_node;
	/* modify dst_node's links (next) */
	dst_node->next = src_node;

	return src_node;
}

struct tree_node_t *tree_move_subtree_before(struct tree_node_t *src_node,
	struct tree_node_t *dst_node)
{
	assert(src_node);
	assert(dst_node);
	assert(src_node->parent != NULL); /* src cannot be root */
	assert(dst_node->parent != NULL); /* dst cannot be root */

	if (src_node->parent == NULL || dst_node->parent == NULL)
		return NULL; /* src, dst cannot be root */

	/* detach src_node */
	if (src_node->prev)
		src_node->prev->next = src_node->next;
	if (src_node->next)
		src_node->next->prev = src_node->prev;
	if (src_node->parent->first_child == src_node)
		src_node->parent->first_child = src_node->next;

	/* modify src_node's links (prev, next, parent) */
	src_node->prev = dst_node->prev;
	src_node->next = dst_node;
	src_node->parent = dst_node->parent;
	/* modify dst_node's prev's next */
	if (dst_node->prev)
		dst_node->prev->next = src_node;
	/* modify dst_node's links (prev) */
	dst_node->prev = src_node;
	/* modify dst_node's parent's links (first_child) */
	if (dst_node->parent->first_child == dst_node)
		dst_node->parent->first_child = src_node;

	return src_node;
}

struct tree_node_t *tree_move_subtree_as_child(struct tree_node_t *src_node,
	struct tree_node_t *dst_node/*, struct tree_node_t *after*/)
{
	/* `after' not implemented yet (since we don't use it) */
	assert(src_node);
	assert(dst_node);
	assert(src_node->parent != NULL); /* src cannot be root */
	assert(dst_node->parent != NULL); /* dst cannot be root */

	if (src_node->parent == NULL || dst_node->parent == NULL)
		return NULL; /* src, dst cannot be root */

	/* detach src_node */
	if (src_node->prev)
		src_node->prev->next = src_node->next;
	if (src_node->next)
		src_node->next->prev = src_node->prev;
	if (src_node->parent->first_child == src_node)
		src_node->parent->first_child = src_node->next;

	/* insert `src_node' as first child of `dst_node' */
	src_node->prev = NULL;
	src_node->next = dst_node->first_child;
	src_node->parent = dst_node;
	if (dst_node->first_child)
		dst_node->first_child->prev = src_node;
	dst_node->first_child = src_node;

	return src_node;
}

int tree_traverse_preorder(struct tree_t *tree, tree_traverser_fn_t tvr, void *cargo)
{
	assert(tree);
	assert(tvr);
	if (tree->root)
		return tree_traverse_subtree_preorder(tree->root, tvr, cargo);
	return 0;
}

int tree_traverse_preorder2(struct tree_t *tree, tree_traverser_fn2_t tvr, void *cargo)
{
	assert(tree);
	assert(tvr);
	if (tree->root)
		return tree_traverse_subtree_preorder2(tree->root, tvr, cargo);
	return 0;
}

int tree_traverse_postorder(struct tree_t *tree, tree_traverser_fn_t tvr, void *cargo)
{
	assert(tree);
	assert(tvr);
	if (tree->root)
		return tree_traverse_subtree_postorder(tree->root, tvr, cargo);
	return 0;
}

int tree_traverse_subtree_preorder(struct tree_node_t *node, tree_traverser_fn_t tvr,
		void *cargo)
{
	int ret;
	struct tree_node_t *curr;
	assert(node);
	assert(tvr);

	/* process self, then visit all children */
	ret = tvr(node, cargo);
	if (ret) return ret;

	curr = node->first_child;
	while (curr) {
		/* for consistency with tree_traverse_subtree_postorder(), remember next node */
		struct tree_node_t *next = curr->next;
		ret = tree_traverse_subtree_preorder(curr, tvr, cargo);
		if (ret) return ret;
		curr = next;
	}

	return 0;
}

int tree_traverse_subtree_preorder2(struct tree_node_t *node, tree_traverser_fn2_t tvr,
		void *cargo)
{
	int ret;
	struct tree_node_t *curr;
	assert(node);
	assert(tvr);

	/* process self, then visit all children */
	/* call tvr with '0' before visiting children */
	ret = tvr(node, cargo, 0);
	if (ret) return ret;

	curr = node->first_child;
	while (curr) {
		/* for consistency with tree_traverse_subtree_postorder(), remember next node */
		struct tree_node_t *next = curr->next;
		ret = tree_traverse_subtree_preorder2(curr, tvr, cargo);
		if (ret) return ret;
		curr = next;
	}

	/* call tvr again, with '1', after visiting children */
	ret = tvr(node, cargo, 1);
	if (ret) return ret;

	return 0;
}

int tree_traverse_subtree_postorder(struct tree_node_t *node, tree_traverser_fn_t tvr, void *cargo)
{
	int ret;
	struct tree_node_t *curr;
	assert(node);
	assert(tvr);

	/* visit all children, then process self */
	curr = node->first_child;
	while (curr) {
		/* curr may get deleted within `tvr', so remember the next node */
		struct tree_node_t *next = curr->next;
		ret = tree_traverse_subtree_postorder(curr, tvr, cargo);
		if (ret) return ret;
		curr = next;
	}

	ret = tvr(node, cargo);
	if (ret) return ret;

	return 0;
}
