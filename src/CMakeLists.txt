find_package(Boost 1.68 
    REQUIRED
    COMPONENTS
    system
)

set(asio_src main.cpp
             generic_service.hpp
             generic_session.hpp
             io_context_pool.hpp)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(simple_asio ${asio_src})

    target_compile_options(simple_asio PUBLIC -std=c++17 -Wall -Wextra)
    target_compile_definitions(simple_asio PUBLIC BOOST_ASIO_NO_DEPRECATED)
    target_link_libraries(simple_asio Boost::system ws2_32 wsock32)
endif()