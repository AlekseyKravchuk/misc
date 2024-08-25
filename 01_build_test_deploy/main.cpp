#include "lib.h"

#include <iostream>

/* Эта программа должна:
 * 1) собраться, протестироваться на github'e через систему "Github Actions" (настроить запуск Actions по коммиту);
 * 2) после успешной сборки это всё должно упаковаться в релиз там же на github'e
 * */
int main(int, char**) {
    std::cout << "Version: "     << version() << std::endl;
    std::cout << "Hello, world!" << std::endl;

    std::cout << "Added some changes to test my Github Actions" << std::endl;

    return 0;
}
