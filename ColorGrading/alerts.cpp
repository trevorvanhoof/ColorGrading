#include <windows.h>
#include <cstdio>
#include <iostream>

char* _format(char* fmt, va_list args)
{
	#pragma warning(suppress: 28719)
	int size = vsnprintf(nullptr, 0, fmt, args);
	char* message = new char[size];
	vsnprintf(message, size, fmt, args);
	return message;
}

void _message(char* title, unsigned int flags, char* fmt, va_list args)
{
	char* message = _format(fmt, args);
	// TODO: throw more info, call stack with line nrs for example
	if (IsDebuggerPresent())
	{
		OutputDebugStringA(message);
		OutputDebugStringA("\r\n");
	}
	else
		MessageBoxA(0, message, title, flags);
	delete message;
}

void _messaged(char* title, char* fmt, va_list args)
{
	char* message = _format(fmt, args);
	// TODO: throw more info, call stack with line nrs for example
	if (IsDebuggerPresent())
	{
		OutputDebugStringA(message);
		OutputDebugStringA("\r\n");
	}
	else
		std::cout << title << ": " << message << std::endl;
	delete message;
}

char* format(char* fmt, ...)
{
	va_list args;
	__crt_va_start(args, fmt);
	char* result = _format(fmt, args);
	__crt_va_end(args);
	return result;
}

void info(char* fmt, ...)
{
	va_list args;
	__crt_va_start(args, fmt);
	_message("Info", MB_OK | MB_ICONINFORMATION, fmt, args);
	__crt_va_end(args);
}

void warning(char* fmt, ...)
{
	va_list args;
	__crt_va_start(args, fmt);
	_message("Warning", MB_OK | MB_ICONWARNING, fmt, args);
	__crt_va_end(args);
	if (IsDebuggerPresent())
		DebugBreak();
}

void error(char* fmt, ...)
{
	va_list args;
	__crt_va_start(args, fmt);
	_message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
	__crt_va_end(args);
	if (IsDebuggerPresent())
		DebugBreak();
}

void fatal(char* fmt, ...)
{
	va_list args;
	__crt_va_start(args, fmt);
	_message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
	__crt_va_end(args);
	if (IsDebuggerPresent())
		DebugBreak();
	else
		ExitProcess(0);
}

void assert(bool expression)
{
	if (expression)
		return;
	if (IsDebuggerPresent())
		DebugBreak();
}

void assert(bool expression, char* fmt, ...)
{
	if (expression)
		return;
	va_list args;
	__crt_va_start(args, fmt);
	_message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
	__crt_va_end(args);
	if (IsDebuggerPresent())
		DebugBreak();
}

void assertFatal(bool expression)
{
	if (expression)
		return;
	if (IsDebuggerPresent())
		DebugBreak();
	else
		ExitProcess(0);
}

void assertFatal(bool expression, char* fmt, ...)
{
	if (expression)
		return;
	va_list args;
	__crt_va_start(args, fmt);
	_message("Error", MB_OK | MB_ICONEXCLAMATION, fmt, args);
	__crt_va_end(args);
	if (IsDebuggerPresent())
		DebugBreak();
	else
		ExitProcess(0);
}

void infod(char* fmt, ...)
{
	va_list args;
	__crt_va_start(args, fmt);
	_messaged("Info", fmt, args);
	__crt_va_end(args);
}

void warningd(char* fmt, ...)
{
	va_list args;
	__crt_va_start(args, fmt);
	_messaged("Warning", fmt, args);
	__crt_va_end(args);
	if (IsDebuggerPresent())
		DebugBreak();
}

void errord(char* fmt, ...)
{
	va_list args;
	__crt_va_start(args, fmt);
	_messaged("Error", fmt, args);
	__crt_va_end(args);
	if (IsDebuggerPresent())
		DebugBreak();
}

void fatald(char* fmt, ...)
{
	va_list args;
	__crt_va_start(args, fmt);
	_messaged("Error", fmt, args);
	__crt_va_end(args);
	if (IsDebuggerPresent())
		DebugBreak();
	else
		ExitProcess(0);
}

void assertd(bool expression, char* fmt, ...)
{
	if (expression)
		return;
	va_list args;
	__crt_va_start(args, fmt);
	_messaged("Error", fmt, args);
	__crt_va_end(args);
	if (IsDebuggerPresent())
		DebugBreak();
}

void assertFatald(bool expression, char* fmt, ...)
{
	if (expression)
		return;
	va_list args;
	__crt_va_start(args, fmt);
	_messaged("Error", fmt, args);
	__crt_va_end(args);
	if (IsDebuggerPresent())
		DebugBreak();
	else
		ExitProcess(0);
}