Name "VideoLAN Movie Creator"

OutFile "@NSIS_OUTPUT_FILE@"

InstallDir "$PROGRAMFILES\@PROJECT_NAME_SHORT@"
InstallDirRegKey HKLM "Software\@PROJECT_NAME_SHORT@" "Install_Dir"

RequestExecutionLevel admin

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "@PROJECT_NAME_SHORT@ (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "libvlc.dll"
  File "libvlccore.dll"
  File "mingwm10.dll"
  File "libgcc_s_dw2-1.dll"
  File "QtCore4.dll"
  File "QtGui4.dll"
  File "QtSvg4.dll"
  File "QtXml4.dll"
  File "vlmc.exe"
  File "@CMAKE_SOURCE_DIR@/share/vlmc.ico"
  File /r "plugins"
  File /r "effects"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "Software\@PROJECT_NAME_SHORT@" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\@PROJECT_NAME_SHORT@" "DisplayName" "@PROJECT_NAME_LONG@"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\@PROJECT_NAME_SHORT@" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\@PROJECT_NAME_SHORT@" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\@PROJECT_NAME_SHORT@" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\@PROJECT_NAME_LONG@"
  CreateShortCut "$SMPROGRAMS\@PROJECT_NAME_LONG@\@PROJECT_NAME_SHORT@.lnk" "$INSTDIR\vlmc.exe" "" "$INSTDIR\vlmc.ico" 0
  CreateShortCut "$SMPROGRAMS\@PROJECT_NAME_LONG@\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
SectionEnd

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\@PROJECT_NAME_SHORT@"
  DeleteRegKey HKLM "Software\@PROJECT_NAME_SHORT@"

  ; Remove files and uninstaller
  Delete "$INSTDIR\vlmc.exe"
  Delete "$INSTDIR\vlmc.ico"
  Delete "$INSTDIR\uninstall.exe"
  Delete "$INSTDIR\libvlc.dll"
  Delete "$INSTDIR\libvlccore.dll"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\QtCore4.dll"
  Delete "$INSTDIR\QtGui4.dll"
  Delete "$INSTDIR\QtSvg4.dll"
  Delete "$INSTDIR\QtXml4.dll"
  Delete "$INSTDIR\plugins\*.*"
  Delete "$INSTDIR\effects\*.*"
  RMDir "$INSTDIR\plugins"
  RMDir "$INSTDIR\effects"
  

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\@PROJECT_NAME_LONG@\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\@PROJECT_NAME_LONG@"
  RMDir "$INSTDIR"

SectionEnd
