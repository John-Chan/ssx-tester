#ifndef SSX_UTILS_HEX_H
#define SSX_UTILS_HEX_H

#include <string>

namespace ssx {
	namespace utils {
		std::string encode_hex(const void* ptr, size_t len, bool human_readable);
		bool decode_hex(const std::string& hex, void* buffer, size_t buffer_size);
	}
}

#endif

