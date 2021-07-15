#include "core/logger.h"

#include <memory>
#include <varargs.h>
#include <stdarg.h>

// TODO: Move to platform layer.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

bool initialize_logging()
{
	// TODO: Create a log file to output to.
	return true;
}

void shutdown_logging()
{
	// TODO: Cleanup logging/write queued entries.
}

void log_output(LogLevel level, const char* message, ...)
{
	const char* level_strings[6] = { "[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ","[TRACE]: " };
	bool is_error = level < LogLevel::LOG_LEVEL_WARN;

	const s32 msg_length = 32000;
	char out_message[msg_length];
	memset(out_message, 0, sizeof(out_message));

	va_list arg_ptr;
	va_start(arg_ptr, message);
	vsnprintf(out_message, msg_length, message, arg_ptr);
	va_end(arg_ptr);

	char out_message2[msg_length];
	sprintf(out_message2, "%s%s\n", level_strings[(u8)level], out_message);

	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	static u8 levels[6] = { 64, 4, 6, 2, 1, 8 };
	SetConsoleTextAttribute(console_handle, levels[(u8)level]);
	OutputDebugStringA(out_message2);
	u64 length = strlen(out_message2);
	LPDWORD number_written = 0;
	WriteConsoleA(console_handle, out_message2, (DWORD)length, number_written, 0);
}