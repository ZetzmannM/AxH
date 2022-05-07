#include "errhndl.h"

#include "env.h"
#include "dtypes.h"

#ifdef _ENV_WIN
#include <Windows.h>
#include <tchar.h>
#define FRAMEWORK_NAME "Win32"

#endif
#ifdef _ENV_LINUX
#include <iostream>
#include <GLFW/glfw3.h>
#include <string>

namespace Stream {
	static void ___LINUX__print(const std::string& arg);
}

void Stream::___LINUX__print(const std::string& arg) {
	std::cout << arg;
}
#define FRAMEWORK_NAME "GLFW"

#endif

#include <string>
#include <sstream>
#include "utils.h"

namespace Stream {

	///<summary>
	///The General Debug Channel string handling function
	///</summary>
	static std::string _genDbgPrntStrmStrFunc(uint64 state, const std::string& message);

	///<summary>
	///The Math Channel string handling function
	///</summary>
	static std::string _mathPrntStrmStrFunc(uint64 state, const std::string& message);

	///<summary>
	///The Win32 Channel string handling function
	///</summary>
	static std::string _win32PrntStrmStrFunc(uint64 state, const std::string& message);

	///<summary>
	///The File/IO Channel string handling function
	///</summary>
	static std::string _filePrntStrmStrFunc(uint64 state, const std::string& message);

	///<summary>
	///The Deconstructor Debug Channel string handling function
	///</summary>
	static std::string _deconstDebugStrFunc(uint64 state, const std::string& message);

	///<summary>
	///The Vulkan Channel string handling function
	///</summary>
	static std::string _vkStrFunc(uint64 state, const std::string& message);

	///<summary>
	///The ContextResource Channel string handling function
	///</summary>
	static std::string _rscStrFunc(uint64 state, const std::string& message);

	///<summary>
	///The ContextResource Channel string handling function
	///</summary>
	static std::string _prllStrFunc(uint64 state, const std::string& message);

	///<summary>
	///The Memory Channel string handling function
	///</summary>
	static std::string _memStrFunc(uint64 state, const std::string& message);
}

using namespace Stream;

#define DIALOG_THRESHOLD PRIORITY_MESSAGE
#define EXCEPTION_THRESHOLD PRIORITY_HALT

ChannelPrintStream ChannelPrintStream::stream = ChannelPrintStream();
ChannelPrintStream::~ChannelPrintStream() {
#ifdef DECONST_DEBUG
	OUTPUTSTRINGFUNC("~ Deconst CPS\n");
#endif
}

ChannelPrintStream::ChannelPrintStream() {
	registerChannel(CHANNEL_GENERAL_DEBUG, _genDbgPrntStrmStrFunc);
	registerChannel(CHANNEL_WIN32, _win32PrntStrmStrFunc);
	registerChannel(CHANNEL_MATH, _mathPrntStrmStrFunc);
	registerChannel(CHANNEL_FILEIO, _filePrntStrmStrFunc);
	registerChannel(CHANNEL_DECONST_DEBUG, _deconstDebugStrFunc);
	registerChannel(CHANNEL_VULKAN, _vkStrFunc);
	registerChannel(CHANNEL_RESOURCES, _rscStrFunc);
	registerChannel(CHANNEL_PARALLEL, _prllStrFunc);
	registerChannel(CHANNEL_MEMORY, _memStrFunc);
}
void ChannelPrintStream::printInfo(const std::string& locationID, const std::string& message, uint64 priority, print_channel channel) {
	_prntFormattedSplit(locationID, message, "INFO", priority, channel);
}
void ChannelPrintStream::printError(const std::string& locationID, const std::string& message, uint64 priority, print_channel channel) {
	_prntFormattedSplit(locationID, message, "ERROR", priority, channel);
	if (priority >= DIALOG_THRESHOLD) {
		_showErrorDialog("[" + locationID + "]: " + resolveChannelID(channel)[0](-(int64)(priority), message), priority >= EXCEPTION_THRESHOLD);
	}
}
void ChannelPrintStream::assert(bool in, const std::string& mes, print_channel channel) {
	if (!in) {
		printError("CHANNELPRINTSTREAM::ASSERT", mes, channel);
	}
}
void ChannelPrintStream::_prntFormattedSplit(const std::string& locationID, const std::string& message, const std::string& tag, uint64 param, print_channel channel) {
	std::vector<std::string> out = Util::StringUtils::split(message, '\n');
	for (unsigned int i = 0; i < out.size(); i++) {
		_outPutMessage(param, (_TAG_SYNTAX_FORMATTER(locationID, tag, out.at(i)) + "\n"), channel);
	}
}
void ChannelPrintStream::_outPutMessage(uint64 state, const std::string& in, print_channel channel) {
	std::vector<channelStrFunc>& chn = resolveChannelID(channel);
	auto it = chn.begin();
	while (it != chn.end()) {
		(*it)(state, in);
		it++;
	}
}
std::vector<channelStrFunc>& ChannelPrintStream::resolveChannelID(print_channel in) {
	if (chId.count(in)) {
		return chId[in];
	}
	else {
		printError("CHANNELPRINTSTREAM::RESOLVECHANNELID", "Invalid Print Channel!");
	}
}
void ChannelPrintStream::_showErrorDialog(const std::string& in, bool exc) {
#ifdef _ENV_WIN
	ShowCursor(1);
	MessageBox(NULL,
		in.c_str(),
		_T("Parabol Creation"),
		NULL);
	if (exc) {
		PostQuitMessage(0);
		throw std::exception("Program HALT!");
	}
#endif
#ifdef _ENV_LINUX
	GLFWwindow* win = glfwCreateWindow(200, 50, in.c_str(), NULL, NULL);

	while (!glfwWindowShouldClose(win)) {


	}


	glfwTerminate();
	throw std::exception();
#endif
}
void ChannelPrintStream::registerChannel(print_channel ch, channelStrFunc func) {
	if (!chId.count(ch)) {
		chId[ch] = std::vector<channelStrFunc>();
	}
	chId[ch].push_back(func);
}
std::string ChannelPrintStream::pointerToString(const void* ptr) const {
	std::stringstream sstream;
	sstream << "0x" << std::hex << reinterpret_cast<intptr_t>(ptr);
	return sstream.str();
}
std::string ChannelPrintStream::devicePointerToString(const void* ptr) const {
	std::stringstream sstream;
	sstream << "D_" << std::hex << reinterpret_cast<intptr_t>(ptr);
	return sstream.str();
}
ChannelPrintStream& ChannelPrintStream::instance() {
	return ChannelPrintStream::stream;
}
Exception::Exception(const std::string& in) {
	message = in;
}
Exception::Exception(const std::string& a, const std::string& b) {
	message = _TAG_SYNTAX_FORMATTER(a, "EXC", b);
}
const char* Exception::what() const throw() {
	return message.c_str();
}
std::string Stream::_genDbgPrntStrmStrFunc(uint64 state, const std::string& message) {
#if not DEBUG_PRINT_STREAM
	return "";
#endif

	OUTPUTSTRING(_ADD_CHANNEL_ID(message, "Debug").c_str());
	if (!state) {
		return "@Debug: " + message;
	}
	return message;
}
std::string Stream::_mathPrntStrmStrFunc(uint64 state, const std::string& message) {
#if not MATH_PRINT_STREAM
	return "";
#endif

	OUTPUTSTRING(_ADD_CHANNEL_ID(message, "Math").c_str());
	if (!state) {
		return "@Math: " + message;
	}
	return message;
}
std::string Stream::_win32PrntStrmStrFunc(uint64 state, const std::string& message) {
#if not WIN32_PRINT_STREAM
	return "";
#endif

	OUTPUTSTRING(_ADD_CHANNEL_ID(message, FRAMEWORK_NAME).c_str());
	if (!state) {
		return std::string("@") + std::string(FRAMEWORK_NAME) + std::string(": ") + message;
	}
	return message;
}
std::string Stream::_filePrntStrmStrFunc(uint64 state, const std::string& message) {
#if not FILE_PRINT_STREAM
	return "";
#endif

	OUTPUTSTRING(_ADD_CHANNEL_ID(message, "FILE/IO").c_str());
	if (!state) {
		return "@FILE/IO: " + message;
	}
	return message;
}
std::string Stream::_deconstDebugStrFunc(uint64 state, const std::string& message) {
#ifdef DECONST_DEBUG
#if not DECONST_PRINT_STREAM
	return "";
#endif

	OUTPUTSTRING(_ADD_CHANNEL_ID(message, "~D").c_str());
	if (!state) {
		return "@~D: " + message;
	}
#endif
	return message;
}
std::string Stream::_vkStrFunc(uint64 state, const std::string& message) {
#if not VULKAN_PRINT_STREAM
	return "";
#endif

	OUTPUTSTRING(_ADD_CHANNEL_ID(message, "VULKAN").c_str());
	if (!state) {
		return "VULKAN: " + message;
	}

	return message;
}
static std::string Stream::_rscStrFunc(uint64 state, const std::string& message) {
#if not RESOURCE_PRINT_STREAM
	return "";
#endif

	OUTPUTSTRING(_ADD_CHANNEL_ID(message, "RESOURCE").c_str());
	if (!state) {
		return "RESOURCE: " + message;
	}
	return message;
}
static std::string Stream::_prllStrFunc(uint64 state, const std::string& message) {
#if not PARALLEL_PRINT_STREAM
	return "";
#endif

	OUTPUTSTRING(_ADD_CHANNEL_ID(message, "PARALLEL").c_str());
	if (!state) {
		return "PARALLEL: " + message;
	}
	return message;
}
static std::string Stream::_memStrFunc(uint64 state, const std::string& message) {
#if not MEMORY_PRINT_STREAM
	return "";
#endif

	OUTPUTSTRING(_ADD_CHANNEL_ID(message, "MEMORY").c_str());
	if (!state) {
		return "MEMORY: " + message;
	}
	return message;
}
