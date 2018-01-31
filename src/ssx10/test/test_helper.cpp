#include "test_helper.h"

#include <boost/log/trivial.hpp>

#include "../utils/hex.h"

namespace ssx {
	namespace testing {
		bool check_response_data(const void * req_buffer, const void * rep_buffer,unsigned int rep_bytes)
		{
			const  int MIN_RESPONSE_BYTES = 9;
			const unsigned char* req_data = (const unsigned char*)req_buffer;
			const unsigned char* rep_data = (const unsigned char*)rep_buffer;

			std::string req_hex = ssx::utils::encode_hex(req_data, 16, true);
			std::string rsp_hex = ssx::utils::encode_hex(rep_data, rep_bytes, true);
			if (rep_bytes < MIN_RESPONSE_BYTES) {
				BOOST_LOG_TRIVIAL(error) << "协议错误,write返回字节: " << rep_bytes << "太少, 不满足最小响应报文.响应数据:" << rsp_hex;
				return false;
			}
			if (0 != memcmp(rep_data, req_data, 4)) {
				BOOST_LOG_TRIVIAL(error) << "协议错误,4字节报文体不匹配,请求:" << req_hex << ".响应:" << rsp_hex;
				return false;
			}
			int request_cmd = req_data[4];
			int response_cmd = rep_data[4];
			if (response_cmd != (request_cmd + 0x20)) {
				BOOST_LOG_TRIVIAL(error) << "协议错误,响应的命令字与请求不匹配,请求:" << req_hex << ".响应:" << rsp_hex;
				return false;
			}
			int msg_bytes = rep_data[5] + 8;
			if (msg_bytes != rep_bytes) {
				BOOST_LOG_TRIVIAL(error) << "协议错误,报文体实际长度与write返回字节不匹配,io_bytes = " << rep_bytes << ", msg_bytes =" << msg_bytes << ".请求:" << req_hex << ".响应:" << rsp_hex;
				return false;
			}
			
			int except_crc = make_crc(rep_data,4, msg_bytes - 6);
			int crc = *((unsigned short*)(rep_data + msg_bytes - 2)); // FIXME: only works on little endian platform
			if (crc != except_crc) {
				BOOST_LOG_TRIVIAL(error) << "协议错误,响应报文的CRC错误,请求:" << req_hex << ".响应:" << rsp_hex;
				return false;
			}
			int ret_code = rep_data[6];
			if (ret_code != 0) {
				BOOST_LOG_TRIVIAL(error) << "芯片已经应答请求,但返回错误 "<< ret_code <<",请求:" << req_hex << ".响应:" << rsp_hex;
				return false;
			}
			return true;
		}

		void print_test_samples(const std::string& title, const std::vector<ChipRequestPtr>& request_list) {
			
			for (int i = 0; i < request_list.size();++i) {
				ChipRequestPtr r = request_list[i];
				int msg_len = (r->get_buffer()[5]) + 8;
				BOOST_LOG_TRIVIAL(info)<< title<<" "<< (i+1)<< " : " << ssx::utils::encode_hex(r->get_buffer(), msg_len, true);
			}
		}

		int make_crc(const void* ptr,unsigned int offset,unsigned int length) {
			const unsigned char* src = (const unsigned char*)ptr;
			int s1 = 0, s2 = 0;
			int p1 = 0, p2 = 0;
			for (int i = 0; i < length; ++i) {
				s2 = 0xFFFF & (s1 + (0xFF & src[i + offset]));
				p2 = 0xFFFF & (p1 + s2);
				s1 = s2;
				p1 = p2;
			}
			// FIXME: it little endian
			return p2 & 0xFFFF; // as unsigned short
		}

	}
}