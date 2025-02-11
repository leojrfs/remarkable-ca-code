#include <iostream>
#include <cassert>

#include "hello_world.h"
 
int main()
{
    assert(getHelloWorld() == "hello world");
    std::cout << "getHelloWorld() works\n";
}