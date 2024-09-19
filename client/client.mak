# Microsoft Developer Studio Generated NMAKE File, Based on client.dsp
!IF "$(CFG)" == ""
CFG=client - Win32 Debug
!MESSAGE No configuration specified. Defaulting to client - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "client - Win32 Release" && "$(CFG)" != "client - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "client.mak" CFG="client - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "client - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "client - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "client - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\client.exe" "$(OUTDIR)\client.bsc"

!ELSE 

ALL : "PokerSrv - Win32 Release" "pplib - Win32 Release" "$(OUTDIR)\client.exe" "$(OUTDIR)\client.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"pplib - Win32 ReleaseCLEAN" "PokerSrv - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\cardroom.obj"
	-@erase "$(INTDIR)\cardroom.sbr"
	-@erase "$(INTDIR)\cards.obj"
	-@erase "$(INTDIR)\cards.sbr"
	-@erase "$(INTDIR)\cashier.obj"
	-@erase "$(INTDIR)\cashier.sbr"
	-@erase "$(INTDIR)\chips.obj"
	-@erase "$(INTDIR)\chips.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.pch"
	-@erase "$(INTDIR)\client.res"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\comm.obj"
	-@erase "$(INTDIR)\comm.sbr"
	-@erase "$(INTDIR)\miscdraw.obj"
	-@erase "$(INTDIR)\miscdraw.sbr"
	-@erase "$(INTDIR)\news.obj"
	-@erase "$(INTDIR)\news.sbr"
	-@erase "$(INTDIR)\prefs.obj"
	-@erase "$(INTDIR)\prefs.sbr"
	-@erase "$(INTDIR)\snacks.obj"
	-@erase "$(INTDIR)\snacks.sbr"
	-@erase "$(INTDIR)\splash.obj"
	-@erase "$(INTDIR)\splash.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\table.obj"
	-@erase "$(INTDIR)\table.sbr"
	-@erase "$(INTDIR)\upgrade.obj"
	-@erase "$(INTDIR)\upgrade.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\waitlist.obj"
	-@erase "$(INTDIR)\waitlist.sbr"
	-@erase "$(OUTDIR)\client.bsc"
	-@erase "$(OUTDIR)\client.exe"
	-@erase "$(OUTDIR)\client.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G6 /MT /W4 /GX /Zi /O1 /I "..\pplib" /I "..\openssl-0.9.4\inc32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\client.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\client.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cardroom.sbr" \
	"$(INTDIR)\cards.sbr" \
	"$(INTDIR)\cashier.sbr" \
	"$(INTDIR)\chips.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\comm.sbr" \
	"$(INTDIR)\miscdraw.sbr" \
	"$(INTDIR)\news.sbr" \
	"$(INTDIR)\prefs.sbr" \
	"$(INTDIR)\snacks.sbr" \
	"$(INTDIR)\splash.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\table.sbr" \
	"$(INTDIR)\upgrade.sbr" \
	"$(INTDIR)\waitlist.sbr"

"$(OUTDIR)\client.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=version.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib comctl32.lib zlib.lib wininet.lib jpeg.lib winmm.lib ..\openssl-0.9.4\out32dll\ssleay32.lib ..\openssl-0.9.4\out32dll\libeay32.lib /nologo /subsystem:windows /pdb:none /map:"$(INTDIR)\client.map" /machine:I386 /out:"$(OUTDIR)\client.exe" /libpath:"..\lib" /debug:none 
LINK32_OBJS= \
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
	"$(INTDIR)\client.res" \
	"..\pplib\Release\pplib.lib"

"$(OUTDIR)\client.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "client - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\client.exe" "$(OUTDIR)\client.bsc"

!ELSE 

ALL : "PokerSrv - Win32 Debug" "pplib - Win32 Debug" "$(OUTDIR)\client.exe" "$(OUTDIR)\client.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"pplib - Win32 DebugCLEAN" "PokerSrv - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\cardroom.obj"
	-@erase "$(INTDIR)\cardroom.sbr"
	-@erase "$(INTDIR)\cards.obj"
	-@erase "$(INTDIR)\cards.sbr"
	-@erase "$(INTDIR)\cashier.obj"
	-@erase "$(INTDIR)\cashier.sbr"
	-@erase "$(INTDIR)\chips.obj"
	-@erase "$(INTDIR)\chips.sbr"
	-@erase "$(INTDIR)\client.obj"
	-@erase "$(INTDIR)\client.pch"
	-@erase "$(INTDIR)\client.res"
	-@erase "$(INTDIR)\client.sbr"
	-@erase "$(INTDIR)\comm.obj"
	-@erase "$(INTDIR)\comm.sbr"
	-@erase "$(INTDIR)\miscdraw.obj"
	-@erase "$(INTDIR)\miscdraw.sbr"
	-@erase "$(INTDIR)\news.obj"
	-@erase "$(INTDIR)\news.sbr"
	-@erase "$(INTDIR)\prefs.obj"
	-@erase "$(INTDIR)\prefs.sbr"
	-@erase "$(INTDIR)\snacks.obj"
	-@erase "$(INTDIR)\snacks.sbr"
	-@erase "$(INTDIR)\splash.obj"
	-@erase "$(INTDIR)\splash.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\table.obj"
	-@erase "$(INTDIR)\table.sbr"
	-@erase "$(INTDIR)\upgrade.obj"
	-@erase "$(INTDIR)\upgrade.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\waitlist.obj"
	-@erase "$(INTDIR)\waitlist.sbr"
	-@erase "$(OUTDIR)\client.bsc"
	-@erase "$(OUTDIR)\client.exe"
	-@erase "$(OUTDIR)\client.ilk"
	-@erase "$(OUTDIR)\client.map"
	-@erase "$(OUTDIR)\client.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G6 /MT /W4 /Gm /GX /ZI /I "..\pplib" /I "..\openssl-0.9.4\inc32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\client.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\client.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cardroom.sbr" \
	"$(INTDIR)\cards.sbr" \
	"$(INTDIR)\cashier.sbr" \
	"$(INTDIR)\chips.sbr" \
	"$(INTDIR)\client.sbr" \
	"$(INTDIR)\comm.sbr" \
	"$(INTDIR)\miscdraw.sbr" \
	"$(INTDIR)\news.sbr" \
	"$(INTDIR)\prefs.sbr" \
	"$(INTDIR)\snacks.sbr" \
	"$(INTDIR)\splash.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\table.sbr" \
	"$(INTDIR)\upgrade.sbr" \
	"$(INTDIR)\waitlist.sbr"

"$(OUTDIR)\client.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=pplib.lib version.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib comctl32.lib zlib.lib wininet.lib jpeg.lib winmm.lib ..\openssl-0.9.4\out32dll\ssleay32.lib ..\openssl-0.9.4\out32dll\libeay32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\client.pdb" /map:"$(INTDIR)\client.map" /debug /machine:I386 /out:"$(OUTDIR)\client.exe" /pdbtype:sept /libpath:"..\lib" 
LINK32_OBJS= \
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
	"$(INTDIR)\client.res" \
	"..\pplib\Debug\pplib.lib"

"$(OUTDIR)\client.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("client.dep")
!INCLUDE "client.dep"
!ELSE 
!MESSAGE Warning: cannot find "client.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "client - Win32 Release" || "$(CFG)" == "client - Win32 Debug"
SOURCE=.\cardroom.cpp

"$(INTDIR)\cardroom.obj"	"$(INTDIR)\cardroom.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\cards.cpp

"$(INTDIR)\cards.obj"	"$(INTDIR)\cards.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\cashier.cpp

"$(INTDIR)\cashier.obj"	"$(INTDIR)\cashier.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\chips.cpp

"$(INTDIR)\chips.obj"	"$(INTDIR)\chips.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\client.cpp

"$(INTDIR)\client.obj"	"$(INTDIR)\client.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\client.rc

"$(INTDIR)\client.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\comm.cpp

"$(INTDIR)\comm.obj"	"$(INTDIR)\comm.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\miscdraw.cpp

"$(INTDIR)\miscdraw.obj"	"$(INTDIR)\miscdraw.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\news.cpp

"$(INTDIR)\news.obj"	"$(INTDIR)\news.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\prefs.cpp

"$(INTDIR)\prefs.obj"	"$(INTDIR)\prefs.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\snacks.cpp

"$(INTDIR)\snacks.obj"	"$(INTDIR)\snacks.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\splash.cpp

"$(INTDIR)\splash.obj"	"$(INTDIR)\splash.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "client - Win32 Release"

CPP_SWITCHES=/nologo /G6 /MT /W4 /GX /Zi /O1 /I "..\pplib" /I "..\openssl-0.9.4\inc32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\client.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\client.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "client - Win32 Debug"

CPP_SWITCHES=/nologo /G6 /MT /W4 /Gm /GX /ZI /I "..\pplib" /I "..\openssl-0.9.4\inc32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\client.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\client.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\table.cpp

"$(INTDIR)\table.obj"	"$(INTDIR)\table.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\upgrade.cpp

"$(INTDIR)\upgrade.obj"	"$(INTDIR)\upgrade.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


SOURCE=.\waitlist.cpp

"$(INTDIR)\waitlist.obj"	"$(INTDIR)\waitlist.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\client.pch"


!IF  "$(CFG)" == "client - Win32 Release"

"pplib - Win32 Release" : 
   cd "\Src\Poker\pplib"
   $(MAKE) /$(MAKEFLAGS) /F .\pplib.mak CFG="pplib - Win32 Release" 
   cd "..\client"

"pplib - Win32 ReleaseCLEAN" : 
   cd "\Src\Poker\pplib"
   $(MAKE) /$(MAKEFLAGS) /F .\pplib.mak CFG="pplib - Win32 Release" RECURSE=1 CLEAN 
   cd "..\client"

!ELSEIF  "$(CFG)" == "client - Win32 Debug"

"pplib - Win32 Debug" : 
   cd "\Src\Poker\pplib"
   $(MAKE) /$(MAKEFLAGS) /F .\pplib.mak CFG="pplib - Win32 Debug" 
   cd "..\client"

"pplib - Win32 DebugCLEAN" : 
   cd "\Src\Poker\pplib"
   $(MAKE) /$(MAKEFLAGS) /F .\pplib.mak CFG="pplib - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\client"

!ENDIF 

!IF  "$(CFG)" == "client - Win32 Release"

"PokerSrv - Win32 Release" : 
   cd "\Src\Poker\PokerSrv"
   $(MAKE) /$(MAKEFLAGS) /F .\PokerSrv.mak CFG="PokerSrv - Win32 Release" 
   cd "..\client"

"PokerSrv - Win32 ReleaseCLEAN" : 
   cd "\Src\Poker\PokerSrv"
   $(MAKE) /$(MAKEFLAGS) /F .\PokerSrv.mak CFG="PokerSrv - Win32 Release" RECURSE=1 CLEAN 
   cd "..\client"

!ELSEIF  "$(CFG)" == "client - Win32 Debug"

"PokerSrv - Win32 Debug" : 
   cd "\Src\Poker\PokerSrv"
   $(MAKE) /$(MAKEFLAGS) /F .\PokerSrv.mak CFG="PokerSrv - Win32 Debug" 
   cd "..\client"

"PokerSrv - Win32 DebugCLEAN" : 
   cd "\Src\Poker\PokerSrv"
   $(MAKE) /$(MAKEFLAGS) /F .\PokerSrv.mak CFG="PokerSrv - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\client"

!ENDIF 


!ENDIF 

