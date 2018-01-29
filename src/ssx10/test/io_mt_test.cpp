#include "io_mt_test.h"


#include <list>
#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/asio.hpp> // for timer

#include "../io/win32/ssx_io.h"
#include "../utils/hex.h"

namespace ssx {
	namespace testing {
		static const unsigned int IO_BUFF_SIZE = 512;
		
		class ChipRequest {
		private:
			boost::mutex mutex_;
			boost::condition_variable io_condition_;
			unsigned char buffer_[IO_BUFF_SIZE];
			HANDLE fd_;
			boost::atomic_bool io_ready_;
		public:
			ChipRequest(HANDLE handle)
			:fd_(handle),io_ready_(true){

			}
			unsigned char* get_buffer() { return buffer_; }
			boost::mutex& get_mutex() { return mutex_; }
			size_t buffer_bytes() { return sizeof(buffer_); }
			boost::atomic_bool& io_ready() { return io_ready_; }
			boost::condition_variable& io_condition() {
				return io_condition_;
			}
			HANDLE get_handle() { return fd_; }
		};
		class Seq
		{
		public:
			Seq(int min,int max)
				:min_(min), max_(max){
				seq_ = min;
			}
			~Seq() {}
			int next() {
				boost::mutex::scoped_lock lock(mutex_);
				if (seq_ >= max_) {
					seq_ = min_;
				}
				int val = seq_;
				seq_++;
				return val;
			}

		private:
			boost::mutex mutex_;
			const int min_;
			const int max_;
			boost::atomic_int32_t seq_;
		};

		typedef boost::shared_ptr<ssx::io::win32::HandleWrapper> HandleWrapperPtr;
		typedef boost::shared_ptr<ChipRequest> ChipRequestPtr;
		static std::string packet_template_chip_b_0x43 = "01000500431000000000000003404883122957145088f211";
		static std::string packet_template_chip_a_0x54 = "000002005499001ec4f4225d888d03cdea4b52506a5601000000000000000000000000000000000000000000034048831229571450883120170618000001230000000000000999b0543d90b1000000a5bb08043e964eec635828ea53a0386fad0ac82e3b616ef084851956709730f9045269aaacab6e517ab49abe5624cb24be631e75c7f5f6d94b64b418e5d11c52ab5faa270f66503398df693fd51b65a437a6";
		


		void handle_timer(const boost::system::error_code& error, bool& stop_flag);
		void test_thread(std::vector<ChipRequestPtr>& request_list, Seq& request_seq, bool& stop_flag, bool dynamic_request, boost::atomic_int32_t& running_threads);
		void print_run_io_mt_test_help() {
			std::cout << " ================================================================= " << std::endl;
			std::cout << "������Ĳ�������:����Ԥ���������ɲ�����������(����֤ÿ��оƬֻ��һ������),Ȼ�����̳߳���ִ��IO����,�����Ӧ�����Ƿ����Ԥ�ڽ��" << std::endl;
			std::cout << "������:ssxtester <second> <threads> <dynamic_req> " << std::endl;
			std::cout << "���� second :������ʱ��,��λ��" << std::endl;
			std::cout << "���� threads �����̳߳�,����������ٲ����߳�����. " << std::endl;
			std::cout << "���� dynamic_req :�Ƿ�����̬�������, y��ʾ����.����ر�,ÿһ�������߳����Ƿ�����ͬ������" << std::endl;
			std::cout << "����:ssxtester 60 40 y " << std::endl;
			std::cout << " ================================================================= " << std::endl;
		}
		void run_io_mt_test(unsigned int seconds,int thread_size,bool dynamic_req) {

			BOOST_LOG_TRIVIAL(info) << "׼������, second = "<< seconds <<",threads = "<< thread_size<< ",dynamic_req = "<< (dynamic_req?"Y":"N" );
			

			std::vector<std::string> device_path_list = ssx::io::win32::search_ssx_devices(0, 4);
			std::vector<HandleWrapperPtr> device_handle_list;
			if (device_path_list.size() <= 0) {
				BOOST_LOG_TRIVIAL(error)  << "no device found " ;
				return;
			}
			for (int i = 0; i < device_path_list.size(); ++i) {
				const std::string path = device_path_list[i];
				BOOST_LOG_TRIVIAL(info) << "open device :"<< path  ;
				HandleWrapperPtr h(new ssx::io::win32::HandleWrapper( ssx::io::win32::open_device(path)) );
				if (h->isValid()) {
					device_handle_list.push_back(h);
				}
				else {
					BOOST_LOG_TRIVIAL(error) << "open device failed: " << path ;
				}
			}

			const int chip_a_count = 8;
			const int chip_b_count = 32;
			if (thread_size <= 0) {
				thread_size = chip_a_count + chip_b_count;
			}
			bool stop_test = false;
			Seq request_seq(0, chip_b_count + chip_a_count);
			std::vector<ChipRequestPtr> request_list;
			for (int di = 0; di < device_handle_list.size(); ++di) {
				for (int chip_idx = 0; chip_idx < chip_b_count; ++chip_idx) {
					ChipRequestPtr cr(new ChipRequest(device_handle_list[di]->getHandle() ));
					memset(cr->get_buffer(), 0x0, cr->buffer_bytes());
					ssx::utils::decode_hex(packet_template_chip_b_0x43, cr->get_buffer(), cr->buffer_bytes());
					//cr->get_buffer()[0] = (unsigned char)di;
					cr->get_buffer()[2] = (unsigned char)chip_idx;
					request_list.push_back(cr);
				}
			}
			for (int di = 0; di < device_handle_list.size(); ++di) {
				for (int chip_idx = 0; chip_idx < chip_a_count; ++chip_idx) {
					ChipRequestPtr cr(new ChipRequest(device_handle_list[di]->getHandle()));
					memset(cr->get_buffer(), 0x0, cr->buffer_bytes());
					ssx::utils::decode_hex(packet_template_chip_a_0x54, cr->get_buffer(), cr->buffer_bytes());
					//cr->get_buffer()[0] = (unsigned char)di;
					cr->get_buffer()[2] = (unsigned char)chip_idx;
					request_list.push_back(cr);
				}
			}
			boost::atomic_int32_t running_threads(thread_size);
			boost::thread_group test_thread_group;
			bool dynamic_request = false;
			for (int i = 0; i < thread_size;++i) {
				test_thread_group.add_thread(
					new boost::thread(boost::bind(test_thread,boost::ref(request_list), boost::ref(request_seq),boost::ref(stop_test), dynamic_req , boost::ref(running_threads)) )
					);
			}
			

			boost::asio::io_service io;
			boost::asio::deadline_timer t(io);
			t.expires_from_now(boost::posix_time::seconds(seconds)); 
			BOOST_LOG_TRIVIAL(info) << "�ȴ����Խ���: "<< seconds<<" s";
			t.async_wait(boost::bind(handle_timer,
				boost::asio::placeholders::error,
				boost::ref(stop_test)));
			io.run();
			BOOST_LOG_TRIVIAL(info) << "�ȴ������߳��˳� ";
			test_thread_group.join_all();
		}

		void handle_timer(const boost::system::error_code& error, bool& stop_flag) {
			BOOST_LOG_TRIVIAL(info) << "�ﵽ����ʱ��";
			stop_flag = true;
		}

		void test_thread(std::vector<ChipRequestPtr>& request_list, Seq& request_seq,bool& stop_flag,bool dynamic_request, boost::atomic_int32_t& running_threads) {
			unsigned int count = 0;
			int seq = -1;
			BOOST_LOG_TRIVIAL(debug)  << "�߳����� ";
			while (!stop_flag){
				if (dynamic_request || seq == -1) {
					seq = request_seq.next();
				}
				BOOST_LOG_TRIVIAL(debug)  << " ������ :"<< seq;
				ChipRequestPtr request = request_list[seq];

				{
					boost::unique_lock<boost::mutex> lock(request->get_mutex());
					while (!request->io_ready()  ) {
						//BOOST_LOG_TRIVIAL(debug) << "wait io ready";
						if (request->io_condition().timed_wait(lock, boost::posix_time::milliseconds(1000))) {
							request->io_ready() = false;
						}
						if (stop_flag) {break;}
					}
				}

				if (stop_flag) {
					break;
				}
				unsigned char data[IO_BUFF_SIZE];
				memcpy(data, request->get_buffer(), request->buffer_bytes());
				std::string req_hex = ssx::utils::encode_hex(data, 16, true);
				//BOOST_LOG_TRIVIAL(debug)  << "start write ";
				int io_bytes = ssx::io::win32::exchange_data(request->get_handle(),data, IO_BUFF_SIZE);
				
				{
					boost::unique_lock<boost::mutex> lock(request->get_mutex());
					request->io_ready() = true;
					request->io_condition().notify_all();
				}

				const  int MIN_RESPONSE_BYTES = 9;
				if (io_bytes <= 0) {
					BOOST_LOG_TRIVIAL(error) << "io����,write�����ֽ�: " << io_bytes ;
					break;
				}
				std::string rsp_hex = ssx::utils::encode_hex(data, io_bytes, true);
				if (io_bytes < MIN_RESPONSE_BYTES) {
					BOOST_LOG_TRIVIAL(error) << "Э�����,write�����ֽ�: " << io_bytes<<"̫��, ��������С��Ӧ����.��Ӧ����:"<< rsp_hex ;
					break;
				}
				if (0 != memcmp(data, request->get_buffer(), 4) ) {
					BOOST_LOG_TRIVIAL(error) << "Э�����,4�ֽڱ����岻ƥ��,����:"<< req_hex<<".��Ӧ:" << rsp_hex ;
					break;
				}
				int request_cmd = request->get_buffer()[4];
				int response_cmd = data[4];
				if (response_cmd  != (request_cmd + 0x20 )) {
					BOOST_LOG_TRIVIAL(error) << "Э�����,��Ӧ��������������ƥ��,����:" << req_hex << ".��Ӧ:" << rsp_hex;
					break;
				}
				int msg_bytes = data[5] + 8;
				if (msg_bytes != io_bytes) {
					BOOST_LOG_TRIVIAL(error) << "Э�����,������ʵ�ʳ�����write�����ֽڲ�ƥ��,io_bytes = "<< io_bytes<< ", msg_bytes ="<< msg_bytes <<   ".����:" << req_hex << ".��Ӧ:" << rsp_hex;
					break;
				}

				count++;
				if ( (count % 1000) == 0) {
					BOOST_LOG_TRIVIAL(info) << "����ͨ��"<< count<<"��";
				}
				// FIXME: CRC CHECK
			}
			running_threads--;
			BOOST_LOG_TRIVIAL(info) << "�߳��˳�.������ͨ������ " << count;
			if (running_threads <= 0 && !stop_flag) {
				BOOST_LOG_TRIVIAL(info) << "���ڲ��Գ���,���в����̶߳�����ǰ�˳�.";
			}
		}
	}
}