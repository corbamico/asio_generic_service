#include "generic_service.hpp"
#include "generic_session.hpp"

int main()
{
    asio_generic_server<chat_session> server(std::thread::hardware_concurrency(),12345);
    server.run();
    return 0;
}