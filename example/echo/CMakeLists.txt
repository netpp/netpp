cmake_minimum_required(VERSION 3.16)
project(echo)

foreach(_target
        echo_client echo_server echo_server_auto_kick)
    add_executable(${_target} "${_target}.cpp")
    target_link_libraries(${_target} netpp pthread)
endforeach()
