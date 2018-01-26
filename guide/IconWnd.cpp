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

#include "stdafx.h"
#include "Guide.h"
#include ".\iconwnd.h"

//---------------------------------------------------------------------
// CGuideIcons

static void AddBitmap(CImageList& il, UINT id)
{
	CBitmap bmp;
	bmp.LoadBitmap(id);
	il.Add(&bmp, RGB(255,255,0));
}

CImageList& CGuideIcons::GetImageList()
{
	if (m_ImageList.GetSafeHandle() != NULL)
		return m_ImageList;

	// create it first now
	m_ImageList.Create(16, 16, ILC_MASK|ILC_COLOR24, 42, 0);

	// the last 2 are the default leaf and non-leaf icons
	AddBitmap(m_ImageList, IDB_ICONS);
	AddBitmap(m_ImageList, IDB_BOOK);
	AddBitmap(m_ImageList, IDB_PAGE);

	// that's it!
	return m_ImageList;
}

int CGuideIcons::GetNonLeafIdx() const
{
	// see implementation of GetImageList() above
	return 40;
}

int CGuideIcons::GetLeafIdx() const
{
	// see implementation of GetImageList() above
	return 41;
}

// the "singleton" antipattern..
CGuideIcons theIcons;

//---------------------------------------------------------------------
// CIconWnd

IMPLEMENT_DYNAMIC(CIconWnd, CMiniFrameWnd)

CIconWnd::CIconWnd()
{
}

CIconWnd::~CIconWnd()
{
}

BEGIN_MESSAGE_MAP(CIconWnd, CMiniFrameWnd)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// CIconWnd message handlers

void CIconWnd::Create(CWnd *parent, int x, int y)
{
	CMiniFrameWnd::Create(NULL, RCSTR(IDS_ICONWND_TITLE),
		WS_POPUP|WS_CAPTION|WS_SYSMENU|MFS_SYNCACTIVE|MFS_BLOCKSYSMENU, CRect(x,y,x+208+6,y+88+22), parent);
	ShowWindow(SW_SHOW);
}

#define X_OFFSET	(6)
#define Y_OFFSET	(4)

void CIconWnd::OnPaint()
{
	CPaintDC dc(this);

	CRect r;
	GetClientRect(r);
	CImageList& il = theIcons.GetImageList();

	::FillRect(dc, r, (HBRUSH)::GetStockObject(WHITE_BRUSH));

	int idx = 0;
	for (int r=0; r<4; ++r)
		for (int c=0; c<10; ++c, ++idx)
			il.Draw(&dc, idx, CPoint(X_OFFSET + (16 + 4)*c, Y_OFFSET + (16 + 4)*r), ILD_NORMAL);
}

void CIconWnd::OnDestroy()
{
	CMiniFrameWnd::OnDestroy();

	theApp.GetLeftView()->SendMessage(WM_GUIDEUI, GM_ICONWND_DESTROYED);
}

void CIconWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	int r = (point.y - Y_OFFSET)/20;
	int c = (point.x - X_OFFSET)/20;
	// adjust for right/bottom extremes
	if (r > 3) r = 3;
	if (c > 9) c = 9;
	int idx = r*10 + c;

	theApp.GetLeftView()->SendMessage(WM_GUIDEUI, GM_SET_NODE_ICON, idx);

	CMiniFrameWnd::OnLButtonDown(nFlags, point);
}
