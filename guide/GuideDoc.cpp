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
#include "GuideDoc.h"
#include "CntrItem.h"
#include "Utils.h"

#include <libguide/guide.h>

// the extension used for backup files (omit the .)
#define EXTN_BAK	_T("bak")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// CFileLocker

CFileLocker::CFileLocker()
	: m_hFile(NULL)
{
	// empty
}

CFileLocker::~CFileLocker()
{
	ASSERT(m_hFile == NULL);
	if (m_hFile)
		unlock();
}

void CFileLocker::lock(const CString& fileName)
{
	ASSERT(fileName.GetLength() > 0);
	ASSERT(m_hFile == NULL);
	if (m_hFile)
		unlock();

	m_hFile = ::CreateFile(fileName, GENERIC_READ|GENERIC_WRITE,
				FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING,
				NULL);
	ASSERT(m_hFile);

	if (m_hFile)
		m_szFileName = fileName;
}

void CFileLocker::unlock()
{
	ASSERT(m_hFile);
	if (m_hFile)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
		m_szFileName.Empty();
	}
}

///////////////////////////////////////////////////////////////////////////////
// CGuideDoc

IMPLEMENT_DYNCREATE(CGuideDoc, CRichEditDoc)

BEGIN_MESSAGE_MAP(CGuideDoc, CRichEditDoc)
	// Enable default OLE container implementation
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, CRichEditDoc::OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS, CRichEditDoc::OnEditLinks)
	ON_UPDATE_COMMAND_UI_RANGE(ID_OLE_VERB_FIRST, ID_OLE_VERB_LAST, CRichEditDoc::OnUpdateObjectVerbMenu)
END_MESSAGE_MAP()

CGuideDoc::CGuideDoc()
	: m_pGuide(0)
{
}

CGuideDoc::~CGuideDoc()
{
	ASSERT(m_pGuide == NULL);
}

BOOL CGuideDoc::OnNewDocument()
{
	if (!CRichEditDoc::OnNewDocument())
		return FALSE;

	ASSERT(m_pGuide == NULL);
	m_pGuide = guide_create();

	return TRUE;
}

CRichEditCntrItem* CGuideDoc::CreateClientItem(REOBJECT* preo) const
{
	return new CGuideCntrItem(preo, const_cast<CGuideDoc*>(this));
}

void CGuideDoc::DeleteContents()
{
	// notify left view (so that it can delete all the items while
	// the tree is still valid)
	::SendMessage(theApp.GetLeftWnd(), WM_GUIDEUI, GM_ABOUT_TO_DELETE, 0);

	CRichEditDoc::DeleteContents();
	if (m_pGuide)
	{
		guide_destroy(m_pGuide);
		m_pGuide = NULL;

		// "unlock" the file (if any -- DeleteContents() is called
		// for every File->New, which means there is no associated
		// file name).
		if (m_Locker.getFileName().IsEmpty() == FALSE)
			m_Locker.unlock();
	}
}

static CString getMessage(DWORD errCode) 
{ 
	CString result;
    LPVOID lpMsgBuf = 0;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, errCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf, 0, NULL);

	if (lpMsgBuf)
	{
		result = static_cast<LPCTSTR>(lpMsgBuf);
		LocalFree(lpMsgBuf);
	}

	return result;
}

static void BackupFile(LPCTSTR lpszPathName)
{
	size_t len = _tcslen(lpszPathName);
	if (len > 4 && _tcsicmp(lpszPathName + len-4, _T(".gde")) == 0)
	{
		CString newFile(lpszPathName);
		// delete the last 3 chars (gde) and append "bak"
		newFile.Delete((int)len-3, 3);
		newFile += EXTN_BAK;

		// (re)set the attributes of the backup file
		SetFileAttributes(newFile, FILE_ATTRIBUTE_NORMAL);
		// copy .gde -> .bak
		if (CopyFile(lpszPathName, newFile, FALSE) == FALSE)
		{
			DWORD err = GetLastError();
			CString msg;
			msg.FormatMessage(IDS_NOBACKUP, getMessage(err));
			AfxMessageBox(msg, MB_OK|MB_ICONWARNING);
		}
	}
}

static bool _GetFileSize(LPCTSTR lpszPathName, DWORD& size)
{
	WIN32_FILE_ATTRIBUTE_DATA info;

	if (GetFileAttributesEx(lpszPathName, GetFileExInfoStandard, &info) == 0)
		return false;

	size = info.nFileSizeLow;
	return true;
}

BOOL CGuideDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	// create backup if required
	if (theApp.GetAutoBackup())
		BackupFile(lpszPathName);

	// == [0-byte file handling] ============================================
	//
	// Treat specially if file is 0 bytes. This is required so that
	// when the app is asked to open a 0 byte file, it acts as though
	// it is doing a file->new, but with the associated file name.
	// This is required when you do New -> Guide from the explorer
	// RMB. Explorer creates a 0 byte file and calls Guide.exe with
	// the filename as argument.
	DWORD fileSize = 0;
	if (_GetFileSize(lpszPathName, fileSize) && fileSize == 0)
	{
		// delete current contents
		DeleteContents();
		// load an empty guide
		m_pGuide = guide_create();
		// return success
		return TRUE;
	}
	// ======================================================================

	// load the file as a new document
	unsigned os_errcode;
	uint32 gde_format = 0;
	struct guide_t *pNewDocument = guide_load(lpszPathName, &os_errcode, &gde_format);

	// could we open it?
	if (pNewDocument == NULL)
	{
		// could not open file: display an error message

		// get only the filename out
		CString fullName(lpszPathName);
		fullName.Replace(_T('/'), _T('\\'));
		int slashPos = fullName.ReverseFind(_T('\\'));
		LPCTSTR fileName = static_cast<LPCTSTR>(fullName) + slashPos + 1;
		// note: if \ is not present, slashPos == -1.

		// we form the error message in 'msg'
		CString msg;
		if (os_errcode == 0)
		{
			msg.FormatMessage(IDS_NOGDE, fileName);
		}
		else
		{
			if (os_errcode == 32)
			{
				// show a specific message for file locked
				msg.FormatMessage(IDS_FILELOCKED, fileName);
			}
			else
			{
				// show a generic message otherwise
				msg.FormatMessage(IDS_FILEOPENERR, fileName, getMessage(os_errcode));
			}
		}

		// display the message
		AfxMessageBox(msg, MB_OK|MB_ICONSTOP);

		// return error
		return FALSE;
	}

	// now we can delete the old document
	DeleteContents();
	ASSERT(m_pGuide == NULL);

	// use the new one hereafter
	m_pGuide = pNewDocument;

	// opened: "lock" the .gde file
	m_Locker.lock(lpszPathName);

	// migrate old (v1.x gde file format) links to new ones
	if (gde_format == 1)
		migrateLinks(m_pGuide);

	// success!
	return TRUE;
}

// Set proper name (just filename w/o extension) in title bar
void CGuideDoc::SetPathName(LPCTSTR lpszPathName,
   BOOL bAddToMRU)
{
	CString strTitle(lpszPathName);
	CDocument::SetPathName(lpszPathName, bAddToMRU);
	strTitle.Replace(L".gde", L"");
	int slashPos = strTitle.ReverseFind(L'\\');
	if (slashPos > 0 && slashPos <= strTitle.GetLength()-1)
		strTitle.Delete(0, slashPos+1);
	SetTitle(strTitle);
}

BOOL CGuideDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// commit the currently being edited text
	::SendMessage(theApp.GetLeftWnd(), WM_GUIDEUI, GM_COMMIT_RTF_NOW, 0);

	// "unlock" the file
	CString fileName = m_Locker.getFileName();
	// if the fileName is empty, it means that we're going to save a
	// previously unsaved document (untitled).
	if (fileName.IsEmpty() == FALSE)
		// otherwise unlock it
		m_Locker.unlock();

	// save the file
	ASSERT(m_pGuide);
	if (guide_store(lpszPathName, m_pGuide) != 0)
		return FALSE;

	// after first save, use the filename and lock in future
	if (fileName.IsEmpty() == TRUE)
		fileName = lpszPathName;

	// re-"lock" the file
	m_Locker.lock(fileName);

	// set unmodified flag
	SetModifiedFlag(FALSE);

	return TRUE;
}
