#ifdef GLADE_WINDOWS

#include <stdlib.h>
#include <windows.h>
#include <dbghelp.h>
#include <glade/debug/log.h>
#include <glade/debug/crashHandler.h>

// StackWalk64() may modify context record passed to it, so we copy it
static void logStackFrames(CONTEXT context_record)
{
  STACKFRAME64 stackFrame;
  memset(&stackFrame, 0, sizeof(stackFrame));
#if _WIN64
  int machineType = IMAGE_FILE_MACHINE_AMD64;
  stackFrame.AddrPC.Offset = context_record.Rip;
  stackFrame.AddrFrame.Offset = context_record.Rbp;
  stackFrame.AddrStack.Offset = context_record.Rsp;
#elif _WIN32
  int machineType = IMAGE_FILE_MACHINE_I386;
  stackFrame.AddrPC.Offset = context_record.Eip;
  stackFrame.AddrFrame.Offset = context_record.Ebp;
  stackFrame.AddrStack.Offset = context_record.Esp;
#else
  #warning "Stack walking not implemented on this platform"
#endif // _WIN64
  stackFrame.AddrPC.Mode = AddrModeFlat;
  stackFrame.AddrFrame.Mode = AddrModeFlat;
  stackFrame.AddrStack.Mode = AddrModeFlat;
  
  HANDLE process = ::GetCurrentProcess();
  
  ::SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
  
  if (!::SymInitialize(process, NULL, TRUE)) {
    log("SymInitialize failed : %d\n", GetLastError());
  }
  
  SYMBOL_INFO  *symbol =
    (SYMBOL_INFO*) ::calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
  symbol->MaxNameLen   = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
  
  while (StackWalk64(machineType,
    process,
    GetCurrentThread(),
    &stackFrame, 
    &context_record,
    NULL,
    &SymFunctionTableAccess64,
    &SymGetModuleBase64,
    NULL)) {
      DWORD64 displacement = 0;

      if (::SymFromAddr(process, (DWORD64) stackFrame.AddrPC.Offset, &displacement, symbol)) {
        IMAGEHLP_MODULE64 moduleInfo = {0};
        moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
        logn("*** ");
        
        if (::SymGetModuleInfo64(process, symbol->ModBase, &moduleInfo)) {
          logn("%s: ", moduleInfo.ModuleName);
        }

        logn("%s - 0x%08X", symbol->Name, stackFrame.AddrPC.Offset);
        
        IMAGEHLP_LINE64 line = {0};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD lineDisplacement = 0;
        
        if (::SymGetLineFromAddr64(process, stackFrame.AddrPC.Offset, &lineDisplacement, &line)) {
          logn(" (%s:%d)", line.FileName, line.LineNumber);
        }
        
        log("");
      }
    }
    
  ::SymCleanup(process);
}

LONG WINAPI win32CrashHandler(struct _EXCEPTION_POINTERS *crashInfo)
{
  char *faultType = "";
  
  switch(crashInfo->ExceptionRecord->ExceptionCode)
  {
    case EXCEPTION_ACCESS_VIOLATION: 
      faultType = "ACCESS VIOLATION";
      break;
    case EXCEPTION_DATATYPE_MISALIGNMENT: 
      faultType = "DATATYPE MISALIGNMENT";
      break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO: 
      faultType = "FLT DIVIDE BY ZERO";
      break;
    default:
      faultType = "(unknown)";
      break;
  }
  
  int errorCode = crashInfo->ExceptionRecord->ExceptionCode;
  PVOID codeAddress = crashInfo->ExceptionRecord->ExceptionAddress;

  log("****************************************************");
  log("*** A Program Fault occurred:");
  log("*** Error code %08X: %s", errorCode, faultType);
  log("****************************************************");
  log("***   Address: %08X", codeAddress);
  log("***     Flags: %08X",
  crashInfo->ExceptionRecord->ExceptionFlags);
  ::logStackFrames(*crashInfo->ContextRecord);
  log("****************************************************");
  
  return EXCEPTION_CONTINUE_SEARCH;
}

void install_crash_handler()
{
  ::SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER) win32CrashHandler);
}
#endif // GLADE_WINDOWS
