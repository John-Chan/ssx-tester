#include "hex.h"

namespace ssx {
	namespace utils {
		static bool hex_value(char asc_char, unsigned char& out) {
			if (asc_char >= '0' && asc_char <= '9') {
				out = asc_char - '0';
				return true;
			}
			else if (asc_char >= 'A' && asc_char <= 'F') {
				out = asc_char - 'A' + 10;
				return true;
			}
			else if (asc_char >= 'a' && asc_char <= 'f') {
				out = asc_char - 'a' + 10;
				return true;
			}
			else {
				return false;
			}

		}

		//note: buffer size >= len/2
		static bool parse_hex(const void* hex_chars, int len, void* buffer)
		{
			if (len % 2) {
				return false;
			}
			const unsigned char* start = (const unsigned char*)hex_chars;
			unsigned char* out = (unsigned char*)buffer;

			for (int i = 0; i < len; i += 2) {
				unsigned char hi = 0, lo = 0;
				if (hex_value(start[i], hi) && hex_value(start[i + 1], lo)) {
					*out++ = (unsigned char)(hi * 16 + lo);
				}
				else {
					return false;
				}
			}
			return true;
		}

		std::string encode_hex(const void* ptr, size_t len, bool human_readable) {
			static const char hexval[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
			const unsigned char* uptr = (const unsigned char*)ptr;
			std::string hex = "";
			for (size_t j = 0; j < len; ++j) {
				if (human_readable) {
					if (hex.size() > 0) {
						hex += ",0x";
					}
					else {
						hex += "0x";
					}
				}
				hex += (hexval[((uptr[j] >> 4) & 0xF)]);
				hex += (hexval[(uptr[j]) & 0x0F]);
			}
			return hex;
		}

		bool decode_hex(const std::string& hex, void* buffer, size_t buffer_size) {
			if (buffer_size < hex.length() / 2) {
				return false;
			}
			return parse_hex(hex.c_str(),hex.length(), buffer);
		}
	}
}