
1 go cmake dir
        $ cd [path_to_project]/cmake
2 make dir for build
        $ mkdir build
        $ cd build
3 generate vs project files

        $ cmake -G "Visual Studio 9 2008" ..
	
	for vs2015:
        $ cmake -G "Visual Studio 14 2015" ..
	see cmake-generaters.txt


install headers:
run [path_to_project]\cmake\build\extract_includes.bat
you will see a 'include' directory

============================================================================
set boost library path manually

you need set two environment variable before run cmake:
BOOST_INCLUDEDIR : the boost header file dir,it should contian a subdir named 'boost' 
BOOST_LIBRARYDIR : the boost libaray files dir(contian .lib,.dll files)
e.g. for window
  set BOOST_INCLUDEDIR=C:\boost\src\boost_1_59_0
  set BOOST_LIBRARYDIR=C:\boost\boost_1_59_0-bin-msvc-all-32-64\boost_1_59_0\lib64-msvc-14.0
for linux
  export BOOST_INCLUDEDIR=/home/cj/boost_1_59_0
  export BOOST_LIBRARYDIR=/home/cj/boost-lib64
