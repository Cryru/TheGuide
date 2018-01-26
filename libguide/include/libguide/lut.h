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

#ifndef LUT_H
#define LUT_H

#include <libguide/config.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lut_t;

LIBGUIDEAPI struct lut_t *lut_create();
LIBGUIDEAPI void lut_free(struct lut_t *lut);

LIBGUIDEAPI void lut_set(struct lut_t *lut, void *lhs, void *rhs);
LIBGUIDEAPI int  lut_get(struct lut_t *lut, void *lhs, void **rhs);

#ifdef __cplusplus
}
#endif

#endif // LUT_H
