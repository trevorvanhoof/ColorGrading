#pragma once

#include <windows.h>
#include <QtOpenGL>
#include <QOpenGLFunctions_4_5_Compatibility>
#pragma comment(lib, "opengl32.lib")

// Qt is a bit weird in that it needs an instance but it beats manual wglGetProcAddress
#ifndef IMPL_GL // currently defined by bufferformats.cpp to implement it somewhere
extern 
#endif
QOpenGLFunctions_4_5_Compatibility gl;
