#include "../os.h"
#include "../memory.h"
#include "../filesystem.h"

using HGLOBAL = HANDLE;

#define INFINITE            0xFFFFFFFF
#define GMEM_MOVEABLE 0x02
#define CF_TEXT 1

extern "C" {
	DLL_IMPORT UINT WINAPI GetCaretBlinkTime(VOID);
	DLL_IMPORT HGLOBAL WINAPI GlobalAlloc(UINT, SIZE_T);
	DLL_IMPORT LPVOID WINAPI GlobalLock(HGLOBAL);
	DLL_IMPORT BOOL WINAPI GlobalUnlock(HGLOBAL);
	DLL_IMPORT HGLOBAL WINAPI GlobalFree(HGLOBAL);

	DLL_IMPORT BOOL WINAPI OpenClipboard(HWND);
	DLL_IMPORT BOOL WINAPI EmptyClipboard(VOID);
	DLL_IMPORT HANDLE WINAPI SetClipboardData(UINT, HANDLE);
	DLL_IMPORT BOOL WINAPI CloseClipboard(VOID);
	DLL_IMPORT HANDLE WINAPI GetClipboardData(UINT);

	DLL_IMPORT void* WINAPI GetProcAddress(HMODULE, LPCSTR);
	DLL_IMPORT BOOL WINAPI FreeLibrary(HMODULE);
	DLL_IMPORT HMODULE WINAPI LoadLibraryA(LPCSTR);
}

bool ch::get_caret_blink_time(u32* out_ms) {
	const u32 blink_time = GetCaretBlinkTime();
	if (blink_time == INFINITE) return false;

	*out_ms = blink_time;
	return true;
}

bool ch::copy_to_clipboard(ch::OS_Window_Handle window_handle, const void* buffer, usize size) {
	HGLOBAL g_mem = GlobalAlloc(GMEM_MOVEABLE, size);
	ch::mem_copy(GlobalLock(g_mem), buffer, size);
	GlobalUnlock(g_mem);
	if (!OpenClipboard((HWND)window_handle)) {
		GlobalFree(g_mem);
		return false;
	}
	EmptyClipboard();
	SetClipboardData(CF_TEXT, g_mem);
	CloseClipboard();
	return true;
}

bool ch::copy_from_clipboard(ch::OS_Window_Handle window_handle, ch::String* out_str) {
	if (!OpenClipboard((HWND)window_handle)) {
		return false;
	}

	defer(CloseClipboard());

	HANDLE c_data = GetClipboardData(CF_TEXT);
	if (!c_data) return false;

	char* out_data = (char*)GlobalLock(c_data);
	if (!out_data) return false;

	const usize str_size = ch::strlen(out_data);
	out_str->reserve(str_size);

	ch::mem_copy(out_str->data, out_data, str_size * sizeof(char));
	out_str->data[str_size] = 0;
	out_str->count = str_size;

	GlobalUnlock(c_data);

	return true;
}


void* ch::Library::get_function(const char* function_name) {
	return GetProcAddress((HMODULE)handle, function_name);
}

void ch::Library::free() {
	if (handle) {
		FreeLibrary((HMODULE)handle);
	}
}

bool ch::load_library(const ch::Path& path, ch::Library* lib) {
	ch::OS_Library_Handle handle = LoadLibraryA(path);
	if (handle == INVALID_HANDLE_VALUE) return false;
	lib->handle = handle;
	return true;
}