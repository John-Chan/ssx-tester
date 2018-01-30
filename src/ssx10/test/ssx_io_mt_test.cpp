#include "io_mt_test.h"
#include <stdlib.h>  
#include <stdio.h>  



int main(int argc, char **argv) {

	if (argc < 4) {
		ssx::testing::print_run_io_mt_test_help();
		return -1;
	}
	int  test_second = atoi(argv[1]);
	if (test_second <= 0) {
		test_second = 60;
	}

	int  test_threads = atoi(argv[2]);
	if (test_threads < 0) {
		test_threads = 0;
	}

	bool  dynamic_req = false;
	if (argv[3][0] == 'Y' || argv[3][0] == 'y' ) {
		dynamic_req = true;
	}

	ssx::testing::run_io_mt_test(test_second, test_threads, dynamic_req);
	return 0;
}
