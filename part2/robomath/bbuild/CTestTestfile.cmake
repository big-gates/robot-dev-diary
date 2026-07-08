# CMake generated Testfile for 
# Source directory: /ws/part2/robomath
# Build directory: /ws/part2/robomath/bbuild
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_angles "/ws/part2/robomath/bbuild/test_angles")
set_tests_properties(test_angles PROPERTIES  _BACKTRACE_TRIPLES "/ws/part2/robomath/CMakeLists.txt;27;add_test;/ws/part2/robomath/CMakeLists.txt;0;")
subdirs("_deps/googletest-build")
