#ifndef SSX_TEST_IO_MT_TEST_H
#define SSX_TEST_IO_MT_TEST_H

namespace ssx {
	namespace testing {
		void run_io_mt_test(unsigned int seconds, int thread_size, bool dynamic_req);
		void print_run_io_mt_test_help();
	}
}

#endif