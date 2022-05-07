#ifndef __H_STRINGUTIL
#define __H_STRINGUTIL

#include <string>
#include <vector>
#include <type_traits>

#include "env.h"


namespace Util {
	struct StringUtils {
		///<summary>
		///	Splits the given string at each occurance of the split argument
		///</summary>
		///<param name='input'>The input string</param>
		///<param name='split'>The split argument</param>
		static std::vector<std::string> split(const std::string& input, char split);


#ifdef _ENV_WIN
		///<summary>
		///Converts the given string to a wide character stringwide
		///</summary>
		///<param name='in'>String to convert</param>
		static std::wstring stringToWideString(const std::string& in);

		///<summary>
		///Converts the given wide string to a normal string
		///</summary>
		///<param name='in'>String to convert</param>
		static std::string wideStringToString(const std::wstring& in);
#endif
	};

	struct ArrayUtils {
		template<
			typename SRC,
			size_t N,
			typename DST,
			size_t M,
			typename = std::enable_if<std::is_convertible<SRC, DST>::value>::type>
		/// <summary>
		/// This method copies contents in the src array to the dst array. The elements to copy accessed per index with the given offset and stride. In total len entries are copied
		/// </summary>
		/// <typeparam name="SRC">Source array type</typeparam>
		/// <typeparam name="DST">Destination array type</typeparam>
		/// <param name="src">source array</param>
		/// <param name="dst">destination array</param>
		/// <param name="offset">offset to first index</param>
		/// <param name="len">amount of entries to copy</param>
		/// <param name="stride">The amount of entries inbetween two consecutive entries to be copied plus one, therefore a stride of 1 correlated to no spacing inbetween elements</param>
		static void arr_copy(const std::array<SRC, N>& src, std::array<DST, M>& dst, size_t offset, size_t len, size_t stride = 1) {
			for (size_t ind = 0; ind < len; ++ind) {
				dst[ind] = src[offset + stride * ind];
			}
		}
	};
}
#endif
