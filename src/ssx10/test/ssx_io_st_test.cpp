#include "io_st_test.h"
#include <stdlib.h>  
#include <stdio.h>  

int main(int argc, char **argv) {
	if (argc < 3) {
		ssx::testing::print_run_io_st_test_help();
		return -1;
	}
	int  test_second = atoi(argv[1]);
	if (test_second <= 0) {
		test_second = 60;
	}


	bool  dynamic_req = false;
	if (argv[2][0] == 'Y' || argv[2][0] == 'y') {
		dynamic_req = true;
	}

	ssx::testing::run_io_st_test(test_second, dynamic_req);
	return 0;
}