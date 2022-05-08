
#include "fileio.h"
#include "utils.h"
#include "env.h"
#include "errhndl.h"

using namespace IO;

Directory::Directory(const std::vector<std::string>& in) {
	this->cnt = in;
}
Directory::Directory(const std::string& in) {
	this->cnt = Util::StringUtils::split(in, DIR_SEPERATOR);
}
Directory Directory::back() const {
	std::vector<std::string> sub = cnt;
	sub.resize(cnt.size() - 1);
	return Directory(sub);
}
Directory Directory::forth(std::string dir) const {
	std::vector<std::string> sub = cnt;
	sub.push_back(dir);
	return Directory(sub);
}
void Directory::appendAtBeginning(const Directory& dir) {
	cnt.insert(cnt.begin(), dir.cnt.begin(), dir.cnt.end());
}
std::string Directory::getAsString() const {
	std::string res;
	for (size_t i = 0; i < cnt.size(); i++) {
		res += cnt.at(i) + DIR_SEPERATOR;
	}
	return res;
}
const std::vector<std::string>& Directory::getDirectoryStack() const {
	return cnt;
}

File::File(const std::string iput) : dir(iput) {
	name = dir.getDirectoryStack().at(dir.getDirectoryStack().size() - 1);
	dir = dir.back();
}
File::File(const Directory& ref, const std::string& name) : dir(ref) {
	this->name = name;
}
std::string File::getAsString() const {
	return dir.getAsString() + name;
}
std::string File::getName() const {
	return name;
}

bool ResourceLocation::setExec = false;
std::string ResourceLocation::exec = "";
ResourceLocation::ResourceLocation(Res::ResType category, std::string in) : file(in) {
	if (!setExec) {
		setExec = true;
		exec = Util::Env::getPathOfExecutable();
	}
	this->category = category;
}
ResourceLocation::ResourceLocation(Res::ResType category, const IO::File& in) : file(in) {
	if (!setExec) {
		setExec = true;
		exec = Util::Env::getPathOfExecutable();
	}
	this->category = category;
}
std::string ResourceLocation::toAbsoluteResourcePath() const {
	std::string stub = "";
	switch (category) {
	case Res::ResType::RES_IMAGE:
		stub = RESOURCE_IMAGES;
		break;
	case Res::ResType::RES_SHADER:
		stub = RESOURCE_SHADERS;
		break;
	case Res::ResType::RES_MODEL:
		stub = RESOURCE_MODELS;
		break;
	case Res::ResType::RES_MS_TAG:
		stub = RESOURCE_TAG;
		break;
	case Res::ResType::RES_SOUND:
		stub = RESOURCE_SOUND;
		break;
	}
	PRINT("Loading File : " + (exec + DIR_SEPERATOR + stub + DIR_SEPERATOR + file.getAsString()), CHANNEL_FILEIO);
	return exec + DIR_SEPERATOR + stub + DIR_SEPERATOR + file.getAsString();
}
std::string ResourceLocation::getName() const {
	return file.getName();
}
File ResourceLocation::getFile() const {
	return file;
}

FileStream::FileStream(ResourceLocation loc, bool iStream, std::ios_base::openmode flags) : r(loc), file(loc.getFile()), inStream{ iStream } {
	this->f.open(r.toAbsoluteResourcePath(), iStream ? (std::fstream::in | std::ios::binary | std::ios::ate | flags) : (std::fstream::out | std::ios::binary | flags));
	std::streampos curr;

	if (iStream) {
		curr = f.tellg();
	}

	if (!iStream) {
		curr = f.tellp();
	}

	if (curr == -1) {
		PRINT_ERR("Invalid File! " + loc.toAbsoluteResourcePath(), PRIORITY_HALT, CHANNEL_FILEIO);
	}

	if (iStream) {
		fsize = pos();
		f.seekg(0);
	}
}
FileStream::FileStream(const FileStream& ref) : r(ref.r) {
	this->operator=(ref);
}
void FileStream::operator=(const FileStream& re) {
	this->markMap = re.markMap;
	this->inStream = re.inStream;
	this->fsize = re.fsize;
	this->file = re.file;
	this->r = re.r;
	this->f.open(r.toAbsoluteResourcePath(), this->inStream ? (std::fstream::in | std::ios::binary) : (std::fstream::out | std::ios::binary));
	int64 curr = pos();
	if (curr == -1) {
		PRINT_ERR("Invalid File! " + r.toAbsoluteResourcePath(), PRIORITY_HALT, CHANNEL_FILEIO);
	}
	if (inStream) {
		f.seekg(0, std::ios::end);
	}
	else {
		f.seekp(0, std::ios::end);
	}
	fsize = pos();
	setPos(curr);
}
void FileStream::close() {
	flush();
	f.close();
}
void FileStream::flush() {
	f.flush();
}
void FileStream::setMark(int64 mark, std::streampos ptr) {
	if (inStream) {
		if (static_cast<std::streamoff>(ptr) < 0) {
			PRINT_ERR("PTR negative", PRIORITY_MESSAGE, CHANNEL_FILEIO);
		}
		if (static_cast<uint64>(static_cast<std::streamoff>(ptr)) < size()) {
			PRINT_ERR("PTR TOO BIG " + std::to_string(ptr) + ", FS:" + std::to_string(f.tellg()), PRIORITY_HALT, CHANNEL_FILEIO);
		}
	}
	this->markMap[mark] = ptr;

}
void FileStream::loadMark(int64 mark) {
	if (this->markMap.count(mark)) {
		setPos(this->markMap[mark]);
	}
	else {
		PRINT_ERR("FILES::FILESTREAM: Invalid mark " + mark, PRIORITY_HALT, CHANNEL_FILEIO);
	}
}
void FileStream::mark(int64 mark) {
	this->markMap[mark] = pos();
}
bool FileStream::internalFormatLE() {
	uint32 intValue = 0x01020304;
	char* ch = reinterpret_cast<char*>(&intValue);
	return (ch[0] == 0x04);
}
void FileStream::removeMark(int64 mark) {
	if (!markMap.count(mark)) {
		PRINT_ERR("Invalid Mark " + std::to_string(mark), PRIORITY_HALT, CHANNEL_FILEIO);
	}
	this->markMap.erase(mark);
}
void FileStream::setPos(std::streampos ref) {
	if (static_cast<std::streamoff>(ref) < 0) {
		PRINT_ERR("PTR negative", PRIORITY_HALT, CHANNEL_FILEIO);
	}
	if (static_cast<uint64>(static_cast<std::streamoff>(ref)) > size()) {
		PRINT_ERR("Invalid Stream Position DBG:" + std::to_string(ref) + ", size:" + std::to_string(size()), PRIORITY_HALT, CHANNEL_FILEIO);
	}
	if (static_cast<uint64>(static_cast<std::streamoff>(ref)) == size()) {
		if (inStream) {
			f.seekg(std::iostream::end);
		}
		else {
			f.seekp(std::iostream::end);
		}
	}
	if (inStream) {
		f.seekg(ref);
	}
	else {
		f.seekp(ref);
	}
}
std::streampos FileStream::pos() {
	if (inStream) {
		return f.tellg();
	}
	else {
		return f.tellp();
	}
}
bool FileStream::eof() {
	std::streampos pos1 = pos();
	if (static_cast<std::streamoff>(pos1) < 0) {
		PRINT_ERR("Sketchy Value returned", PRIORITY_MESSAGE, CHANNEL_FILEIO);
		pos1 = 0;
	}
	uint64 size1 = size();
	return static_cast<uint64>(static_cast<std::streamoff>(pos1)) >= size1 || f.eof();
}
uint64 FileStream::size() {
	return fsize;
}
void FileStream::skip(uint64 len) {
	setPos(static_cast<std::streamoff>(pos()) + len);
}
File FileStream::getFile() {
	return file;
}

FileOutputStream::FileOutputStream(ResourceLocation loc, std::ios_base::openmode mode) : FileStream(loc, false, mode) {}
void FileOutputStream::writeInteger(int32 out) {
	f.write(static_cast<char*>(static_cast<void*>(&out)), sizeof(int32));
}
void FileOutputStream::writeFloat(float out) {
	_write<float>(out);
}
void FileOutputStream::writeDouble(double out) {
	_write<double>(out);
}
void FileOutputStream::writeShort(int16 out) {
	_write<int16>(out);
}
void FileOutputStream::writeString(std::string out) {
	writeUInteger(static_cast<uint32>(out.size()));//size_t->int
	f.write(out.c_str(), out.size());
}
void FileOutputStream::writeBytes(const std::vector<byte>& out, int size) {
	//f.write((char*)(&(out[0])), elements);
	for (int i = 0; i < size; i++) {
		writeUChar(out.at(i));
	}
}
void FileOutputStream::writeBytes(const byte* arrayOut, int size) { // UNSAFE
	f.write(static_cast<const char*>(static_cast<const void*>(arrayOut)), size);
}
void FileOutputStream::writeUInteger(uint32 out) {
	_write<uint32>(out);
}
void FileOutputStream::writeUShort(uint16 out) {
	_write<uint16>(out);
}
void FileOutputStream::writeLongLong(int64 out) {
	_write<int64>(out);
}
void FileOutputStream::writeUChar(uint8 in) {
	f.write(static_cast<char*>(static_cast<void*>(&in)), sizeof(uint8));
}
void FileOutputStream::writeULongLong(uint64 in) {
	_write<uint64>(in);
}
void FileOutputStream::writeChar(int8 in) {
	char chr = (char)in;
	f.write(&chr, sizeof(int8));
}

FileInputStream::FileInputStream(const ResourceLocation& loc, std::ios_base::openmode mode) : FileStream(loc, true, mode) {}
void FileInputStream::readInteger(int32& in) {
	_readBE<int32>(in);
}
void FileInputStream::readFloat(float& in) {
	_readBE<float>(in);
}
void FileInputStream::readDouble(double& in) {
	_readBE<double>(in);
}
void FileInputStream::readShort(int16& in) {
	_readBE<int16>(in);
}
void FileInputStream::readString(std::string& in) {
	checkBounds();
	unsigned int size;
	readUIntegerLE(size);
	std::vector<char> data(size);
	f.read(&(data[0]), size);
	in = std::string(data.begin(), data.end());
}
void FileInputStream::readBytes(std::vector<byte>& in, uint32 size) {
	checkBounds();
	uint8 c;
	for (uint32 i = 0; i < size; i++) {
		readUChar(c);
		in.push_back(c);
	}
}
void FileInputStream::readBytes(byte* arrayIn, uint32 size) {
	checkBounds();
	f.read(static_cast<char*>(static_cast<void*>(arrayIn)), size);
}
void FileInputStream::readUInteger(uint32& in) {
	_readBE<uint32>(in);
}
void FileInputStream::readUShort(uint16& in) {
	_readBE<uint16>(in);
}
void FileInputStream::readLongLong(int64& in) {
	_readBE<int64>(in);
}
void FileInputStream::readULongLong(uint64& in) {
	_readBE<uint64>(in);
}
void FileInputStream::readUChar(uint8& in) {
	checkBounds();
	in = f.get();
}
void FileInputStream::readChar(int8& in) {
	checkBounds();
	in = f.get();
}
void FileStream::checkBounds() {
	if (eof()) {
		PRINT_ERR("FILES::CHECKBOUNDS: EOF!", PRIORITY_HALT, CHANNEL_FILEIO);
	}
}
void FileInputStream::readIntegerLE(int32& in) {
	_readLE<int32>(in);
}
void FileInputStream::readFloatLE(float& in) {
	_readLE<float>(in);
}
void FileInputStream::readDoubleLE(double& in) {
	_readLE<double>(in);
}
void FileInputStream::readShortLE(int16& in) {
	_readLE<int16>(in);
}
void FileInputStream::readUIntegerLE(uint32& in) {
	_readLE<uint32>(in);
}
void FileInputStream::readUShortLE(uint16& in) {
	_readLE<uint16>(in);
}
void FileInputStream::readLongLongLE(int64& in) {
	_readLE<int64>(in);
}
void FileInputStream::readULongLongLE(uint64& in) {
	_readLE<uint64>(in);
}

BitStream::BitStream(std::vector<unsigned char>* in, int posIn, int offIn) {
	assertBits(offIn);
	this->pos = posIn;
	this->data = in;
	this->bits = 0;
}
void BitStream::assertBits(WORD i) {
	while (bits < i) { appendByte(); }
}
WORD BitStream::pullBits(WORD i) {
	return (handl & ((1 << i) - 1));
}
void BitStream::dropBits(WORD i) {
	handl >>= i;
	bits -= i;
	off += i;
	off %= 8;
}
void BitStream::appendByte() {
	handl += (data->at(pos++)) << bits;
	bits += 8;
}
bool BitStream::readBit() {
	assertBits(1);
	bool rt = pullBits(1);
	dropBits(1);
	return rt;
}
WORD BitStream::readBitsM32(int length) {
	assertBits(length);
	WORD rt = pullBits(length);
	dropBits(length);
	return rt;
}
int BitStream::readBitsM32RV(int length) {
	int rt = 0;
	for (int i = 0; i < length; i++) {
		rt = (rt << 1) + readBit();
	}
	return rt;
}
void BitStream::jumpToNextByte() {
	dropBits(off != 0 ? (8 - off) : 0);
}
