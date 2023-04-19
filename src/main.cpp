#include <iostream>
#include "stack_container.h"

int main(int argc, char** argv){
    // 用法参考
    StackVector<int, 16> stack_vec;
    for(int i = 0; i < 16; ++i){
        stack_vec->push_back(i);
    }

    // TODO 这里用法和正常vector的用法不一致，因此直接搬运后还需要做一定的修改，以符合常规vector使用原则。
    for(const auto& val : stack_vec.container())
        std::cout << val << std::endl;

    return 0;
}