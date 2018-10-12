#pragma once
//DWORD find_PID_from_ModuleName();
//void PID_to_Token();
//char __stdcall check_with_SCM(char *Str2);
//unsigned long __stdcall StartAddress(LPVOID lpThreadParameter);
int __stdcall dlexec_done(LPCSTR lpszUrl, char *Filename);
int __stdcall DLL_Main(char sel);
void __stdcall Start_DLL_LOOP(LPVOID lpThreadParameter);
char *__stdcall DNS_query_url(char *Source);
void *__stdcall rand_name_making(int a1);

