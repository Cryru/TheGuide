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

class CFlatButtonBase : public CButton
{
	DECLARE_DYNAMIC(CFlatButtonBase)
	bool m_bMouseInHouse;
	bool m_bChecked;

protected:
	DECLARE_MESSAGE_MAP()

public:
	CFlatButtonBase();
	virtual ~CFlatButtonBase();

	void SetChecked(bool checked = true);
	bool IsChecked() const { return m_bChecked; }
	bool IsMouseInHouse() const { return m_bMouseInHouse; }

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
};

// CFlatButton

class CFlatButton : public CFlatButtonBase
{
	DECLARE_DYNAMIC(CFlatButton)

protected:
	DECLARE_MESSAGE_MAP()

public:
	CFlatButton();
	virtual ~CFlatButton();

	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};

// CColorButton

class CColorButton : public CFlatButtonBase
{
	DECLARE_DYNAMIC(CColorButton)
	COLORREF m_Color;

protected:
	DECLARE_MESSAGE_MAP()

public:
	CColorButton();
	virtual ~CColorButton();
	void SetColor(COLORREF c);

	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};
