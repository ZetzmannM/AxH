#include "env.h"

#ifdef _ENV_WIN
#include <Windows.h>

std::string Util::Env::getPathOfExecutable() {
	HMODULE hModule = GetModuleHandleW(NULL);
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(hModule, path, MAX_PATH);
	std::wstring ws(path);
	size_t lastIndex = ws.find_last_of('\\');
	std::string str(ws.begin(), ws.begin() + lastIndex);
	return str;

}
std::string Util::Env::getEnvironmentType() {
	return "Win32";
}
#endif

#ifdef _ENV_LINUX

#include <linux/limits.h>
#include <unistd.h>

using namespace Util;

std::string Util::Env::getPathOfExecutable() {
	char exePath[PATH_MAX];
	ssize_t len = ::readlink("/proc/self/exe", exePath, sizeof(exePath));
	if (len == -1 || len == sizeof(exePath))
		len = 0;
	exePath[len] = '\0';
	std::string s(exePath);
	size_t lastIndex = s.find_last_of("/");
	std::string str(s.begin(), s.begin() + lastIndex);
	return str;
}

std::string Util::Env::getEnvironmentType() {
	return "X11";
}

#endif
