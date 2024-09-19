# Microsoft Developer Studio Generated NMAKE File, Based on Client.dsp
!IF "$(CFG)" == ""
CFG=Client - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Client - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Client - Win32 Release" && "$(CFG)" != "Client - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Client.mak" CFG="Client - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Client - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Client - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Client - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\Client.exe"


CLEAN :
	-@erase "$(INTDIR)\anim.obj"
	-@erase "$(INTDIR)\array.obj"
	-@erase "$(INTDIR)\cardroom.obj"
	-@erase "$(INTDIR)\cards.obj"
	-@erase "$(INTDIR)\cashier.obj"
	-@erase "$(INTDIR)\chips.obj"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.res"
	-@erase "$(INTDIR)\clientip.obj"
	-@erase "$(INTDIR)\comm.obj"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\fnames.obj"
	-@erase "$(INTDIR)\gunzip.obj"
	-@erase "$(INTDIR)\hand.obj"
	-@erase "$(INTDIR)\iptools.obj"
	-@erase "$(INTDIR)\jpeg.obj"
	-@erase "$(INTDIR)\kprintf.obj"
	-@erase "$(INTDIR)\llip.obj"
	-@erase "$(INTDIR)\mem.obj"
	-@erase "$(INTDIR)\miscdraw.obj"
	-@erase "$(INTDIR)\news.obj"
	-@erase "$(INTDIR)\parms.obj"
	-@erase "$(INTDIR)\pktpool.obj"
	-@erase "$(INTDIR)\poker.obj"
	-@erase "$(INTDIR)\pplib.obj"
	-@erase "$(INTDIR)\prefs.obj"
	-@erase "$(INTDIR)\proc.obj"
	-@erase "$(INTDIR)\qsort.obj"
	-@erase "$(INTDIR)\roboplay.obj"
	-@erase "$(INTDIR)\seconds.obj"
	-@erase "$(INTDIR)\serverip.obj"
	-@erase "$(INTDIR)\snacks.obj"
	-@erase "$(INTDIR)\splash.obj"
	-@erase "$(INTDIR)\stackcr.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\table.obj"
	-@erase "$(INTDIR)\tree.obj"
	-@erase "$(INTDIR)\upgrade.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\waitlist.obj"
	-@erase "$(INTDIR)\winblit.obj"
	-@erase "$(INTDIR)\winsnap.obj"
	-@erase "$(INTDIR)\wintools.obj"
	-@erase "..\Client.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\Client.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\client.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Client.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib winmm.lib wininet.lib version.lib libcmt.lib /nologo /subsystem:windows /incremental:no /pdb:"c:\poker\client/Client.pdb" /machine:I386 /nodefaultlib:"libc" /out:"c:\poker\client/Client.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\anim.obj" \
	"$(INTDIR)\array.obj" \
	"$(INTDIR)\clientip.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\fnames.obj" \
	"$(INTDIR)\gunzip.obj" \
	"$(INTDIR)\hand.obj" \
	"$(INTDIR)\iptools.obj" \
	"$(INTDIR)\jpeg.obj" \
	"$(INTDIR)\kprintf.obj" \
	"$(INTDIR)\llip.obj" \
	"$(INTDIR)\mem.obj" \
	"$(INTDIR)\parms.obj" \
	"$(INTDIR)\pktpool.obj" \
	"$(INTDIR)\poker.obj" \
	"$(INTDIR)\pplib.obj" \
	"$(INTDIR)\proc.obj" \
	"$(INTDIR)\qsort.obj" \
	"$(INTDIR)\roboplay.obj" \
	"$(INTDIR)\seconds.obj" \
	"$(INTDIR)\serverip.obj" \
	"$(INTDIR)\stackcr.obj" \
	"$(INTDIR)\tree.obj" \
	"$(INTDIR)\winblit.obj" \
	"$(INTDIR)\winsnap.obj" \
	"$(INTDIR)\wintools.obj" \
	"$(INTDIR)\cardroom.obj" \
	"$(INTDIR)\cards.obj" \
	"$(INTDIR)\cashier.obj" \
	"$(INTDIR)\chips.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\comm.obj" \
	"$(INTDIR)\miscdraw.obj" \
	"$(INTDIR)\news.obj" \
	"$(INTDIR)\prefs.obj" \
	"$(INTDIR)\snacks.obj" \
	"$(INTDIR)\splash.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\table.obj" \
	"$(INTDIR)\upgrade.obj" \
	"$(INTDIR)\waitlist.obj" \
	"$(INTDIR)\client.res"

"..\Client.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Client - Win32 Debug"

OUTDIR=.\..\client
INTDIR=d:\bin\work_cl
# Begin Custom Macros
OutDir=.\..\client
# End Custom Macros

ALL : "$(OUTDIR)\Client.exe"


CLEAN :
	-@erase "$(INTDIR)\anim.obj"
	-@erase "$(INTDIR)\array.obj"
	-@erase "$(INTDIR)\cardroom.obj"
	-@erase "$(INTDIR)\cards.obj"
	-@erase "$(INTDIR)\cashier.obj"
	-@erase "$(INTDIR)\chips.obj"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.res"
	-@erase "$(INTDIR)\clientip.obj"
	-@erase "$(INTDIR)\comm.obj"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\fnames.obj"
	-@erase "$(INTDIR)\gunzip.obj"
	-@erase "$(INTDIR)\hand.obj"
	-@erase "$(INTDIR)\iptools.obj"
	-@erase "$(INTDIR)\jpeg.obj"
	-@erase "$(INTDIR)\kprintf.obj"
	-@erase "$(INTDIR)\llip.obj"
	-@erase "$(INTDIR)\mem.obj"
	-@erase "$(INTDIR)\miscdraw.obj"
	-@erase "$(INTDIR)\news.obj"
	-@erase "$(INTDIR)\parms.obj"
	-@erase "$(INTDIR)\pktpool.obj"
	-@erase "$(INTDIR)\poker.obj"
	-@erase "$(INTDIR)\pplib.obj"
	-@erase "$(INTDIR)\prefs.obj"
	-@erase "$(INTDIR)\proc.obj"
	-@erase "$(INTDIR)\qsort.obj"
	-@erase "$(INTDIR)\roboplay.obj"
	-@erase "$(INTDIR)\seconds.obj"
	-@erase "$(INTDIR)\serverip.obj"
	-@erase "$(INTDIR)\snacks.obj"
	-@erase "$(INTDIR)\splash.obj"
	-@erase "$(INTDIR)\stackcr.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\table.obj"
	-@erase "$(INTDIR)\tree.obj"
	-@erase "$(INTDIR)\upgrade.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\waitlist.obj"
	-@erase "$(INTDIR)\winblit.obj"
	-@erase "$(INTDIR)\winsnap.obj"
	-@erase "$(INTDIR)\wintools.obj"
	-@erase "$(OUTDIR)\Client.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W4 /GX /O1 /I "..\pplib" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "HORATIO" /D "LAPTO" /D "ADMINP" /Fp"$(INTDIR)\Client.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\client.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Client.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib winmm.lib wininet.lib version.lib ..\lib\zlib.lib ..\lib\ssleay32.lib ..\lib\libeay32.lib ..\lib\jpeg.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\Client.pdb" /machine:I386 /out:"$(OUTDIR)\Client.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\anim.obj" \
	"$(INTDIR)\array.obj" \
	"$(INTDIR)\clientip.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\fnames.obj" \
	"$(INTDIR)\gunzip.obj" \
	"$(INTDIR)\hand.obj" \
	"$(INTDIR)\iptools.obj" \
	"$(INTDIR)\jpeg.obj" \
	"$(INTDIR)\kprintf.obj" \
	"$(INTDIR)\llip.obj" \
	"$(INTDIR)\mem.obj" \
	"$(INTDIR)\parms.obj" \
	"$(INTDIR)\pktpool.obj" \
	"$(INTDIR)\poker.obj" \
	"$(INTDIR)\pplib.obj" \
	"$(INTDIR)\proc.obj" \
	"$(INTDIR)\qsort.obj" \
	"$(INTDIR)\roboplay.obj" \
	"$(INTDIR)\seconds.obj" \
	"$(INTDIR)\serverip.obj" \
	"$(INTDIR)\stackcr.obj" \
	"$(INTDIR)\tree.obj" \
	"$(INTDIR)\winblit.obj" \
	"$(INTDIR)\winsnap.obj" \
	"$(INTDIR)\wintools.obj" \
	"$(INTDIR)\cardroom.obj" \
	"$(INTDIR)\cards.obj" \
	"$(INTDIR)\cashier.obj" \
	"$(INTDIR)\chips.obj" \
	"$(INTDIR)\client.obj" \
	"$(INTDIR)\comm.obj" \
	"$(INTDIR)\miscdraw.obj" \
	"$(INTDIR)\news.obj" \
	"$(INTDIR)\prefs.obj" \
	"$(INTDIR)\snacks.obj" \
	"$(INTDIR)\splash.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\table.obj" \
	"$(INTDIR)\upgrade.obj" \
	"$(INTDIR)\waitlist.obj" \
	"$(INTDIR)\client.res"

"$(OUTDIR)\Client.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Client.dep")
!INCLUDE "Client.dep"
!ELSE 
!MESSAGE Warning: cannot find "Client.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Client - Win32 Release" || "$(CFG)" == "Client - Win32 Debug"
SOURCE=..\pplib\anim.cpp

"$(INTDIR)\anim.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\array.cpp

"$(INTDIR)\array.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\clientip.cpp

"$(INTDIR)\clientip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\common.cpp

"$(INTDIR)\common.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\crc.cpp

"$(INTDIR)\crc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\fnames.cpp

"$(INTDIR)\fnames.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\gunzip.cpp

"$(INTDIR)\gunzip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\hand.cpp

"$(INTDIR)\hand.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\iptools.cpp

"$(INTDIR)\iptools.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\jpeg.cpp

"$(INTDIR)\jpeg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\kprintf.cpp

"$(INTDIR)\kprintf.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\llip.cpp

"$(INTDIR)\llip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\mem.cpp

"$(INTDIR)\mem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\parms.cpp

"$(INTDIR)\parms.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\pktpool.cpp

"$(INTDIR)\pktpool.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\poker.cpp

"$(INTDIR)\poker.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\pplib.cpp

"$(INTDIR)\pplib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\proc.cpp

"$(INTDIR)\proc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\qsort.cpp

"$(INTDIR)\qsort.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\roboplay.cpp

"$(INTDIR)\roboplay.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\seconds.cpp

"$(INTDIR)\seconds.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\serverip.cpp

"$(INTDIR)\serverip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\stackcr.cpp

"$(INTDIR)\stackcr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\tree.cpp

"$(INTDIR)\tree.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\winblit.cpp

"$(INTDIR)\winblit.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\winsnap.cpp

"$(INTDIR)\winsnap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\pplib\wintools.cpp

"$(INTDIR)\wintools.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\cardroom.cpp

"$(INTDIR)\cardroom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\cards.cpp

"$(INTDIR)\cards.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\cashier.cpp

"$(INTDIR)\cashier.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\chips.cpp

"$(INTDIR)\chips.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\client.cpp

"$(INTDIR)\client.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\CLIENT\client.rc

!IF  "$(CFG)" == "Client - Win32 Release"


"$(INTDIR)\client.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\client.res" /i "\poker\client\CLIENT" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "Client - Win32 Debug"


"$(INTDIR)\client.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\client.res" /i "\poker\client\CLIENT" /d "_DEBUG" $(SOURCE)


!ENDIF 

SOURCE=..\Client\comm.cpp

"$(INTDIR)\comm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\miscdraw.cpp

"$(INTDIR)\miscdraw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\client\news.cpp

"$(INTDIR)\news.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\prefs.cpp

"$(INTDIR)\prefs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\snacks.cpp

"$(INTDIR)\snacks.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\splash.cpp

"$(INTDIR)\splash.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\StdAfx.cpp

"$(INTDIR)\StdAfx.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\table.cpp

"$(INTDIR)\table.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\client\upgrade.cpp

"$(INTDIR)\upgrade.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\Client\waitlist.cpp

"$(INTDIR)\waitlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

