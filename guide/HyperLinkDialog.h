/* Copyright 2007-08 Mahadevan R
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

struct guide_t;
struct tree_node_t;

class CHyperLinkDialog : public CDialog
{
	// dyncreate
	DECLARE_DYNAMIC(CHyperLinkDialog)

private:
	// private methods
	void SetLink(const CString& link);
	void SelectRadio(int idx);
	void EnableAppropriately();

	// private vars
	struct guide_t *m_pGuide;
	struct tree_node_t *m_pNode;
	unsigned int m_nNodeUID;
	bool m_bClearLink;

public:
	// ctor/dtor
	CHyperLinkDialog(const CString& sel, struct guide_t *guide, 
		struct tree_node_t *node, CWnd* pParent = NULL);
	virtual ~CHyperLinkDialog();

	// public APIs
	CString GetText() { return m_cText; }
	CString GetLink();
	bool IsClearLink() { return m_bClearLink; }

	// overrides
	void DoDataExchange(CDataExchange* pDX);
	BOOL OnInitDialog();

	// DDX vars
	CString m_cText;
	CString m_cNode;
	CString m_cFileDir;
	CString m_cEmail;
	CString m_cURL;
	int m_cLinkType;

	// message handlers
	afx_msg void OnBnClickedClearlink();
	afx_msg void OnBnClickedUpdatelink();
	afx_msg void OnBnClickedChoosedir();
	afx_msg void OnBnClickedChoosefile();
	afx_msg void OnBnClickedChoosenode();

	// the message map
	DECLARE_MESSAGE_MAP()
};
