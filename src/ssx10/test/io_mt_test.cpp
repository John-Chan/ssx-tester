#include "io_mt_test.h"

#include <list>
#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/asio.hpp> // for timer

#include "../io/win32/ssx_io.h"
#include "../utils/hex.h"
#include "test_helper.h"


namespace ssx {
	namespace testing {
		static std::string packet_template_chip_b_0x43 = "01000500431000000000000003404883122957145088f211";
		static std::string packet_template_chip_a_0x54 = "000002005499001ec4f4225d888d03cdea4b52506a5601000000000000000000000000000000000000000000034048831229571450883120170618000001230000000000000999b0543d90b1000000a5bb08043e964eec635828ea53a0386fad0ac82e3b616ef084851956709730f9045269aaacab6e517ab49abe5624cb24be631e75c7f5f6d94b64b418e5d11c52ab5faa270f66503398df693fd51b65a437a6";
		


		void handle_timer(const boost::system::error_code& error, bool& stop_flag);
		void test_thread(std::vector<ChipRequestPtr>& request_list, Seq& request_seq, bool& stop_flag, bool dynamic_request, boost::atomic_int32_t& running_threads);
		void print_run_io_mt_test_help() {
			std::cout << " ================================================================= " << std::endl;
			std::cout << "本程序的测试流程:程序预先生成若干测试样本数据(但保证每个芯片只有一个样本),然后在线程池中执行IO请求,检查响应数据是否符合预期结果" << std::endl;
			std::cout << "命令行:ssx_io_mt_test <second> <threads> <dynamic_req> " << std::endl;
			std::cout << "参数 second :最大测试时间,单位秒" << std::endl;
			std::cout << "参数 threads 测试线程池,如果出错会减少测试线程数量. " << std::endl;
			std::cout << "参数 dynamic_req :是否允许动态请求采样, y表示开启.如果关闭,每一个测试线程总是发送相同的数据" << std::endl;
			std::cout << "例子:ssxtester 60 40 y " << std::endl;
			std::cout << " ================================================================= " << std::endl;
		}
		void run_io_mt_test(unsigned int seconds,int thread_size,bool dynamic_req) {
			
			BOOST_LOG_TRIVIAL(info) << "准备测试, second = "<< seconds <<",threads = "<< thread_size<< ",dynamic_req = "<< (dynamic_req?"Y":"N" );
#if (IO_WITH_TIMEOUT)
			BOOST_LOG_TRIVIAL(info) << "IO 模型:阻塞+超时";
#else
			BOOST_LOG_TRIVIAL(info) << "IO 模型:阻塞";
#endif

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

			const int chip_a_count = chip_a_max_count;
			const int chip_b_count = chip_b_max_count;
			if (thread_size <= 0) {
				thread_size = chip_a_count + chip_b_count;
			}
			bool stop_test = false;
			Seq request_seq(0, chip_b_count + chip_a_count);
			std::vector<ChipRequestPtr> request_list;
			for (int device_index = 0; device_index < device_handle_list.size(); ++device_index) {
				for (int chip_idx = 0; chip_idx < chip_b_count; ++chip_idx) {
					ChipRequestPtr cr(new ChipRequest(device_handle_list[device_index]->getHandle()));
					memset(cr->get_buffer(), 0x0, cr->buffer_bytes());
					ssx::utils::decode_hex(packet_template_chip_b_0x43, cr->get_buffer(), cr->buffer_bytes());
					cr->get_buffer()[2] = (unsigned char)chip_idx;
					request_list.push_back(cr);
				}
			}
			for (int device_index = 0; device_index < device_handle_list.size(); ++device_index) {
				for (int chip_idx = 0; chip_idx < chip_a_count; ++chip_idx) {
					ChipRequestPtr cr(new ChipRequest(device_handle_list[device_index]->getHandle()));
					memset(cr->get_buffer(), 0x0, cr->buffer_bytes());
					ssx::utils::decode_hex(packet_template_chip_a_0x54, cr->get_buffer(), cr->buffer_bytes());
					cr->get_buffer()[2] = (unsigned char)chip_idx;
					request_list.push_back(cr);
				}
			}

			print_test_samples("测试样本数据", request_list);

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
			BOOST_LOG_TRIVIAL(info) << "等待测试结束: "<< seconds<<" s";
			t.async_wait(boost::bind(handle_timer,
				boost::asio::placeholders::error,
				boost::ref(stop_test)));
			io.run();
			BOOST_LOG_TRIVIAL(info) << "等待测试线程退出 ";
			test_thread_group.join_all();
		}

		void handle_timer(const boost::system::error_code& error, bool& stop_flag) {
			BOOST_LOG_TRIVIAL(info) << "达到测试时限";
			stop_flag = true;
		}

		void test_thread(std::vector<ChipRequestPtr>& request_list, Seq& request_seq,bool& stop_flag,bool dynamic_request, boost::atomic_int32_t& running_threads) {
			unsigned int count = 0;
			int seq = -1;
			BOOST_LOG_TRIVIAL(debug)  << "线程启动 ";
			while (!stop_flag){
				if (dynamic_request || seq == -1) {
					seq = request_seq.next();
				}
				BOOST_LOG_TRIVIAL(debug)  << " 使用请求编号 :"<< seq;
				ChipRequestPtr request = request_list[seq];
				int io_bytes = -1;
				unsigned char data[IO_BUFF_SIZE];
				
				{
					boost::unique_lock<boost::mutex> lock(request->get_mutex());
					memcpy(data, request->get_buffer(), request->buffer_bytes());
					//BOOST_LOG_TRIVIAL(debug)  << "start write ";
#if (IO_WITH_TIMEOUT)
					io_bytes = ssx::io::win32::exchange_data(request->get_handle(), data, IO_BUFF_SIZE, 2000);
#else
					io_bytes = ssx::io::win32::exchange_data(request->get_handle(), data, IO_BUFF_SIZE);
#endif
				}

			
				

				if (io_bytes <= 0) {
					BOOST_LOG_TRIVIAL(error) << "io错误,write返回字节: " << io_bytes ;
					break;
				}
				if (!check_response_data(request->get_buffer(), data,io_bytes)) {
					break;
				}

				count++;
				if ( (count % 1000) == 0) {
					BOOST_LOG_TRIVIAL(info) << "当前线程测试通过"<< count<<"次";
				}
			}
			running_threads--;
			BOOST_LOG_TRIVIAL(info) << "线程退出.当前线程测试通过次数 " << count;
			if (running_threads <= 0 && !stop_flag) {
				BOOST_LOG_TRIVIAL(info) << "由于测试出错,所有测试线程都已提前退出.";
			}
		}
	}
}