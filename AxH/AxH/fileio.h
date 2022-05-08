#ifndef __H_IO
#define __H_IO

#include <map>
#include <vector>
#include <string>
#include <fstream>

#include "env.h"
#include "dtypes.h"
#include "res_type.h"
#include "ptr.h"

#define MSTAG_BASE_TYPE_COUNT 13
#define _MSTAG_ENUM_ARR_FROM_BASE(A) A##_ARR


#ifdef _ENV_WIN
#define DIR_SEPERATOR '\\'
#endif
#ifdef _ENV_LINUX
#define DIR_SEPERATOR '/'
#endif

#define RESOURCE_IMAGES std::string("res")	+	std::string(1, DIR_SEPERATOR)	+	std::string("img")
#define RESOURCE_SHADERS std::string("res")	+	std::string(1, DIR_SEPERATOR)	+	std::string("sha")
#define RESOURCE_MODELS std::string("res")	+	std::string(1, DIR_SEPERATOR)	+	std::string("mdl")
#define RESOURCE_TAG std::string("res")		+	std::string(1, DIR_SEPERATOR)	+	std::string("tag")
#define RESOURCE_SOUND std::string("res")		+	std::string(1, DIR_SEPERATOR)	+	std::string("snd")

#ifdef _ENV_LINUX
#define __DEFAULT_IOOPEN_MODE std::ios_base::openmode::_S_ate
#endif

#ifdef _ENV_WIN
#define __DEFAULT_IOOPEN_MODE 0
#endif


namespace IO {
	struct Directory {
	private:
		std::vector<std::string> cnt;

	public:
		///<summary>
		///Creates a directory from a vector of sub directory string
		/// f.e.: "C:", "Users", "D", "Desktop"
		///</summary>
		///<param name="in">Path String</param>
		Directory(const std::vector<std::string>& in);

		///<summary>
		///Creates a directory from a path string
		/// f.e.: "C:\Users\D\Desktop"
		///</summary>
		///<param name="in">Path String</param>
		Directory(const std::string& in);

		///<summary>
		///Creates a dummy directory
		///</summary>
		Directory() { }

		///<summary>
		///returns the parent directory
		///</summary>
		Directory back() const;

		///<summary>
		///Appends the give directory onto the path
		///f.e.:"C:\" --- forth("Users") ---> "C:\Users"
		///</summary>
		///</param name="dir">Directory name</param>
		Directory forth(std::string dir) const;

		///<summary>
		///Adds the given directory to the beginning of the path
		///f.e.: "Users" --- appendAtBeginning("C:") ---> "C:\Users"
		///</summary>
		///<param name="dir">Directory Name</param>
		void appendAtBeginning(const Directory& dir);

		///<summary>Returns the directory path as a string</summary>
		std::string getAsString() const;

		///<summary>
		///Returns a vector of all parent directories (recursively)
		///f.e.: "C:\Users\D" -> "C:", "Users", "D"
		///</summary>
		const std::vector<std::string>& getDirectoryStack() const;
	};
	struct File {
	private:
		Directory dir;
		std::string name = "";

	public:
		///<summary>
		///Creates a File object using the given path
		///</summary>
		///<param name="path">The path to the file</param>
		File(const std::string path);

		///<summary>
		///Creates a File object using a reference directory and the name of the file
		///</summary>
		///<param name="dir">The path to the directory of the file</param>
		///<param name="name">The name of the file</param>
		File(const Directory& dir, const std::string& name);

		///<summary>
		///Creates a dummy file
		///</summary>
		File() {}

		///<summary>
		///Returns the Path of the File as a String
		///</summary>
		std::string getAsString() const;

		///<summary>
		///Returns the name of the file
		///</summary>
		std::string getName() const;
	};

	struct ResourceLocation {
	private:
		File file;
		Res::ResType category;

	public:
		///<summary>
		///Creates an abstract reference to a resource using a ContextResource Category and a stub path to the file.
		///<summary>
		///<param name="category">The Category of the resource</param>
		///<param name="stub">The path stub of the resource</param>
		ResourceLocation(Res::ResType category, std::string stub);

		///<summary>
		///Creates an abstract reference to a resource using a ContextResource Category and a stub File.
		///<summary>
		///<param name="category">The Category of the resource</param>
		///<param name="in">The file stub</param>
		ResourceLocation(Res::ResType category, const IO::File& in);

		///<summary>
		///Returns the absolute path to the resource.
		///The category and stub tuple is resolved by adding the excecutable path to fixed path stub associated with the categories and finally the the stub passed during construction.
		///<summary>
		std::string toAbsoluteResourcePath() const;

		///<summary>
		///Returns the name of the abstract resource
		///<summary>
		std::string getName() const;

		///<summary>
		///Returns the stub file to the resource
		///</summary>
		File getFile() const;

	private:
		static std::string exec;
		static bool setExec;
	};

	struct FileStream {
	private:
		std::map<int64, std::streampos> markMap;
		uint64 fsize = -1;
		ResourceLocation r;

	protected:
		std::fstream f;
		File file;

	private:
		bool inStream = false;

	public:

		///<summary>
		///Creates a FileStream from a resourcelocation
		///</summary>
		///<param name="loc">the Resourcelocation of the desired object</param>
		///<param name="iStream">indicates whether the stream is an input stream</param>
		FileStream(ResourceLocation loc, bool iStream, std::ios_base::openmode mode = __DEFAULT_IOOPEN_MODE);

		FileStream(const FileStream& ref);
		void operator=(const FileStream& re);

		///<summary> Closes the stream </summary>
		void close();

		///<summary> Flushes the stream </summary>
		void flush();

		///<summary>
		///marks the current position with the parameter key
		///</summary>
		///<param name="key">The mark key</param>
		void mark(int64 key);

		///<summary>
		///Removes the given mark key
		///</summary>
		///<param name="key">The mark key</param>
		void removeMark(int64 key);

		///<summary>
		///Sets the current position to the position of the given mark key.
		///</summary>
		///<param name="key">The mark key</param>
		void loadMark(int64 key);
		///<summary>
		///Sets the given mark key to a specific location
		///</summary>
		///<param name="mark">The mark key</param>
		///<param name="dev">The wanted location</param>
		void setMark(int64 mark, std::streampos ptr);

		///<summary>
		///Sets the current position to the specified location.
		///</summary>
		///<param name="dev">The wanted location</param>
		void setPos(std::streampos ptr);

		///<summary>
		/// Returns the current position of the stream
		///</summary>
		std::streampos pos();

		///<summary>
		///Returns the EOF flag
		///</summary>
		bool eof();

		///<summary>
		///Returns the size of the current stream.
		///</summary>
		uint64 size();

		///<summary>
		///Returns whether the internal format is LE
		///</summary>
		bool internalFormatLE();

		///<summary>
		///Skips the specified amount of bytes
		///</summary>
		///<param name="len">The bytes to skip</param>
		void skip(uint64 len);

		///<summary>
		///Returns the file being written/read
		///</summary>
		File getFile();
	protected:
		inline void checkBounds();
	};
	struct FileOutputStream : FileStream {
		///<summary>
		///Creates an FileOutputStream. It always writes in !!Little Endian!!.
		///</summary>
		///<param name="loc">The destination resource</param>
		FileOutputStream(ResourceLocation loc, std::ios_base::openmode mode = __DEFAULT_IOOPEN_MODE);

		///<summary>
		///Writes a int8 (byte) into the stream
		///</summary>
		///<param name="in">the data to be written</param>
		void writeChar(int8 in);

		///<summary>
		///Writes a int16 into the stream.
		///</summary>
		///<param name="out">the data to be written</param>
		void writeShort(int16 out);

		///<summary>
		///Writes an int32 into the stream.
		///</summary>
		///<param name="out">the data to be written</param>
		void writeInteger(int32 out);

		///<summary>
		///Writes a int64 into the stream.
		///</summary>
		///<param name="out">the data to be written</param>
		void writeLongLong(int64 out);

		///<summary>
		///Writes a uint8 into the stream.
		///</summary>
		///<param name="in">the data to be written</param>
		void writeUChar(uint8 in);

		///<summary>
		///Writes a uint16 into the stream.
		///</summary>
		///<param name="in">the data to be written</param>
		void writeUShort(uint16 out);

		///<summary>
		///Writes a uint32 into the stream.
		///</summary>
		///<param name="in">the data to be written</param>
		void writeUInteger(uint32 out);

		///<summary>
		///Writes a uint64 into the stream.
		///</summary>
		///<param name="in">the data to be written</param>
		void writeULongLong(uint64 out);

		///<summary>
		///Writes a float into the stream.
		///</summary>
		///<param name="out">the data to be written</param>
		void writeFloat(float out);

		///<summary>
		///Writes a double into the stream.
		///</summary>
		///<param name="out">the data to be written</param>
		void writeDouble(double out);

		///<summary>
		///Writes a string into the stream. This is done by first writing the elements of the string (LE) followed by the char array that represents the string.
		///</summary>
		///<param name="out">the data to be written</param>
		void writeString(std::string out);

		///<summary>
		///Writes an array of bytes into the stream.
		///</summary>
		///<param name="out">the data to be written</param>
		///<param name="size">size of the array</param>
		void writeBytes(const std::vector<byte>& out, int size);

		///<summary>
		///Writes an array of bytes into the stream.
		///</summary>
		///<param name="out">the data to be written</param>
		///<param name="size">size of the array</param>
		void writeBytes(const byte* out, int size);

	private:
		template<typename T> void _write(const T& in) {
			const char* ptr = static_cast<const char*>(static_cast<const void*>(&in));
			if (internalFormatLE()) {
				f.write(ptr, sizeof(T));
			}
			else {
				char curr;
				for (size_t i = 0; i < sizeof(T); i++) {
					curr = ptr[sizeof(T) - 1 - i];
					f.write(&curr, 1);
				}
			}

		}
	};
	struct FileInputStream : FileStream {
		///<summary>
		///Creates an FileOutputStream. It takes its endianess from the machine-internal format.
		///</summary>
		///<param name="loc">The destination resource</param>
		FileInputStream(const ResourceLocation& loc, std::ios_base::openmode mode = __DEFAULT_IOOPEN_MODE);

		///<summary>
		///Reads an int32 from the stream (BE)
		///</summary>
		///<param name ='in'>reference to the destination data</param>
		void readInteger(int32& in);

		///<summary>
		///Reads an int64 from the stream (BE)
		///</summary>
		///<param name ='out'>reference to the destination data</param>
		void readLongLong(int64& out);

		///<summary>
		///Reads an uint64 from the stream (BE)
		///</summary>
		///<param name ='out'>reference to the destination data</param>
		void readULongLong(uint64& out);

		///<summary>
		///Reads an float from the stream (BE)
		///</summary>
		///<param name ='in'>reference to the destination data</param>
		void readFloat(float& in);

		///<summary>
		///Reads an double from the stream (BE)
		///</summary>
		///<param name ='in'>reference to the destination data</param>
		void readDouble(double& in);

		///<summary>
		///Reads an int16 from the stream (BE)
		///</summary>
		///<param name ='in'>reference to the destination data</param>
		void readShort(int16& in);

		///<summary>
		///Reads a string from the stream (BE). It asserts that the chars comprising the string are preceded by an uint32 telling the string's length
		///</summary>
		///<param name ='in'>reference to the destination string</param>
		void readString(std::string& in);

		///<summary>
		///Reads a byte array from the stream (BE)
		///</summary>
		///<param name ='in'>reference to the destination vector</param>
		///<param name ='size'>Size of the array to be read</param>
		void readBytes(std::vector<byte>& in, uint32 size);

		///<summary>
		///Reads a byte array from the stream (BE)
		///</summary>
		///<param name ='in'>pointer to the destination array</param>
		///<param name ='size'>Size of the array to be read</param>
		void readBytes(byte* in, uint32 size);

		///<summary>
		///Reads a uint32 from the stream
		///</summary>
		///<param name ='in'>reference to the destination data</param>
		void readUInteger(uint32& in);

		///<summary>
		///Reads a uint16 from the stream
		///</summary>
		///<param name ='in'>reference to the destination data</param>
		void readUShort(uint16& in);

		///<summary>
		///Reads a uint8 from the stream.
		///</summary>
		///<param name ='in'>reference to the destination data</param>
		void readUChar(uint8& in);

		///<summary>
		///Reads a int8 from the stream.
		///</summary>
		///<param name ='in'>reference to the destination data</param>
		void readChar(int8& in);

		///<summary>
		///Reads a int16 from the stream using little endianess.
		///</summary>
		///<param name ='out'>reference to the destination data</param>
		void readShortLE(int16& out);

		///<summary>
		///Reads a uint16 from the stream using little endianess.
		///</summary>
		///<param name ='out'>reference to the destination data</param>
		void readUShortLE(uint16& out);

		///<summary>
		///Reads a uint32 from the stream using little endianess.
		///</summary>
		///<param name ='out'>reference to the destination data</param>
		void readUIntegerLE(uint32& out);

		///<summary>
		///Reads a int32 from the stream using little endianess.
		///</summary>
		///<param name ='out'>reference to the destination data</param>
		void readIntegerLE(int32& out);

		///<summary>
		///Reads a int64 from the stream using little endianess.
		///</summary>
		///<param name ='out'>reference to the destination data</param>
		void readLongLongLE(int64& out);

		///<summary>
		///Reads a uint64 from the stream using little endianess.
		///</summary>
		///<param name ='out'>reference to the destination data</param>
		void readULongLongLE(uint64& out);

		///<summary>
		///Reads a float from the stream using little endianess
		///</summary>
		///<param name ='out'>reference to the destination data</param>
		void readFloatLE(float& out);

		///<summary>
		///Reads a double from the stream using little endianess.
		///</summary>
		///<param name ='out'>reference to the destination data</param>
		void readDoubleLE(double& out);

		template<typename T> void _readBE(T& in) {
			checkBounds();
			char arr[sizeof(T)];
			f.read(arr, sizeof(T));
			if (internalFormatLE()) {
				char dst[sizeof(T)];
				for (size_t i = 0; i < sizeof(T); i++) {
					dst[i] = arr[sizeof(T) - 1 - i];
				}
				in = *static_cast<T*>(static_cast<void*>(dst));
			}
			else {
				in = *static_cast<T*>(static_cast<void*>(arr));
			}
		}
		template<typename T> inline void _readLE(T& in) {
			checkBounds();
			char arr[sizeof(T)];
			f.read(arr, sizeof(T));
			if (!internalFormatLE()) {
				char dst[sizeof(T)];
				for (size_t i = 0; i < sizeof(T); i++) {
					dst[i] = arr[sizeof(T) - 1 - i];
				}
				in = *static_cast<T*>(static_cast<void*>(dst));
			}
			else {
				in = *static_cast<T*>(static_cast<void*>(arr));
			}
		}

	};

	struct BitStream {
		std::vector<unsigned char>* data;
		int pos = 0;
		WORD bits = 0;
		int handl = 0;
		WORD off = 0;
		BitStream(std::vector<unsigned char>* in, int posIn = 0, int offIn = 0);
	private:
		void assertBits(WORD i);
		WORD pullBits(WORD i);
		void dropBits(WORD i);
		void appendByte();
	public:
		void jumpToNextByte();
		bool readBit();
		WORD readBitsM32(int length);
		int readBitsM32RV(int length);
	};

}
#endif
