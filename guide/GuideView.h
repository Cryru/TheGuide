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

class RTFConcatenator;
class CGuideCntrItem;

#include "FgColorDialog.h"

class CRightViewColorDialog : public CFgColorDialog
{
public:
	CRightViewColorDialog(WPARAM wParam) : m_wParam(wParam) { }
	void OnSelectColor(COLORREF selectedColor);
private:
	WPARAM m_wParam;
};

class CGuideView : public CRichEditView
{
protected: // create from serialization only
	CGuideView();
	DECLARE_DYNCREATE(CGuideView)
	bool m_bClearStatusBar;

// Attributes
public:
	CGuideDoc* GetDocument() const;
	CMenu m_RMBMenu;
	CRightViewColorDialog m_fgColorDialog;
	CRightViewColorDialog m_bgColorDialog;

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate();
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);

// Implementation
private:
	WORD wLastNumbering;
	void SetNumbering(WORD wNumbering);
	void ClearNumbering();
	void SetSpacing(LONG dyLineSpacing);
	void ProcessHyperlink(const CString& link);
	void SwitchFocus();
	void ApplyFont();
	void ApplyColor();
	void OnTreeRmbPrint(HTREEITEM hItem);
	void OnPreviewPrint(PrintType printType);
	void OnTreeRmbPreview(HTREEITEM hItem);
	void OnPreviewPreview(PrintType printType);
	void OnFilePrintPreviewWrapper();
	CString GetCurPos();
	void OnSetBgColor(COLORREF cr);
	void OnSetFgColor(COLORREF cr);

public:
	virtual ~CGuideView();

// Generated message map functions
protected:
	afx_msg void OnDestroy();
	afx_msg void OnGetCharFormat(NMHDR* pNMHDR, LRESULT* pRes);
	afx_msg void OnSetCharFormat(NMHDR* pNMHDR, LRESULT* pRes);
	void OnNumber();
	void OnUpdateNumber(CCmdUI* pCmdUI);
	void OnBackgroundColor();
	void OnLineSpacing();
	void OnForegroundColor();
	afx_msg LRESULT OnGuideUI(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateBullet(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
public:
	void UpdateEnabledState();
	afx_msg void OnFmtnumArabic();
	afx_msg void OnFmtnumLowerAlpha();
	afx_msg void OnFmtnumLowerRoman();
	afx_msg void OnFmtnumUpperAlpha();
	afx_msg void OnFmtnumUpperRoman();
	afx_msg void OnFormatLinespcOne();
	afx_msg void OnFormatLinespcOnehalf();
	afx_msg void OnFormatLinespcTwo();
private:
	void OnDropDownNumberingButton();
public:
	afx_msg void OnFormatStrikethrough();
	afx_msg void OnUpdateFormatStrikethrough(CCmdUI *pCmdUI);
private:
	void SetUserPreferences();

public:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnFormatFont();
	afx_msg void OnEnLink(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFormatHyperlink();
	afx_msg void OnEnMsgfilter(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	long MyPrintPage(CDC* pDC, long nIndexStart, long nIndexStop);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	RTFConcatenator* m_pRTFConcatenator;
public:
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	afx_msg void OnFormatDecindent();
	afx_msg void OnFormatIncindent();
	afx_msg void OnFileExport();
protected:
	virtual void OnEndPrintPreview(CDC* pDC, CPrintInfo* pInfo, POINT point, CPreviewView* pView);
public:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnFilePrint();
	afx_msg void OnEditInsertdate();
	afx_msg void OnEditInserttime();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnEditPasteastext();
	afx_msg void OnUpdateEditPasteastext(CCmdUI *pCmdUI);
};

inline CGuideDoc* CGuideView::GetDocument() const
   { return reinterpret_cast<CGuideDoc*>(m_pDocument); }
