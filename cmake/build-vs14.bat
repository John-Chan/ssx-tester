mkdir build
cd build
set BOOST_INCLUDEDIR=C:\boost\src\boost_1_59_0
set BOOST_LIBRARYDIR=C:\boost\boost_1_59_0-bin-msvc-all-32-64\boost_1_59_0\lib32-msvc-14.0
cmake -G "Visual Studio 14 2015" ..

pause