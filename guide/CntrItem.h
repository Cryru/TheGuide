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

class CGuideDoc;
class CGuideView;

class CGuideCntrItem : public CRichEditCntrItem
{
	DECLARE_SERIAL(CGuideCntrItem)

// Constructors
public:
	CGuideCntrItem(REOBJECT* preo = NULL, CGuideDoc* pContainer = NULL);
		// Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-NULL document pointer

// Attributes
public:
	CGuideDoc* GetDocument()
		{ return reinterpret_cast<CGuideDoc*>(CRichEditCntrItem::GetDocument()); }
	CGuideView* GetActiveView()
		{ return reinterpret_cast<CGuideView*>(CRichEditCntrItem::GetActiveView()); }

// Implementation
public:
	~CGuideCntrItem();
};

