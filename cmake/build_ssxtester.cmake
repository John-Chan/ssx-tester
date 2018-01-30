set(ssx_io_mt_tester_src_files
   ${ssx_src_DIR}/ssx10/protocol/protocol_helper.h
   ${ssx_src_DIR}/ssx10/protocol/protocol_helper.cpp
   ${ssx_src_DIR}/ssx10/io/win32/ssx_io.h
   ${ssx_src_DIR}/ssx10/io/win32/ssx_io.cpp
   ${ssx_src_DIR}/ssx10/utils/hex.h
   ${ssx_src_DIR}/ssx10/utils/hex.cpp
   ${ssx_src_DIR}/ssx10/test/test_helper.h
   ${ssx_src_DIR}/ssx10/test/test_helper.cpp
   ${ssx_src_DIR}/ssx10/test/io_mt_test.h
   ${ssx_src_DIR}/ssx10/test/io_mt_test.cpp
   ${ssx_src_DIR}/ssx10/test/ssx_io_mt_test.cpp
 )

add_executable(ssx_io_mt_test ${ssx_io_mt_tester_src_files})
if (MSVC)
  add_definitions( -D_WIN32_WINNT=0x0501 )
  target_link_libraries(ssx_io_mt_test Setupapi)
endif (MSVC)
target_link_libraries(ssx_io_mt_test ${Boost_LIBRARIES})

set(ssx_io_st_tester_src_files
   ${ssx_src_DIR}/ssx10/protocol/protocol_helper.h
   ${ssx_src_DIR}/ssx10/protocol/protocol_helper.cpp
   ${ssx_src_DIR}/ssx10/io/win32/ssx_io.h
   ${ssx_src_DIR}/ssx10/io/win32/ssx_io.cpp
   ${ssx_src_DIR}/ssx10/utils/hex.h
   ${ssx_src_DIR}/ssx10/utils/hex.cpp
   ${ssx_src_DIR}/ssx10/test/test_helper.h
   ${ssx_src_DIR}/ssx10/test/test_helper.cpp
   ${ssx_src_DIR}/ssx10/test/io_st_test.h
   ${ssx_src_DIR}/ssx10/test/io_st_test.cpp
   ${ssx_src_DIR}/ssx10/test/ssx_io_st_test.cpp
 )

add_executable(ssx_io_st_test ${ssx_io_st_tester_src_files})
if (MSVC)
  add_definitions( -D_WIN32_WINNT=0x0501 )
  target_link_libraries(ssx_io_st_test Setupapi)
endif (MSVC)
target_link_libraries(ssx_io_st_test ${Boost_LIBRARIES})
