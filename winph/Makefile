EXE=winph.exe
OBJ1=ieproxy.obj
OBJ2=initmgr.obj
OBJ3=conn.obj
OBJ4=winph.obj
RESOURCE=winph.res

CL_FLAG = /O2 /Oi /GL /D "_X86_" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FD /EHsc /MD /Gy /W3 /nologo /c /Zi /TP 
LINK_FLAG= /INCREMENTAL:NO /NOLOGO /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /LTCG /DYNAMICBASE /NXCOMPAT /MACHINE:X86 /ERRORREPORT:PROMPT
SYSLIB="kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib"

$(EXE): $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(RESOURCE)
    LINK $(LINK_FLAG) /out:$(EXE) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(RESOURCE) $(SYSLIB)

$(OBJ4): winph.c
    CL $(CL_FLAG) /Fo$(OBJ4) winph.c

$(OBJ3): conn.c
    CL $(CL_FLAG) /Fo$(OBJ3) conn.c

$(OBJ2): initmgr.c
    CL $(CL_FLAG) /Fo$(OBJ2) initmgr.c

$(OBJ1): ieproxy.c
    CL $(CL_FLAG) /Fo$(OBJ1) ieproxy.c

$(RESOURCE): winph.rc
	RC /nologo /Fo$(RESOURCE) winph.rc

clean:
    del /s /q /f *.obj
    del /s /q /f *.res
    del /s /q /f *.pdb
    del /s /q /f *.idb
	