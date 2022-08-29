@echo off

if not exist ..\build mkdir ..\build

set TARGET=app
set CFLAGS=/nologo /FC /Od /Zi /Wall /WX /EHsc /wd4668 /wd4100 /wd4062 /wd4820 /wd5045 /wd4324 /wd4711 /wd4710 /wd5220 /wd4191 /wd4255 /wd4201 /wd5039 /wd4514 /wd4587 /wd4365 /wd5219
set SRCS=*.cpp ..\thirdparty\glad\src\glad.c ..\thirdparty\cgltf\cgltf.c ..\thirdparty\stb\stb_image.cpp
set LFLAGS=/incremental:no
set LIBS=user32.lib gdi32.lib Winmm.lib opengl32.lib Kernel32.lib
set OUT_DIR=/Fo..\build\ /Fe..\build\%TARGET% /Fm..\build\ /Fd..\build\
set INC_DIR=/I.\ /I..\thirdparty\stb /I..\thirdparty\cgltf /I..\thirdparty\glad\include 
set LIB_DIR=/LIBPATH:.\
set DEFINES=/D_APP_DEBUG
cl %CFLAGS% %INC_DIR% %SRCS% %OUT_DIR%  %DEFINES% /link %LFLAGS% %LIB_DIR% %LIBS%
