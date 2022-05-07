#ifndef __H_ENV
#define __H_ENV

#include <string>

//You are on a Windows Environment :)
#ifdef _WIN32
#define _ENV_WIN
#endif

//You are on a Linux Environment :)
#ifdef __linux__
#define _ENV_LINUX
#endif

namespace Util {
	struct Env {
		///<summary>This method returns the Path to the Exectuable of the File</summary>
		static std::string getPathOfExecutable();

		///<summary>This method returns on which OS the program is running</summary>
		static std::string getEnvironmentType();
	};
}

#endif
