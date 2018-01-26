;
; NSIS script for creating winguide installer
;

; ---------------------------------------------------
; Definitions
;

!define VERSION		'2.0'
!define VER_MAJOR	'2'
!define VER_MINOR	'0'
!define VER_FIX		'0'

!define SHCNE_ASSOCCHANGED 0x8000000
!define SHCNF_IDLIST 0

; ---------------------------------------------------
;
;

; general params
SetCompressor /SOLID lzma
XPStyle on
OutFile winguide-${VERSION}-win32.exe

; add version info to installer
VIAddVersionKey "ProductName" "The Guide"
VIAddVersionKey "CompanyName" "Mahadevan R"
VIAddVersionKey "LegalCopyright" "(c) Mahadevan R 2005-08. All rights reserved."
VIAddVersionKey "FileDescription" "The Guide"
VIAddVersionKey "FileVersion" "${VER_MAJOR}.${VER_MINOR}.${VER_FIX}.0"
VIProductVersion "${VER_MAJOR}.${VER_MINOR}.${VER_FIX}.0"

; default installation dir
InstallDir "$PROGRAMFILES\The Guide"

; ---------------------------------------------------
; Interface settings
;

!include "MUI.nsh"

Name "The Guide"
Caption "The Guide ${VERSION} Setup"

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
!define MUI_ABORTWARNING
;!define MUI_ICON   ..\guide\res\Guide.ico
;!define MUI_UNICON ${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico

!define MUI_WELCOMEPAGE_TITLE "Welcome to The Guide ${VERSION} Setup Wizard"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of The Guide ${VERSION}. The Guide is a tree-based information management tool, that lets you organize information and ideas as nodes in a tree.\r\n\r\n$_CLICK"

!define MUI_FINISHPAGE_LINK "Click here to visit The Guide's homepage."
!define MUI_FINISHPAGE_LINK_LOCATION "http://theguide.sourceforge.net/"
!define MUI_FINISHPAGE_RUN "$INSTDIR\Guide.exe"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

; ---------------------------------------------------
; Installer
;

Section "The Guide"

	;-------------------------------------------------------------
	DetailPrint "Copying Files..."
	;-------------------------------------------------------------
	SetOutPath $INSTDIR
	SetOverwrite on
	File ..\libguide\Release\libguide.dll
	File ..\guide\Release\Guide.exe
	File ..\gdeutil\Release\gdeutil.exe
	File ..\README
	File ..\LICENSE
	File ..\CHANGES
	File ..\NOTICE
	File ..\guide\Guide.exe.manifest
	File ..\guide\guide.ini
	File msvcr71.dll
	File mfc71u.dll
	File msftedit.dll

	;-------------------------------------------------------------
	DetailPrint "Creating Shortcuts..."
	;-------------------------------------------------------------
	CreateDirectory "$SMPROGRAMS\The Guide"
	CreateShortcut	"$SMPROGRAMS\The Guide\The Guide.lnk"	$INSTDIR\Guide.exe
	CreateShortcut	"$SMPROGRAMS\The Guide\CHANGES.lnk"		%SystemRoot%\system32\notepad.exe CHANGES
;	CreateShortcut	"$SMPROGRAMS\The Guide\LICENSE.lnk"		%SystemRoot%\system32\notepad.exe LICENSE
;	CreateShortcut	"$SMPROGRAMS\The Guide\README.lnk"		%SystemRoot%\system32\notepad.exe README
	CreateShortcut	"$SMPROGRAMS\The Guide\Uninstall The Guide.lnk"	$INSTDIR\uninstall.exe
	CreateShortCut	"$DESKTOP\The Guide.lnk" 				$INSTDIR\Guide.exe

	;-------------------------------------------------------------
	DetailPrint "Associating Files..."
	;-------------------------------------------------------------
	WriteRegStr HKCR ".gde" "" "Guide.Document"
	WriteRegStr HKCR ".gde\ShellNew" "NullFile" ""
	WriteRegStr HKCR "Guide.Document" "" "Guide"
	WriteRegStr HKCR "Guide.Document\DefaultIcon" "" "$INSTDIR\Guide.exe,1"
	WriteRegStr HKCR "Guide.Document\shell" "" ""
	WriteRegStr HKCR "Guide.Document\shell\open" "" ""
	WriteRegStr HKCR "Guide.Document\shell\open\command" "" '$INSTDIR\Guide.exe "%1"'
	WriteRegStr HKCR "Guide.Document\shell\print" "" ""
	WriteRegStr HKCR "Guide.Document\shell\print\command" "" '$INSTDIR\Guide.exe /p "%1"'
	System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, i 0, i 0)'

	;-------------------------------------------------------------
	; write version info in registry
	;-------------------------------------------------------------
	WriteRegStr HKLM "Software\The Guide" "" ""
	WriteRegStr HKLM "Software\The Guide" "Version" "${VER_MAJOR}.${VER_MINOR}.${VER_FIX}"
	WriteRegStr HKLM "Software\The Guide" "InstallDir" "$INSTDIR"

	;-------------------------------------------------------------
	DetailPrint "Writing Uninstaller..."
	;-------------------------------------------------------------
	WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide" "UninstallString" '"$INSTDIR\uninstall.exe"'
	WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide" "DisplayName" "The Guide"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide" "DisplayIcon" "$INSTDIR\The Guide.exe,0"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide" "DisplayVersion" "${VERSION}"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide" "VersionMajor" "${VER_MAJOR}"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide" "VersionMinor" "${VER_MINOR}.${VER_FIX}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide" "URLInfoAbout" "http://theguide.sourceforge.net/"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide" "HelpLink" "http://theguide.sourceforge.net/"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide" "NoModify" "1"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide" "NoRepair" "1"
	WriteUninstaller $INSTDIR\uninstall.exe

SectionEnd

; ---------------------------------------------------
; Uninstaller
;

Section Uninstall

	DetailPrint "Uninstalling The Guide..."

	;-------------------------------------------------------------
	DetailPrint "Deleting Registry Keys..."
	;-------------------------------------------------------------
	DeleteRegKey HKCR ".gde"
	DeleteRegKey HKCR "Guide.Document"
	System::Call 'Shell32::SHChangeNotify(i ${SHCNE_ASSOCCHANGED}, i ${SHCNF_IDLIST}, i 0, i 0)'
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\The Guide"
	DeleteRegKey HKLM "Software\The Guide"

	;-------------------------------------------------------------
	DetailPrint "Deleting Files..."
	;-------------------------------------------------------------
	Delete "$SMPROGRAMS\The Guide\The Guide.lnk"
	Delete "$SMPROGRAMS\The Guide\Uninstall The Guide.lnk"
	RMDir /r "$SMPROGRAMS\The Guide"
	Delete "$DESKTOP\The Guide.lnk"
	Delete $INSTDIR\libguide.dll
	Delete $INSTDIR\Guide.exe
	Delete $INSTDIR\gdeutil.exe
	Delete $INSTDIR\guide.ini
	Delete $INSTDIR\README
	Delete $INSTDIR\CHANGES
	Delete $INSTDIR\LICENSE
	Delete $INSTDIR\NOTICE
	Delete $INSTDIR\Guide.exe.manifest
	Delete $INSTDIR\msvcr71.dll
	Delete $INSTDIR\mfc71u.dll
	Delete $INSTDIR\msftedit.dll
	RMDir /r $INSTDIR

SectionEnd

