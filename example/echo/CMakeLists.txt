cmake_minimum_required(VERSION 3.18)
project(echo)

foreach(_target
        echo_client echo_server echo_server_auto_kick)
    add_executable(example_${_target} "${_target}.cpp")
    target_link_libraries(example_${_target} netpp pthread)
endforeach()
