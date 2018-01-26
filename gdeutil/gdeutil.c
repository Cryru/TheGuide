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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include "argtable2.h"
#include "libguide/guide.h"

#define VERSION		"2.0"

/* default options, overridden with command line args */
int opt_verbose = 0;
int opt_omit_text = 0;

/* helper methods */
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

struct xml_export_cb_data_t
{
	struct tree_t *tree;
	FILE *xml_fp;
};

/* print 'n' spaces */
void fspace(FILE *fp, int n)
{
	while (n--) fprintf(fp, " ");
}

#define INDENT_BY	2 /* spaces */

/* tree traverser for outputting xml */
static int _export_traverser(struct tree_node_t *node, void *cargo, int after)
{
	struct xml_export_cb_data_t *params = (struct xml_export_cb_data_t *)cargo;
	FILE *fp = params->xml_fp;
	struct tree_node_t *root = tree_get_root(params->tree), *node2;
	struct guide_nodedata_t *data = (struct guide_nodedata_t *)tree_get_data(node);
	unsigned char *utf8;
	int level = 0;

	/* get level (for indent) */
	node2 = node;
	while (node2 && node2 != root)
	{
		node2 = tree_get_parent(node2);
		++level;
	}

	if (node == root)
	{
		if (after == 0)
			fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<guide>\r\n");
		else
			fprintf(fp, "</guide>\r\n");
	}
	else
	{
		if (after == 0)
		{
			/* begin node tag */
			fspace(fp, level * INDENT_BY);
			fprintf(fp, "<node state=\"%d\" icon=\"%d\" first_line=\"%d\" color=\"%d\" bgcolor=\"%d\" uid=\"%u\" tc_state=\"%u\">\r\n",
				data->state, data->icon, data->first_line, data->color, data->bgcolor, data->uid, data->tc_state);

			/* convert title to utf8 and add node->title */
			utf8 = convert_to_utf8(data->title);
			assert(utf8);
			fspace(fp, (level+1) * INDENT_BY);
			fprintf(fp, "<title>%s</title>\r\n", utf8);
			free(utf8);

			/* add node->text as CDATA (omit if required) */
			if (! opt_omit_text)
			{
				fspace(fp, (level+1) * INDENT_BY);
				fprintf(fp, "<text format=\"text/rtf\"><![CDATA[");
				fwrite(data->text, strlen(data->text), 1, fp);
				fprintf(fp, "]]></text>\r\n");
			}
		}
		else
		{
			/* end node tag */
			fspace(fp, level * INDENT_BY);
			fprintf(fp, "</node>\r\n");
		}
	}

	return 0; /* always continue */
}

/* the only thing we do as of now */
void export_xml(const char *filename)
{
	struct guide_t *guide;
	uint32 err = 0, format = 0, len;
	wchar_t *uni_filename = 0;
	char *xml_filename;
	FILE *xml_fp;
	struct xml_export_cb_data_t params;

	/* load it */
	uni_filename = convert_to_unicode_from_usercp(filename);
	guide = guide_load(uni_filename, &err, &format);
	if (opt_verbose)
		printf("%s: loaded at %p, error=%lu, format=%lu\n", filename, guide, err, format);
	if (!guide)
	{
		fprintf(stderr, "%s: could not open: code=%lu\n", filename, err);
		return;
	}

	/* open output file */
	xml_filename = (char *)malloc(strlen(filename) + 16);
	strcpy(xml_filename, filename);
	len = strlen(xml_filename);
	if (len < 4 || stricmp(xml_filename + len - 4, ".gde") != 0)
		strcat(xml_filename, ".xml");
	else
		strcpy(xml_filename + len - 4, ".xml");
	if (opt_verbose)
		printf("converting %s -> %s\n", filename, xml_filename);
	xml_fp = fopen(xml_filename, "wb");
	if (!xml_fp)
	{
		fprintf(stderr, "%s: could not open: ", xml_filename);
		perror(NULL);
		return;
	}

	/* setup callback data */
	params.tree = guide_get_tree(guide);
	assert(params.tree);
	params.xml_fp = xml_fp;

	/* traverse the tree */
	tree_traverse_preorder2(params.tree, _export_traverser, &params);

	/* cleanup */
	guide_destroy(guide);
	fclose(xml_fp);
	free(uni_filename);
	free(xml_filename);
}

/* display usage information (--help) */
void usage(void **argtable)
{
	printf("Usage: gdeutil");
	arg_print_syntax(stdout, argtable, "\n");
	printf("Performs the specified ACTION on the given Guide (.gde) files.\n"
		"As of now, only action=export with format=xml are implemented.\n"
		"Exported XML files are created in the same directory as the .gde file.\n"
		"\nOptions:\n");
	arg_print_glossary(stdout, argtable, "  %-25s %s\n");
	printf("\nReport bugs to <mdevan@users.sourceforge.net>.\n");
}

/* main */
int main(int argc, char *argv[])
{
	/* create argtable */
    struct arg_str  *action  = arg_str0("a","action","<action>",	"action (must be \"export\")");
    struct arg_str  *format  = arg_str0("f","format","<format>",	"output format for export (must be \"xml\")");
    struct arg_lit  *omit_txt= arg_lit0(NULL,"omit-text",			"omit <text> tags when exporting to XML");
    struct arg_lit  *verbose = arg_lit0("v","verbose",				"verbose messages");
    struct arg_lit  *help    = arg_lit0(NULL,"help",                "print this help and exit");
    struct arg_lit  *version = arg_lit0(NULL,"version",             "print version information and exit");
    struct arg_file *infiles = arg_filen(NULL,NULL,NULL,1,100,      "input file(s)");
    struct arg_end  *end     = arg_end(20);
    void* argtable[8];
    int nerrors;

	argtable[0] = action;
	argtable[1] = format;
	argtable[2] = omit_txt;
	argtable[3] = verbose;
	argtable[4] = help;
	argtable[5] = version;
	argtable[6] = infiles;
	argtable[7] = end;

	/* parse command line */
	nerrors = arg_parse(argc, argv, argtable);
	if (help->count > 0)
	{
		usage(argtable);
		exit(0);
	}
	if (version->count > 0)
	{
		printf(
			"gdeutil (part of The Guide) " VERSION "\n"
			"Copyright (C) 2007-08 Mahadevan R.\n"
			"\n"
			"This software is distributed under the Apache License v2.0.\n"
			"See The Guide's home page http://theguide.sourceforge.net/ for more details.\n");
		exit(0);
	}
	if (nerrors > 0)
	{ 
        arg_print_errors(stdout, end, "gdeutil");
        printf("Try 'gdeutil --help' for more information.\n");
		exit(1);
	}

	/* process options */
	if (verbose->count > 0)
		opt_verbose = 1;
	if (omit_txt->count > 0)
		opt_omit_text = 1;

	/* perform actions */
	while (infiles->count-- > 0)
	{
		export_xml(*(infiles->filename++));
	}
	
	/* done */
	arg_freetable(argtable, sizeof(argtable)/sizeof(*argtable));
	exit(0);
	return 0;
}