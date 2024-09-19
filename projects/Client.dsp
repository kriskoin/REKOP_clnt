# Microsoft Developer Studio Project File - Name="Client" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Client - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Client.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Client.mak" CFG="Client - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Client - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Client___Win32_Debug"
# PROP BASE Intermediate_Dir "Client___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\client"
# PROP Intermediate_Dir "D:\PokerBin\work_cl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /GX /O1 /I "..\pplib" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "HORATIO" /D "LAPTO" /D "ADMINP" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib comctl32.lib winmm.lib wininet.lib version.lib ..\lib\zlib.lib ..\lib\ssleay32.lib ..\lib\libeay32.lib ..\lib\jpeg.lib /version:3.1 /subsystem:windows /pdb:none /machine:I386 /force
# SUBTRACT LINK32 /nologo /map /debug
# Begin Target

# Name "Client - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "pplib source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\pplib\anim.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\array.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\clientip.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\common.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\crc.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\fnames.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\gunzip.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\hand.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\iptools.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\jpeg.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\kprintf.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\llip.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\mem.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\parms.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\pktpool.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\poker.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\pplib.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\proc.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\qsort.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\roboplay.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\seconds.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\serverip.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\stackcr.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\tree.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\winblit.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\winsnap.cpp
# End Source File
# Begin Source File

SOURCE=..\pplib\wintools.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Client\cardroom.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\cards.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\cashier.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\chips.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\client.cpp
# End Source File
# Begin Source File

SOURCE=..\CLIENT\client.rc
# End Source File
# Begin Source File

SOURCE=..\Client\comm.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\miscdraw.cpp
# End Source File
# Begin Source File

SOURCE=..\client\news.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\prefs.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\snacks.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\splash.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\table.cpp
# End Source File
# Begin Source File

SOURCE=..\client\upgrade.cpp
# End Source File
# Begin Source File

SOURCE=..\Client\waitlist.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "pplib header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\pplib\gamedata.h
# End Source File
# Begin Source File

SOURCE=..\pplib\hand.h
# End Source File
# Begin Source File

SOURCE=..\pplib\list_pplib.h
# End Source File
# Begin Source File

SOURCE=..\pplib\llip.h
# End Source File
# Begin Source File

SOURCE=..\pplib\poker.h
# End Source File
# Begin Source File

SOURCE=..\pplib\pplib.h
# End Source File
# Begin Source File

SOURCE=..\pplib\roboplay.h
# End Source File
# Begin Source File

SOURCE=..\pplib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\pplib\zlib.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\client\car.h
# End Source File
# Begin Source File

SOURCE=..\client\client.h
# End Source File
# Begin Source File

SOURCE=..\client\pay.html
# End Source File
# Begin Source File

SOURCE=..\client\resource.h
# End Source File
# Begin Source File

SOURCE=..\client\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\client\bitmap3.bmp
# End Source File
# Begin Source File

SOURCE=..\client\data\media\bitmap4.bmp
# End Source File
# Begin Source File

SOURCE=..\CLIENT\bitmap_g.bmp
# End Source File
# Begin Source File

SOURCE=..\CLIENT\bitmap_r.bmp
# End Source File
# Begin Source File

SOURCE=..\client\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=..\client\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=..\client\CLIENT.ICO
# End Source File
# Begin Source File

SOURCE=..\client\data\media\client.ico
# End Source File
# Begin Source File

SOURCE=..\client\data\media\emedia.bmp
# End Source File
# Begin Source File

SOURCE=..\client\HANDSHAK.ICO
# End Source File
# Begin Source File

SOURCE=..\client\icon3.ico
# End Source File
# Begin Source File

SOURCE=..\client\icon4.ico
# End Source File
# Begin Source File

SOURCE=..\client\lock.bmp
# End Source File
# Begin Source File

SOURCE=..\client\logo.bmp
# End Source File
# Begin Source File

SOURCE=..\client\data\media\logo2.bmp
# End Source File
# Begin Source File

SOURCE=..\client\logo2.bmp
# End Source File
# Begin Source File

SOURCE=..\client\data\media\logo3.bmp
# End Source File
# Begin Source File

SOURCE=..\client\logo3.bmp
# End Source File
# Begin Source File

SOURCE=..\CLIENT\pdlg_fp.bmp
# End Source File
# Begin Source File

SOURCE=..\client\pdlg_mc.bmp
# End Source File
# Begin Source File

SOURCE=..\client\pdlg_vi.bmp
# End Source File
# Begin Source File

SOURCE=..\client\data\media\Splash.bmp
# End Source File
# Begin Source File

SOURCE=..\client\tab.bmp
# End Source File
# Begin Source File

SOURCE=..\client\data\media\tropicana.bmp
# End Source File
# Begin Source File

SOURCE=..\CLIENT\vip.bmp
# End Source File
# Begin Source File

SOURCE=..\client\W95MBX03.ICO
# End Source File
# End Group
# End Target
# End Project
