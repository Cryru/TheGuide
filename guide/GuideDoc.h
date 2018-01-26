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

#include <libguide/guide.h>

// helper class to "lock" a file
class CFileLocker
{
private:
	// handle to the file
	HANDLE m_hFile;
	// filename
	CString m_szFileName;

	// no copying
	CFileLocker(const CFileLocker&);
	CFileLocker& operator=(const CFileLocker&);

public:
	CFileLocker();
	~CFileLocker();

	void lock(const CString& fileName);
	void unlock();

	const CString& getFileName() const { return m_szFileName; }
};

class CGuideDoc : public CRichEditDoc
{
protected: // create from serialization only
	CGuideDoc();
	DECLARE_DYNCREATE(CGuideDoc)
	CFileLocker m_Locker;

// Attributes
public:
	struct guide_t *m_pGuide;

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual CRichEditCntrItem* CreateClientItem(REOBJECT* preo) const;
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU);

	// make the protected base class member public
	using CRichEditDoc::OnFileSave;

// Implementation
public:
	virtual ~CGuideDoc();

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};
