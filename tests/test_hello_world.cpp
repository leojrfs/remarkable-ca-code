#include <iostream>
#include <cassert>

#include "hello_world.h"

/* This is for creating a failing test for testing the test infrastructure */
#ifndef FAIL_COMPARISON_STR
#define FAIL_COMPARISON_STR ""
#endif

#define PASS_COMPARSION_STR "hello world"

int main() {
    auto ret_string = getHelloWorld();
    if(0 == ret_string.compare(PASS_COMPARSION_STR FAIL_COMPARISON_STR)) {
        std::cout << "PASS: " << ret_string << " = " << PASS_COMPARSION_STR << std::endl;
    } else {
        std::cout << "FAIL: " << ret_string << " != " << PASS_COMPARSION_STR << std::endl;
        return 1;
    }
}
