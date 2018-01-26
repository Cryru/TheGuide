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
#include <string.h>		/* memmove() */
#include <libguide/lut.h>

#define _LUT_INITIAL_SIZE		(512)
#define _LUT_GROW_SIZE			(512)

struct _lut_entry_t
{
	void *lhs;
	void *rhs;
};

struct lut_t
{
	struct _lut_entry_t *entries;
	unsigned n;
	unsigned alloc;
};

struct lut_t *lut_create()
{
	struct lut_t *lut = (struct lut_t *)malloc(sizeof(struct lut_t));
	lut->entries = 
		(struct _lut_entry_t *)malloc(
			_LUT_INITIAL_SIZE * sizeof(struct _lut_entry_t));
	lut->n = 0;
	lut->alloc = _LUT_INITIAL_SIZE;
	return lut;
}

void lut_free(struct lut_t *lut)
{
	free(lut->entries);
	lut->entries = 0;
	lut->n = lut->alloc = 0;
	free(lut);
}

static int _lut_get_index(struct lut_t *lut, void *lhs)
{
	int i;
	for (i=0; i<(int)lut->n; ++i) {
		if (lut->entries[i].lhs == lhs)
			return i;
	}
	return -1;
}

void lut_set(struct lut_t *lut, void *lhs, void *rhs)
{
	struct _lut_entry_t *e = lut->entries;
	unsigned old_idx = _lut_get_index(lut, lhs);
	unsigned n = lut->n;

	if (old_idx != -1) {
		e[old_idx].lhs = rhs;
	} else {
		if (n >= lut->alloc) {
			lut->entries = (struct _lut_entry_t *)realloc(
								lut->entries,
								(lut->alloc + _LUT_GROW_SIZE) *
								 	sizeof(struct _lut_entry_t));
			lut->alloc += _LUT_GROW_SIZE;
		}

		lut->entries[n].lhs = lhs;
		lut->entries[n].rhs = rhs;
		++(lut->n);
	}
}

int lut_get(struct lut_t *lut, void *lhs, void **rhsp)
{
	unsigned idx = _lut_get_index(lut, lhs);
	if (idx != -1) {
		void *rhs = lut->entries[idx].rhs;
		*((int **)rhsp) = (int *) rhs;
		return 0;
	} else {
		return -1;
	}
}
