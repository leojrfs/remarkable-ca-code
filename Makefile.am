AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/include
bin_PROGRAMS = hello_world
hello_world_SOURCES = src/hello_world.cpp include/hello_world.h src/main.cpp
check_PROGRAMS = test_hello_world
test_hello_world_SOURCES = tests/test_hello_world.cpp src/hello_world.cpp include/hello_world.h
TESTS = $(check_PROGRAMS)
