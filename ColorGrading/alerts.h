#pragma once

#include <windows.h>
#include <cstdio>

// Internals, please ignore. only here due to templates
char* _format(char* fmt, va_list args);

// String formatting utility, the templated version will assign a char* to the type and free internally allocated memory
char* format(char* fmt, ...); // char* needs to be deleted by the user!
template<typename T>
T format(char* fmt, ...)
{
	va_list args;
	__crt_va_start(args, fmt);
	char* message = _format(fmt, args);
	__crt_va_end(args);
	T result = message;
	delete message;
	return result;
}

// Message utilities
// In DEBUG all these use OutputDebugString() and if IsDebuggerPresent(): DebugBreak()
// In Release the messages suffixed 'd' will use stdout, the others will show MessageBox() instead
void info(char* fmt, ...);
void warning(char* fmt, ...);
void error(char* fmt, ...);
void fatal(char* fmt, ...); // will ExitProcess
void assert(bool expression, char* fmt, ...);
void assertFatal(bool expression, char* fmt, ...); // will ExitProcess
void infod(char* fmt, ...);
void warningd(char* fmt, ...);
void errord(char* fmt, ...);
void fatald(char* fmt, ...); // will ExitProcess
void assertd(bool expression, char* fmt, ...); // error if(!(expression))
void assertFatald(bool expression, char* fmt, ...); // error if(!(expression)), will ExitProcess

void assert(bool expression); // no message, does nothing outside of debug
void assertFatal(bool expression); // no message, will ExitProcess

// Utility to take char* from a QString, in order to toss it to format() or equivalent
#define CONVERT_QSTRING(QSTR, DST) std::string huilen = QSTR.toStdString(); const char* DST = huilen.c_str();
