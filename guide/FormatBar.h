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

struct CHARHDR : public tagNMHDR
{
	CHARFORMAT2 cf;
	CHARHDR()
	{ 
		memset(&cf, 0, sizeof(cf));
		cf.cbSize = sizeof(CHARFORMAT2);
	}
};

static const unsigned int FN_SETFORMAT = 0x1000;
static const unsigned int FN_GETFORMAT = 0x1001;

class CFormatBar : public CToolBar
{
	DECLARE_DYNAMIC(CFormatBar)

private:
	CComboBox m_comboFontName;
	CComboBox m_comboFontSize;
	CFont m_Font;

	void SyncToView();

public:
	CFormatBar();
	void PositionCombos();
	virtual ~CFormatBar();

	void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnSelectFontName();
	void OnSelectFontSize();
	BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
};
