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
				BOOST_LOG_TRIVIAL(error) << "Э�����,write�����ֽ�: " << rep_bytes << "̫��, ��������С��Ӧ����.��Ӧ����:" << rsp_hex;
				return false;
			}
			if (0 != memcmp(rep_data, req_data, 4)) {
				BOOST_LOG_TRIVIAL(error) << "Э�����,4�ֽڱ����岻ƥ��,����:" << req_hex << ".��Ӧ:" << rsp_hex;
				return false;
			}
			int request_cmd = req_data[4];
			int response_cmd = rep_data[4];
			if (response_cmd != (request_cmd + 0x20)) {
				BOOST_LOG_TRIVIAL(error) << "Э�����,��Ӧ��������������ƥ��,����:" << req_hex << ".��Ӧ:" << rsp_hex;
				return false;
			}
			int msg_bytes = rep_data[5] + 8;
			if (msg_bytes != rep_bytes) {
				BOOST_LOG_TRIVIAL(error) << "Э�����,������ʵ�ʳ�����write�����ֽڲ�ƥ��,io_bytes = " << rep_bytes << ", msg_bytes =" << msg_bytes << ".����:" << req_hex << ".��Ӧ:" << rsp_hex;
				return false;
			}
			// FIXME: CRC CHECK
			return true;
		}

		void print_test_samples(const std::string& title, const std::vector<ChipRequestPtr>& request_list) {
			
			for (int i = 0; i < request_list.size();++i) {
				ChipRequestPtr r = request_list[i];
				int msg_len = (r->get_buffer()[5]) + 8;
				BOOST_LOG_TRIVIAL(info)<< title<<"  " << ssx::utils::encode_hex(r->get_buffer(), msg_len, true);
			}
		}
	}
}