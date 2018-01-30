#ifndef SSX_TEST_CHIP_REQUEST_H
#define SSX_TEST_CHIP_REQUEST_H

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include "../io/win32/ssx_io.h"

namespace ssx {
	namespace testing {

		static const unsigned int IO_BUFF_SIZE = 512;

		static const int chip_a_max_count = 8;
		static const int chip_b_max_count = 32;

		class ChipRequest {
		private:
			boost::mutex mutex_;
			//boost::condition_variable io_condition_;
			unsigned char buffer_[IO_BUFF_SIZE];
			HANDLE fd_;
			//boost::atomic_bool io_ready_;
		public:
			ChipRequest(HANDLE handle)
				:fd_(handle)/*, io_ready_(true) */{

			}
			unsigned char* get_buffer() { return buffer_; }
			boost::mutex& get_mutex() { return mutex_; }
			size_t buffer_bytes() { return sizeof(buffer_); }
			/*
			boost::atomic_bool& io_ready() { return io_ready_; }
			boost::condition_variable& io_condition() {
				return io_condition_;
			}
			*/
			HANDLE get_handle() { return fd_; }
		};



		class Seq
		{
		public:
			Seq(int min, int max)
				:min_(min), max_(max) {
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

		bool check_response_data(const void* req_buffer,const void* rep_buffer, unsigned int rep_bytes);
		void print_test_samples(const std::string& title,const std::vector<ChipRequestPtr>& request_list);
	}
}

#endif