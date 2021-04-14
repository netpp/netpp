cmake_minimum_required(VERSION 3.16)
project(chat)

add_executable(chat_server chat_server.cpp)
target_link_libraries(chat_server netpp pthread)
