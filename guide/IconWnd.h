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

// The icons.

class CGuideIcons
{
	CImageList m_ImageList;

public:
	CImageList& GetImageList();
	int GetNonLeafIdx() const;
	int GetLeafIdx() const;
};

// the "singleton" antipattern..
extern CGuideIcons theIcons;


// CIconWnd

class CIconWnd : public CMiniFrameWnd 
{
	DECLARE_DYNAMIC(CIconWnd)

public:
	CIconWnd();
	virtual ~CIconWnd();
	void Create(CWnd *parent, int x, int y);

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

