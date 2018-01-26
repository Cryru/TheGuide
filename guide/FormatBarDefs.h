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

#pragma once

static const UINT BUTTON_FONTNAME = 0;
static const UINT BUTTON_FONTSIZE = 1;

static const UINT WIDTH_FONTNAME = 180;
static const UINT WIDTH_FONTSIZE = 60;

static const UINT BASED_CODE formatBarButtons[] =
{
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_FORMAT_BOLD,
	ID_FORMAT_ITALIC,
	ID_FORMAT_UNDERLINE,
	ID_FORMAT_STRIKETHROUGH,
	ID_SEPARATOR,
	ID_FORMAT_LEFTJUSTIFIED,
	ID_FORMAT_CENTERJUSTIFIED,
	ID_FORMAT_RIGHTJUSTIFIED,
	ID_FORMAT_LINESPACING,
	ID_SEPARATOR,
	ID_FORMAT_NUMBERED,
	ID_FORMAT_BULLETED,
	ID_FORMAT_DECINDENT,
	ID_FORMAT_INCINDENT,
	ID_SEPARATOR,
	ID_FORMAT_BACKGROUNDCOLOUR,
	ID_FORMAT_FOREGROUNDCOLOUR,
};

static const int formatBarButtonCount = 
	sizeof(formatBarButtons)/sizeof(*formatBarButtons);
