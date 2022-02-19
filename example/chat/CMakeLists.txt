cmake_minimum_required(VERSION 3.18)
project(example_chat_server)

add_executable(example_chat_server chat_server.cpp)
target_link_libraries(example_chat_server netpp pthread)
