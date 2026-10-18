#ifdef GLADE_WINDOWS

#include <cstring>

#include <stdlib.h>
#include <windows.h>
#include <dbghelp.h>
#include <glade/debug/log.h>
#include <glade/debug/crashHandler.h>

// StackWalk64() may modify context record passed to it, so we copy it
static void log_stack_frames(CONTEXT context_record) {
  STACKFRAME64 stack_frame;
  ::memset(&stack_frame, 0, sizeof(stack_frame));
#if _WIN64
  int machine_type = IMAGE_FILE_MACHINE_AMD64;
  stack_frame.AddrPC.Offset = context_record.Rip;
  stack_frame.AddrFrame.Offset = context_record.Rbp;
  stack_frame.AddrStack.Offset = context_record.Rsp;
#elif _WIN32
  int machine_type = IMAGE_FILE_MACHINE_I386;
  stack_frame.AddrPC.Offset = context_record.Eip;
  stack_frame.AddrFrame.Offset = context_record.Ebp;
  stack_frame.AddrStack.Offset = context_record.Esp;
#else
#warning "Stack walking not implemented on this platform"
#endif // _WIN64
  stack_frame.AddrPC.Mode = AddrModeFlat;
  stack_frame.AddrFrame.Mode = AddrModeFlat;
  stack_frame.AddrStack.Mode = AddrModeFlat;

  HANDLE process = ::GetCurrentProcess();

  ::SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);

  if (!::SymInitialize(process, NULL, TRUE)) {
    ::log("SymInitialize failed : %d\n", ::GetLastError());
  }

  SYMBOL_INFO *symbol = (SYMBOL_INFO *)::calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
  symbol->MaxNameLen = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

  while (::StackWalk64(machine_type, process, ::GetCurrentThread(), &stack_frame, &context_record,
                       NULL, &::SymFunctionTableAccess64, &::SymGetModuleBase64, NULL)) {
    DWORD64 displacement = 0;

    if (::SymFromAddr(process, (DWORD64)stack_frame.AddrPC.Offset, &displacement, symbol)) {
      IMAGEHLP_MODULE64 module_info = {0};
      module_info.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
      ::logn("*** ");

      if (::SymGetModuleInfo64(process, symbol->ModBase, &module_info)) {
        ::logn("%s: ", module_info.ModuleName);
      }

      ::logn("%s - 0x%08X", symbol->Name, stack_frame.AddrPC.Offset);

      IMAGEHLP_LINE64 line = {0};
      line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
      DWORD line_displacement = 0;

      if (::SymGetLineFromAddr64(process, stack_frame.AddrPC.Offset, &line_displacement, &line)) {
        ::logn(" (%s:%d)", line.FileName, line.LineNumber);
      }

      ::log("");
    }
  }

  ::SymCleanup(process);
}

LONG WINAPI win32_crash_handler(struct _EXCEPTION_POINTERS *crash_info) {
  char *fault_type = "";

  switch (crash_info->ExceptionRecord->ExceptionCode) {
  case EXCEPTION_ACCESS_VIOLATION:
    fault_type = "ACCESS VIOLATION";
    break;
  case EXCEPTION_DATATYPE_MISALIGNMENT:
    fault_type = "DATATYPE MISALIGNMENT";
    break;
  case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    fault_type = "FLT DIVIDE BY ZERO";
    break;
  default:
    fault_type = "(unknown)";
    break;
  }

  int error_code = crash_info->ExceptionRecord->ExceptionCode;
  PVOID code_address = crash_info->ExceptionRecord->ExceptionAddress;

  ::log("****************************************************");
  ::log("*** A Program Fault occurred:");
  ::log("*** Error code %08X: %s", error_code, fault_type);
  ::log("****************************************************");
  ::log("***   Address: %08X", code_address);
  ::log("***     Flags: %08X", crash_info->ExceptionRecord->ExceptionFlags);
  ::log_stack_frames(*crash_info->ContextRecord);
  ::log("****************************************************");

  return EXCEPTION_CONTINUE_SEARCH;
}

void install_crash_handler() {
  ::SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)::win32_crash_handler);
}
#endif // GLADE_WINDOWS
