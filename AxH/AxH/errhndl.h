#ifndef __H_ERROR_HANDLING
#define __H_ERROR_HANDLING

#include "Misc.h"
#include "Env.h"

#include <exception>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "inc_settings.h"

#ifdef _ENV_LINUX
#include <iostream>
#endif

#include "dtypes.h"

#define CHANNEL_GENERAL_DEBUG 0
#define CHANNEL_WIN32 1
#define CHANNEL_MATH 2
#define CHANNEL_FILEIO 3
#define CHANNEL_RESOURCES 4
#define CHANNEL_DECONST_DEBUG 5
#define CHANNEL_VULKAN 6
#define CHANNEL_PARALLEL 7 
#define CHANNEL_MEMORY 8

#define PRIORITY_HALT 32
#define PRIORITY_MESSAGE 16
#define PRIORITY_DEBUG_MESSAGE 0

#define CHANNEL_GFLW CHANNEL_WIN32

#define PRINT_STREAM Stream::ChannelPrintStream::instance()

///<summary>PRINT(LOCID, MESS, CHANNEL_TARGET, PRIORITY)</summary>
#ifdef __LINUX
#define PRINTP(M, P, C) PRINT_STREAM.printInfo(__func__, M, P, C)
#define PRINTA(M) PRINT_STREAM.printInfo(__func__, M, 0, CHANNEL_GENERAL_DEBUG);
#define PRINT(M, C) PRINT_STREAM.printInfo(__func__, M, 0, C)
#else 
#define PRINTP(M, P, C) PRINT_STREAM.printInfo(__FUNCTION__, M, P, C)
#define PRINTA(M) PRINT_STREAM.printInfo(__FUNCTION__, M, 0, CHANNEL_GENERAL_DEBUG);
#define PRINT(M, C) PRINT_STREAM.printInfo(__FUNCTION__, M, 0, C)
#endif


///<summary>PRINT_ERR(LOCID, MESS, PRIORITY, CHANNEL_TARGET)</summary>
#define PRINT_ERR_F(L, M, P, ...) \
	PRINT_STREAM.printError(L, M, P, __VA_ARGS__ ); \
	if constexpr (P == PRIORITY_HALT){throw new std::exception();}
#define PRINT_ERR(M, P, ...) PRINT_ERR_F(__FUNCTION__, M, P, __VA_ARGS__)

#define PTRSTR(A) PRINT_STREAM.pointerToString(A)
#define DEVPTRSTR(A) PRINT_STREAM.devicePointerToString(A)

#define ASSERT(X,M,C) if(!(X)) { PRINT_ERR(M, PRIORITY_HALT, C); }
#define COND_INFO(X, M, C) if(!(X)) {PRINT(M, C); }


#ifdef __ROBUST
#define ROBUST_ASSERT(X,M,C) ASSERT(X,M,C)
#define ROBUST_INFO(X, M, C ) COND_INFO(X,M,C)
#else
#define ROBUST_ASSERT(X,M,C)
#define ROBUST_INFO(X,M,C)
#endif


#ifdef __DEBUG
#define DPRINT(M,C) PRINT(M,C)
#define DASSERT_INFO(X,M,C) COND_INFO(X,M,C)
#else
#define DPRINT(M,C) 
#define DASSERT_INFO(X,M,C)
#endif

namespace Stream {

	static std::mutex _OutputDebugStringA_Mutex;

#define _AQUIRE_STRINGOUTPUT_MUTEX std::unique_lock<std::mutex> __OUTPUT_MUTEX__(_OutputDebugStringA_Mutex);
#define _RELEASE_STRINGOUTPUT_MUTEX __OUTPUT_MUTEX__.unlock();

#ifdef _ENV_WIN
#define OUTPUTSTRINGFUNC OutputDebugStringA
#endif
#ifdef _ENV_LINUX
#define OUTPUTSTRINGFUNC ___LINUX__print
#endif

#define OUTPUTSTRING(...) _AQUIRE_STRINGOUTPUT_MUTEX OUTPUTSTRINGFUNC(__VA_ARGS__); _RELEASE_STRINGOUTPUT_MUTEX

	typedef uint64 print_channel;

#define _TAG_SYNTAX_FORMATTER(a,b,c) (std::string("[") + b + "] @" + a + ": " + c)
#define _ADD_CHANNEL_ID(a,b) (std::string("|")+b+"| "+a)

	///<summary>
	/// This Method recieves every Info message (state >= 0). 
	/// For every error message (state < 0), the function must return a message for the error dialog. The return string is otherwise ignored.
	///</summary>
	typedef std::string(*channelStrFunc)(uint64 state, const std::string& message);

	struct ChannelPrintStream {
	private:
		std::map<print_channel, std::vector<channelStrFunc>> chId;
		ChannelPrintStream();

		~ChannelPrintStream();

		static ChannelPrintStream stream;
	public:
		///<summary>
		///	Prints Debug Info
		///</summary>
		///<param name='locationID'>Description of the method from which the message originates.  
		/// Usually in the Syntax: STRUCT::METHOD </param>
		///<param name='message'>Debug Info Message</param>
		///<param name='channel'>Destination Channel</param>
		///<param name='priority'>The priority of the message. Starting from highest priority of 0 to any value desired (2^32-1)</param>
		thread_safe void printInfo(const std::string& locationID, const std::string& message, uint64 priority = 0, print_channel channel = CHANNEL_GENERAL_DEBUG);

		///<summary>
		///	Writes an error message, opens an error dialog and throws an exception with the given message.
		///</summary>
		///<param name='locationID'>Description of the method from which the message originates.  
		/// Usually in the Syntax: STRUCT::METHOD </param>
		///<param name='message'>Debug Info Message</param>
		///<param name='channel'>Destination Channel</param>
		thread_safe void printError(const std::string& locationID, const std::string& message, uint64 priority = PRIORITY_HALT, print_channel channel = CHANNEL_GENERAL_DEBUG);

		///<summary>
		///Asserts the input to be true. If it is not, it causes an exception
		///</summary>
		///<param name='in'>Value to be asserted</param>
		///<param name='mes'>Optional Message</param>
		///<param name='channel'>Destination Channel</param>
		void assert(bool in, const std::string& mes = "Assert Exception Triggered!", print_channel channel = CHANNEL_GENERAL_DEBUG);

		///<summary>
		///Converts the passed pointer into a string
		///Useful for deconst debugging
		///</summary>
		std::string pointerToString(const void* ptr) const;

		///<summary>
		///Converts the passed device pointer into a string
		///Useful for deconst debugging
		///Difference to pointerToString are solely about formatting
		///</summary>
		std::string devicePointerToString(const void* ptr) const;

		///<summary>
		///Registeres a Print Channel via its print_channel enum
		///</summary>
		void registerChannel(print_channel ch, channelStrFunc func);

		///<summary>
		///Returns the instance of the ChannelPrintStream
		///</summary>
		static ChannelPrintStream& instance();
	private:
		///<summary>
		/// Prints a string in the format of: 
		///	"[$locationID$][$tag$]: $message$ \n". The message is split into lines to then print every single resulting line in this format. 
		///</summary>
		///<param name='locationID'>Location ID of Sender</param>
		///<param name='message'>Message to show</param>
		///<param name='tag'>The tag string</param>
		///<param name='param'>The String function parameter (state)</param>
		///<param name='channel'>Destination Channel</param>
		void _prntFormattedSplit(const std::string& locationID, const std::string& message, const std::string& tag, uint64 param, print_channel channel = CHANNEL_GENERAL_DEBUG);

		///<summary>
		///Shows an Error Dialog Window with the given message. Stops the program if exc is true.
		///</summary>
		///<param name='in'>Message to show</param>
		///<param name='exc'>Exception flag</param>
		void _showErrorDialog(const std::string& in, bool exc);

		///<summary>
		/// Writes a String into the OuputStream
		///</summary>
		///<param name='in'>Output message</param>
		///<param name='channel'>Destination Channel</param>
		///<param name='state'>StrFunc param</param>
		inline void _outPutMessage(uint64 state, const std::string& in, print_channel channel = CHANNEL_GENERAL_DEBUG);

		///<summary>
		/// Resolves the enum and returns the according function pointer.
		///</summary>
		///<param name='in'>The print_channel enum</param>
		std::vector<channelStrFunc>& resolveChannelID(print_channel in);
	};
	struct Exception : std::exception {
	private:
		///<summary>The exception message</summary>
		std::string message;

	public:
		///<summary>Creates a new Exception</summary>
		Exception(const std::string& in);

		///<summary>Creates a new Exception</summary>
		Exception(const std::string& a, const std::string& b);

		///<summary>Returns an explanation</summary>
		virtual const char* what() const throw();

	};
}

#endif
