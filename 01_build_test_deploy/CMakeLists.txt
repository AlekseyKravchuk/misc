cmake_minimum_required(VERSION 3.28)

# Переменные создаются через функцию "set"
# PATCH_VERSION назначаем значение по умолчанию равное "1".
# Переменная "PATCH_VERSION" будет передаваться снаружи через файл "release.yml":
#   - run: cmake . -DPATCH_VERSION=${{ github.run_number }}
#         , где github.run_number - это порядковый номер build'a в github'e
# CACHE INTERNAL позволяет управлять видимостью переменных в кеше.
set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VERSION 0.0.${PATCH_VERSION})

# Вывести значение некоторой переменной на консоль (для Debug'a):
# set(SOME_VARIABLE 0)
# message("SOME_VARIABLE = ${SOME_VARIABLE}")

# Делаем версионирование нашего проекта
project(helloworld VERSION ${PROJECT_VERSION})

# Наш проект зависит от библиотеки Boost и это нужно явно указать.
# Т.о. если у пользователя не установлена boost требуемой версии, он получит ошибку от cmake
# В функцию "find_package" передаем имя библиотеки "Boost" и перечисляем компоненты (COMPONENTS), которые нам нужны.
# В данном случае это "unit_test_framework" (но далеко не все библиотеки разделены на компоненты).
find_package(Boost 1.86 REQUIRED
                        COMPONENTS unit_test_framework REQUIRED
)

# Выполняем конфигурирование файла "version.h.in" и результат конфигурирования сохраняем в файл "version.h"
# CMake позволяет конфигурировать исходники на очень базовом уровне
configure_file(version.h.in version.h)

# ========================== Собственно описание проекта ==========================
set(SOURCES
        main.cpp)

set(HELLOWORLD_LIB_SOURCES
        lib.cpp)

set(TESTS_SOURCES
        test_version.cpp)

#set(HEADERS
#       XXXX.h)

# Собрать исполняемыей файл с именем "helloworld_cli" из списка исходников "SOURCES"
add_executable(helloworld_cli ${SOURCES})

# Собрать библиотеку с именем "helloworld" (по умолчанию статическая библиотека) из списка исходников "HELLOWORLD_LIB_SOURCES"
add_library(helloworld ${HELLOWORLD_LIB_SOURCES})

# Собрать исполняемыей файл с именем "test_version" (unit-test) из списка исходников "TESTS_SOURCES".
# Unit-тесты практически всегда делаются отдельным исполняемым файлом
add_executable(test_version ${TESTS_SOURCES})
# =================================================================================

# Через функцию "set_target_properties" можно для каждого target'a (цели сборки) по отдельности или, как в данном случае
# для всех 3-х целей сборки установить требуемые свойства:
set_target_properties(helloworld_cli helloworld test_version PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

# К target'у (цели сборки) можно добавить путь поиска заголовочных файлов (дополнительно к списку стандартных путей)
# В данном случае это текущая билдовая директория ("build/"), т.е. та директория, из которой был запущен CMake
# У CMake'а есть встроенная переменная "CMAKE_BINARY_DIR", в которой хранится полный путь к этой директории
target_include_directories(helloworld
    PRIVATE "${CMAKE_BINARY_DIR}"
)

# Для работы Boost-теста нужен такой дефайн: "BOOST_TEST_DYN_LINK" - он нужен, чтобы Boost искал shared-библиотеки (*.so)
set_target_properties(test_version PROPERTIES
    COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
    INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
)

# Смысл функции set_target_properties в следующем. Теперь при компиляции исходников мы попадем в блок #ifdef ...
#ifdef BOOST_TEST_DYN_LINK
# ...
#endif

# функция "target_link_libraries" позволяет сказать, что таргет "helloworld_cli" зависит от библиотеки "helloworld"
 target_link_libraries(helloworld_cli PRIVATE
    helloworld
)

# говорим, что таргет "test_version" зависит от 2-х библиотек: "Boost_LIBRARIES" И "helloworld"
target_link_libraries(test_version
        ${Boost_LIBRARIES}
        helloworld
)

# CMake позволяет передавать опции компиляторам. Они являются platform-specific.
if (MSVC)
    target_compile_options(helloworld_cli PRIVATE
        /W4
    )
    target_compile_options(helloworld PRIVATE
        /W4
    )
    target_compile_options(test_version PRIVATE
        /W4
    )
else ()
    target_compile_options(helloworld_cli PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    target_compile_options(helloworld PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    target_compile_options(test_version PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
endif()

# ========================== Процедура формирования пакета ==========================
# В результате сборки наших целей мы получим следующие 3 файла:
# 1) исполняемый файл "helloworld_cli"    (!!! единственный файл, который нужен заказчику !!!)
# 2) статическую библиотеку "helloworld"  (заказчику в виде отдельного файла не нужен, т.к. это статическая либа)
# 3) исполняемый файл с unit-тестом "test_version" (тоже не нужен заказчику)

# С помощью функции "install" говорим установить всё, что привязано к таргету "helloworld_cli" в
install(TARGETS helloworld_cli
        RUNTIME
            DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)

# Подключаем вспомогательный модуль "CPack", который содержит в себе генератор Debian-пакетов
include(CPack)

# Делаем возможным регистрировать некоторые бинарники в качестве unit-тестов
enable_testing()

# В Linux тесты нужно будет запустить так:   ./test_version
# В Windows тесты нужно будет запустить так: test_version.exe
# CMake позволяет зарегистрировать бинарники, которые ему известны, в качестве теста
add_test(test_version test_version)

# Благодаря "enable_testing" и "add_test" в файле release.yml можно прописать запуск unit-тестов:
# - run: cmake --build . --target test
