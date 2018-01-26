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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef BUILD_libguide
#define LIBGUIDEAPI __declspec(dllexport)
#else
#define LIBGUIDEAPI __declspec(dllimport)
#endif

/** A 32-bit unsigned integer. */
#ifdef _MSC_VER
typedef unsigned __int32 uint32;
#else
#error Please define the type uint32 for this compiler.
#endif

#endif // _CONFIG_H_