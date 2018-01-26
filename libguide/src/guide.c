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

#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <libguide/tree.h>
#include <libguide/lut.h>
#include <libguide/guide.h>

#define guide_get_next_uid(gde)			(++((gde)->_counter))
#define guide_set_next_uid(gde, uid)	(gde)->_counter = ((uid)-1)

struct guide_nodedata_t *guide_nodedata_create(struct guide_t *guide)
{
	return guide_nodedata_create_with_data(guide, NULL, NULL);
}

struct guide_nodedata_t *guide_nodedata_create_with_data(struct guide_t *guide,
	const wchar_t *title, const char *text)
{
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)malloc(sizeof(struct guide_nodedata_t));
	assert(data);
	if (!data) return data;

	data->title = _wcsdup(title ? title : L"");
	data->text  = strdup(text ? text : "");
	data->state = 0;
	data->icon	= 0;
	data->first_line = 0;
	data->color = (uint32)-1;
	data->bgcolor = (uint32)-1;
	data->uid = guide_get_next_uid(guide);
	data->tc_state = 0;

	assert(data->title);
	assert(data->text);

	return data;
}

/* "copy ctor" for a guide_nodedata_t */
struct guide_nodedata_t *guide_nodedata_clone(struct guide_nodedata_t *src, struct guide_t *guide)
{
	struct guide_nodedata_t *old_data = (struct guide_nodedata_t *)src;
	struct guide_nodedata_t *data;

	/* assert valid input */
	assert(old_data);

	/* allocate a new node data */
	data =  (struct guide_nodedata_t *)malloc(sizeof(struct guide_nodedata_t));
	assert(data);
	if (!data) return data;

	data->title = _wcsdup(old_data->title);
	data->text  = strdup(old_data->text);
	data->color = old_data->color;
	data->bgcolor = old_data->bgcolor;
	data->first_line = old_data->first_line;
	data->icon = old_data->icon;
	data->state = old_data->state;
	data->uid = guide_get_next_uid(guide);
	data->tc_state = old_data->tc_state;

	assert(data->title);
	assert(data->text);

	return data;
}

void guide_nodedata_set_title(struct guide_nodedata_t *data, const wchar_t *title)
{
	assert(data);
	assert(data->title);

	free(data->title);
	data->title = _wcsdup(title ? title : L"");

	assert(data->title);
}

void guide_nodedata_set_text(struct guide_nodedata_t *data, const char *text)
{
	char *p;

	assert(data);
	assert(data->text);

	p = strdup(text ? text : "");
	assert(p);
	if (!p) return;

	free(data->text);
	data->text = p;
}

void guide_nodedata_set_textn(struct guide_nodedata_t *data, const char *text, size_t n)
{
	char *p;

	assert(data);
	assert(data->text);
	assert(text);
	assert(n > 0);

	p = (char *)malloc(n+1);
	assert(p);
	if (!p) return;
	memcpy(p, text, n);
	p[n] = 0;

	free(data->text);
	data->text = p;
}

void guide_nodedata_destroy(struct guide_nodedata_t *data)
{
	assert(data);
	assert(data->title);
	assert(data->text);

	free(data->title);
	free(data->text);
	free(data);
}

/*----------------------------------------------------------------------------------------------------*/

struct _guide_mappedfile_t
{
	HANDLE h_file, h_mapping;
	void *data;
};

static struct _guide_mappedfile_t *_guide_map_file(const wchar_t *filename, unsigned *os_errcode)
{
	struct _guide_mappedfile_t *m = (struct _guide_mappedfile_t *)
										malloc(sizeof(struct _guide_mappedfile_t));
	assert(m);
	*os_errcode = ERROR_NOT_ENOUGH_MEMORY;
	if (!m) return m;

	/* open file */
	m->h_file = CreateFile(
					filename, 				GENERIC_READ, 
					FILE_SHARE_READ, 		NULL, 
					OPEN_EXISTING, 			0,
					NULL);
	if (m->h_file == INVALID_HANDLE_VALUE)
	{
		*os_errcode = GetLastError();
		free(m);
		return NULL;
	}

	/* create mapping */
	m->h_mapping = 
		CreateFileMapping(m->h_file, NULL, PAGE_READONLY, 0, 0, NULL);
	if (m->h_mapping == NULL)
	{
		*os_errcode = GetLastError();
		CloseHandle(m->h_file);
		free(m);
		return NULL;
	}

	/* map view */
	m->data = MapViewOfFile(m->h_mapping, FILE_MAP_READ, 0, 0, 0);
	if (!m->data)
	{
		*os_errcode = GetLastError();
		CloseHandle(m->h_mapping);
		CloseHandle(m->h_file);
		free(m);
		return NULL;
	}

	*os_errcode = 0;
	return m;
}

static void _guide_unmap_file(struct _guide_mappedfile_t *m)
{
	UnmapViewOfFile(m->data);
	CloseHandle(m->h_mapping);
	CloseHandle(m->h_file);
	free(m);
}

static unsigned _guide_get_filelength(const wchar_t *filename)
{
	WIN32_FILE_ATTRIBUTE_DATA info;

	if (GetFileAttributesEx(filename, GetFileExInfoStandard, &info) == 0)
		return 0;

	return info.nFileSizeLow;
}

static void _guide_write_attr_count(uint32 count, FILE *fp)
{
	fwrite(&count, 1, 4 /*bytes*/, fp);
}

static void _guide_write_uint32_attr(uint32 attr_id, uint32 *attr_p, FILE *fp)
{
	static const uint32 attr_val_len = sizeof(uint32);

	fwrite(&attr_id,	  1, 4 /*bytes*/, fp);
	fwrite(&attr_val_len, 1, 4 /*bytes*/, fp);
	fwrite(attr_p,		  1, 4 /*bytes*/, fp);
}

static void _guide_write_node_attrs(struct guide_nodedata_t *data, FILE *fp)
{
	// number of attrs = 7
	_guide_write_attr_count(7, fp);

	// 1: state
	_guide_write_uint32_attr(1, &(data->state), fp);
	// 2: icon
	_guide_write_uint32_attr(2, &(data->icon), fp);
	// 3: first_line
	_guide_write_uint32_attr(3, &(data->first_line), fp);
	// 4: color
	_guide_write_uint32_attr(4, &(data->color), fp);
	// 5: bgcolor
	_guide_write_uint32_attr(5, &(data->bgcolor), fp);
	// 6: uid
	_guide_write_uint32_attr(6, &(data->uid), fp);
	// 7: state
	_guide_write_uint32_attr(7, &(data->tc_state), fp);
}

static unsigned char *convert_to_utf8(const wchar_t *s)
{
	int r;
	unsigned char *utf8;

	/* check for s == NULL */
	assert(s);
	if (!s) return NULL;

	/* check for s == L"" */
	if (s[0] == 0)
		return (unsigned char *)strdup("");

	/* find out the number of bytes required for the utf-8 string */
	r = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);
	if (r <= 0)
		return NULL; /* ?? when can this happen? */

	/* allocate memory to hold the utf-8 string */
	utf8 = (unsigned char *)malloc(r + 1);
	if (!utf8)
		return NULL; /* out of memory */

	/* actually convert to utf-8 */
	r = WideCharToMultiByte(CP_UTF8, 0, s, -1, utf8, r+1, NULL, NULL);
	if (r == 0) /* some error occured */
	{
		free(utf8);
		return NULL;
	}

	/* ensure null-terminator */
	utf8[r] = 0;

	/* return the utf8 string */
	return utf8;
}

static wchar_t *convert_to_unicode_from_usercp(const char *s)
{
	int n;
	wchar_t *uni;

	/* calculate how many _wide_chars_ are required to hold the string */
	n = MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED|MB_ERR_INVALID_CHARS, 
			s, -1, NULL, 0);
	if (n <= 0)
		return NULL;

	/* allocate enough memory */
	uni = (wchar_t *)malloc((n+1) * sizeof(wchar_t));
	if (!uni)
		return NULL; /* out of memory */

	/* actually convert */
	if (MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED|MB_ERR_INVALID_CHARS, 
			s, -1, uni, n+1) == 0)
	{
		free(uni);
		return NULL;
	}

	/* ensure null-terminator */
	uni[n] = 0;

	/* return the unicode string */
	return uni;
}

static wchar_t *convert_to_unicode_from_utf8(const char *s, size_t len)
{
	int n;
	wchar_t *uni;

	/* calculate how many _wide_chars_ are required to hold the string */
	n = MultiByteToWideChar(CP_UTF8, 0, //MB_PRECOMPOSED|MB_ERR_INVALID_CHARS, 
			s, (int) len, NULL, 0);
	if (n <= 0)
	{
		DWORD x = GetLastError();
		return NULL;
	}

	/* allocate enough memory */
	uni = (wchar_t *)malloc((n+1) * sizeof(wchar_t));
	if (!uni)
		return NULL; /* out of memory */

	/* actually convert */
	if (MultiByteToWideChar(CP_UTF8, 0, //MB_PRECOMPOSED|MB_ERR_INVALID_CHARS, 
			s, (int) len, uni, n+1) == 0)
	{
		free(uni);
		return NULL;
	}

	/* success */
	uni[n] = 0;
	return uni;
}

static int _guide_write_node_title(struct guide_nodedata_t *data, FILE *fp)
{
	char *utf8 = convert_to_utf8(data->title);
	uint32 len = (uint32) strlen(utf8);

	fwrite(&len, 1, sizeof(len), fp);
	fwrite(utf8, 1, len, fp);
	return 0;
}

static int _guide_write_node_text(struct guide_nodedata_t *data, FILE *fp)
{
	uint32 len = (uint32) strlen(data->text);
	fwrite(&len, 1, sizeof(len), fp);
	fwrite(data->text, 1, len, fp);
	return 0;
}

static int _guide_storer_fn(struct tree_node_t *node, void *cargo)
{
	struct tree_node_t *parent = tree_get_parent(node);
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);
	FILE *fp = (FILE *)cargo;

	assert(node);
	assert(data);
	assert(fp);

// <node_id> <parent_node_id> <node_attrs> <title_len> <title> <text_len> <text>
// attrs := <n_attrs> [ <attr_id> <attr_val_len> <attr_val> ]{n_attrs}

	// node_id
	fwrite(&node,       1, sizeof(node),   		  fp);
	// parent_node_id
	fwrite(&parent,     1, sizeof(parent), 		  fp);
	// node_attrs
	_guide_write_node_attrs(data, fp);
	// title
	_guide_write_node_title(data, fp); /* TODO: check ret val */
	// text
	_guide_write_node_text(data, fp); /* TODO: check ret val */

	return 0;
}

int guide_store(const wchar_t *filename, struct guide_t *guide)
{
	uint32 i;
	FILE *fp = _wfopen(filename, L"wb");
	if (!fp) return -1;

	/* write file header */
// 'GDE' <file_format_version_number> <file_attrs>
	fwrite("GDE", 1, 3, fp);			 /* signature */
	i = 2; fwrite(&i, 1, sizeof(i), fp); /* format version = 2 */

	/* number of attributes = 2 */
	_guide_write_attr_count(2, fp);

	/* attributes: */
	/* 1: _counter */
	_guide_write_uint32_attr(1, &(guide->_counter), fp);
	/* 2: sel_node */
	i = (uint32) guide->sel_node; /* not 64-bit safe */
	_guide_write_uint32_attr(2, &i, fp);

	/* write each node */
	tree_traverse_preorder(guide->tree, _guide_storer_fn, fp);

	fclose(fp);
	return 0;
}

static struct guide_nodedata_t *_guide_get_dummy_data(struct guide_t *guide)
{
	return guide_nodedata_create_with_data(guide, L"dummy", "dummy");
}

static struct tree_node_t *_guide_get_last_child(struct tree_node_t *par)
{
	struct tree_node_t *t, *last = tree_get_first_child(par);
	if (!last)
		return NULL;
	while ((t = tree_get_next_sibling(last)))
		last = t;
	return last;
}

struct guide_t *guide_create()
{
	/* create a guide struct */
	struct guide_t *guide = (struct guide_t *)malloc(sizeof(struct guide_t));
	assert(guide);
	if (!guide)
		return NULL;

	/* init counter to 0 */
	guide->_counter = 0;

	/* create the uid->node* lookup table */
	guide->_uidtbl = lut_create();
	assert(guide->_uidtbl);

	/* NOTE: the counter and uid table has to be created before the tree,
	   because the root node needs a uid and it's pointer has to be entered
	   in the table. */

	/* create tree with dummy data */
	guide->tree = guide_create_with_root(guide, _guide_get_dummy_data(guide));
	assert(guide->tree);
	if (!guide->tree)
	{
		free(guide);
		return NULL;
	}

	/* don't select anything (will be set/changed by app) */
	guide->sel_node = 0;

	return guide;
}

static struct guide_nodedata_t *_guide_read_node_v1(char **pp,
	struct tree_node_t **fake_node_ptr, struct tree_node_t **fake_parent_ptr,
	struct guide_t *guide)
{
	char *utf8_text;
	wchar_t *uni_title, *uni_tmp;
	struct guide_nodedata_t *node_data;
	char *p;

	/* assert valid input */
	assert(pp);
	assert(*pp);
	assert(fake_node_ptr);
	assert(fake_parent_ptr);

	/* read node ptr and parent ptr */
	p = *pp;
	*fake_node_ptr   = ((struct tree_node_t **)p)[0];
	*fake_parent_ptr = ((struct tree_node_t **)p)[1];
	p += 2 * sizeof(struct tree_node_t *);

	/* read title and convert to unicode */
	uni_title = convert_to_unicode_from_usercp(p);
	assert(uni_title);
	p += strlen(p) + 1;

	/* read text and convert to utf8 */
	uni_tmp = convert_to_unicode_from_usercp(p); /* first mbcs -> unicode */
	assert(uni_tmp);
	utf8_text = convert_to_utf8(uni_tmp); /* then unicode -> utf8 */
	assert(utf8_text);
	p += strlen(p) + 1;

	/* create the node data object */
	node_data = guide_nodedata_create_with_data(guide, uni_title, utf8_text);
	assert(node_data);

	/* free temporary strings */
	free(uni_title);
	free(uni_tmp);
	free(utf8_text);

	/* bump the iterator */
	*pp = p;

	/* return the newly created node */
	return node_data;
}

/* Loads a .gde file of format=1 (as written by guide versions <2.0). It
 * assumes default values for new attributes. */
static struct guide_t *guide_load_v1(struct _guide_mappedfile_t *m, unsigned len, unsigned *os_errcode)
{
	char *begin, *end, *p;
	struct guide_t *guide;
	struct lut_t *lm;
	struct guide_nodedata_t *node_data;
	struct tree_node_t *node, *parent;

	/* reset os error code */
	assert(os_errcode);
	*os_errcode = 0;

	/* create a blank guide */
	guide = (struct guide_t *)malloc(sizeof(struct guide_t));
	assert(guide);
	if (!guide)
		return NULL;
	guide->_counter = 0;
	guide->_uidtbl = lut_create();
	guide->sel_node = 0;

	/* create lut for mapping parents */
	lm = lut_create();

	/* first and last bytes of valid data */
	begin = (char *)(m->data);
	end   = (char *)(m->data) + len;
	p	  = begin;

	/* process the root node: */
	/*	-> first read it */
	node_data = _guide_read_node_v1(&p, &node, &parent, guide);
	/*  -> create new tree (with the root node data) */
	guide->tree = guide_create_with_root(guide, node_data);
	/*	-> add to the lookup table */
	lut_set(lm, node, tree_get_root(guide->tree));

	/* iterate through nodes one by one */
	while (p < end)
	{
		struct tree_node_t *real_parent, *new_node;

		/* read a node */
		node_data = _guide_read_node_v1(&p, &node, &parent, guide);
		assert(node_data);

		/* lookup parent */
		lut_get(lm, parent, &real_parent);
		new_node = guide_add_child(guide, real_parent, node_data, 
						_guide_get_last_child(real_parent));

		/* remember node */
		lut_set(lm, node, new_node);
	}

	/* free the lut */
	lut_free(lm);

	return guide;
}

static char *_guide_read_header(struct guide_t *guide, char *begin)
{
	uint32 fmt;
	uint32 i, attr_count;
	assert(begin);

	/* first 3 bytes == 'GDE' */
	if (memcmp(begin, "GDE", 3) != 0)
		return NULL;
	begin += 3;

	/* next 4 bytes is version number */
	fmt = *(uint32 *)begin;
	if (fmt != 2)
		return NULL;
	begin += 4;
	
	/* next 4 bytes is attr_count */
	attr_count = *(uint32 *)begin;
	begin += 4;

	/* for each attr */
	for (i=0; i<attr_count; ++i)
	{
		uint32 attr_id, attr_val_len;

		/* read attr id */
		attr_id = *(uint32 *)begin;
		begin += 4;
		/* read attr len */
		attr_val_len = *(uint32 *)begin;
		begin += 4;

		/* id=1, value=_counter */
		if (attr_id == 1)
			guide->_counter = *(uint32 *)begin;
		/* id=2, value=sel_node */
		else if (attr_id == 2)
			guide->sel_node = *(struct tree_node_t **)begin;

		/* move onto the next attr start */
		begin += attr_val_len;
	}

	/* done */
	return begin;
}

static struct guide_nodedata_t *_guide_read_node_v2(char **pp,
	struct tree_node_t **fake_node_ptr, struct tree_node_t **fake_parent_ptr,
	struct guide_t *guide, uint32 *maxuid)
{
	char *p;
	wchar_t *uni_title;
	struct guide_nodedata_t *node_data;
	uint32 n_attrs, i, title_len, text_len;

	/* assert valid input */
	assert(pp);
	assert(*pp);
	assert(fake_node_ptr);
	assert(fake_parent_ptr);
	assert(maxuid);

	/* read node ptr and parent ptr */
	p = *pp;
	*fake_node_ptr   = ((struct tree_node_t **)p)[0];
	*fake_parent_ptr = ((struct tree_node_t **)p)[1];
	p += 2 * sizeof(struct tree_node_t *);

	/* create a blank node data object */
	node_data = guide_nodedata_create(guide);
	assert(node_data);

	/* read the attrs */
	n_attrs = *(uint32 *)p; p += 4;
	for (i=0; i<n_attrs; ++i)
	{
		uint32 attr_id, attr_val_len;
		attr_id = *(uint32 *)p; p += 4;
		attr_val_len = *(uint32 *)p; p += 4;

		switch (attr_id)
		{
		case 1: node_data->state	  = *(uint32 *)p; break;
		case 2: node_data->icon       = *(uint32 *)p; break;
		case 3: node_data->first_line = *(uint32 *)p; break;
		case 4: node_data->color	  = *(uint32 *)p; break;
		case 5: node_data->bgcolor	  = *(uint32 *)p; break;
		case 6: node_data->uid    	  = *(uint32 *)p; break;
		case 7: node_data->tc_state   = *(uint32 *)p; break;
		default:
			/* must ignore unknown attrs */
			break;
		}

		/* position iterator to start of next attribute */
		p += attr_val_len;
	}

	/* collect the largest uid value */
	if (node_data->uid > *maxuid)
		*maxuid = node_data->uid;

	/* read title */
	title_len = *(uint32 *)p; p += 4;
	uni_title = convert_to_unicode_from_utf8((const char *)p, title_len);
	guide_nodedata_set_title(node_data, uni_title);
	p += title_len;

	/* read text */
	text_len = *(uint32 *)p; p += 4;
	guide_nodedata_set_textn(node_data, p, text_len);
	p += text_len;

	/* free temporary strings */
	free(uni_title);

	/* bump the iterator */
	*pp = p;

	/* return the newly created node */
	return node_data;
}

static struct guide_t *guide_load_v2(struct _guide_mappedfile_t *m, unsigned len, unsigned *os_errcode)
{
	char *begin, *end, *p;
	struct guide_t *guide;
	struct lut_t *lm;
	struct guide_nodedata_t *node_data;
	struct tree_node_t *node, *parent;
	uint32 maxuid;

	/* reset os error code */
	assert(os_errcode);
	*os_errcode = 0;

	/* create a new guide */
	guide = (struct guide_t *)malloc(sizeof(struct guide_t));
	assert(guide);
	if (!guide)
		return NULL;
	guide->_counter = 0; /* filled in by _guide_read_header */
	guide->_uidtbl = lut_create();
	guide->sel_node = 0; /* filled in by _guide_read_header */

	/* create lut for mapping parents */
	lm = lut_create();

	/* first and last bytes of valid data */
	begin = (char *)(m->data);
	end   = (char *)(m->data) + len;

	/* read header */
	p = begin = _guide_read_header(guide, begin);
	assert(begin);
	if (!begin)
	{
		/* file format error */
		lut_free(lm);
		return NULL;
	}

	/* collect the biggest uid */
	maxuid = guide->_counter;

	/* process the root node: */
	/*	-> first read it */
	node_data = _guide_read_node_v2(&p, &node, &parent, guide, &maxuid);
	/*  -> create new tree (with the root node data) */
	guide->tree = guide_create_with_root(guide, node_data);
	/*	-> add to the lookup table */
	lut_set(lm, node, tree_get_root(guide->tree));

	/* iterate through nodes one by one */
	while (p < end)
	{
		struct tree_node_t *new_node, *real_parent;

		/* read a node */
		node_data = _guide_read_node_v2(&p, &node, &parent, guide, &maxuid);

		/* add node to tree */
		/* lookup parent */
		lut_get(lm, parent, &real_parent);
		new_node = guide_add_child(guide, real_parent, node_data, 
						_guide_get_last_child(real_parent));

		/* remember node */
		lut_set(lm, node, new_node);
	}

	/* translate the selected node */
	if (guide->sel_node)
	{
		node = 0;
		lut_get(lm, guide->sel_node, &node);
		guide->sel_node = node;
	}

	/* free the lut */
	lut_free(lm);

	/* set the guide uid to the max uid */
	guide->_counter = maxuid;

	return guide;
}

/* Loads a .gde file, of any file format. It detects the file format
 * and then behaves appropriately. */
struct guide_t *guide_load(const wchar_t *filename, unsigned *os_errcode, uint32 *format)
{
	unsigned len;
	struct _guide_mappedfile_t *m;
	struct guide_t *gde;

	/* assert valid input param */
	assert(filename);
	assert(*filename);
	assert(format);

	/* reset os error code */
	assert(os_errcode);
	*os_errcode = 0;

	/* get file length */
	len = _guide_get_filelength(filename);

	/* The file has to be at least 20 bytes long. This is the size of
	 * a format 1 .gde file, with no nodes. This check was added in v1.6. */
	if (len < 20) /* *os_errcode = 0; */ return NULL;

	/* map the file */
	m = _guide_map_file(filename, os_errcode);
	if (!m) return NULL;

	/* check for format1 signature */
	if (memcmp((char *)(m->data) + 4, "\0\0\0\0dummy\0dummy\0", 16) == 0)
	{
		*format = 1;
		gde = guide_load_v1(m, len, os_errcode);
	}
	/* check for format2 signature */
	else if (memcmp((char *)(m->data), "GDE\x02\0\0\0", 7) == 0)
	{
		*format = 2;
		gde = guide_load_v2(m, len, os_errcode);
	}
	/* not a .gde file */
	else
	{
		*os_errcode = 0;
		gde = NULL;
	}

	/* unmap the file */
	_guide_unmap_file(m);

	/* return the newly loaded guide */
	return gde;
}

/* callback function to cleanup a single node */
static void _guide_deleter(struct tree_node_t *node, void *cargo)
{
	struct guide_nodedata_t *data = 
		(struct guide_nodedata_t *)tree_get_data(node);
	struct guide_t *guide = 
		(struct guide_t *)cargo;

	assert(data);
	assert(guide);

	/* "remove" from the map */
	lut_set(guide->_uidtbl, (void *)(data->uid), NULL);

	guide_nodedata_destroy(data);
}

void guide_destroy(struct guide_t *guide)
{
	assert(guide);

	assert(guide->tree);
	tree_delete_tree(guide->tree, _guide_deleter, guide);
	guide->tree = NULL;

	assert(guide->_uidtbl);
	lut_free(guide->_uidtbl);
	guide->_uidtbl = NULL;
}

void guide_delete_subtree(struct guide_t *guide, struct tree_node_t *node)
{
	assert(node);
	tree_delete_subtree(node, _guide_deleter, guide);
}

struct tree_t *guide_create_with_root(struct guide_t *guide, struct guide_nodedata_t *data)
{
	struct tree_t *p = tree_create_with_root(data);
	if (p)
		lut_set(guide->_uidtbl, (void *)(data->uid), p);
	return p;
}

struct tree_node_t *guide_add_child(struct guide_t *guide, struct tree_node_t *parent,
	struct guide_nodedata_t *data, struct tree_node_t *after)
{
	struct tree_node_t *p = tree_add_child(parent, data, after);
	if (p)
		lut_set(guide->_uidtbl, (void *)(data->uid), p);
	return p;
}

struct tree_node_t *guide_add_sibling_after(struct guide_t *guide, 
	struct tree_node_t *node, struct guide_nodedata_t *data)
{
	struct tree_node_t *p = tree_add_sibling_after(node, data);
	if (p)
		lut_set(guide->_uidtbl, (void *)(data->uid), p);
	return p;
}

struct tree_node_t *guide_add_sibling_before(struct guide_t *guide, 
	struct tree_node_t *node, struct guide_nodedata_t *data)
{
	struct tree_node_t *p = tree_add_sibling_before(node, data);
	if (p)
		lut_set(guide->_uidtbl, (void *)(data->uid), p);
	return p;
}

struct tree_node_t *guide_get_node_by_uid(struct guide_t *guide, uint32 uid)
{
	struct tree_node_t *node = NULL;
	lut_get(guide->_uidtbl, (void *)uid, (void **)&node);
	return node;
}
