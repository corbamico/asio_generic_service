#ifndef GENERIC_SERVICE_HPP
#define GENERIC_SERVICE_HPP

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <vector>
#include <iostream>

#include "io_context_pool.hpp"

class signal_handle
{
  public:
    signal_handle(io_context_pool &pool) : signal_(pool.get_io_context())
    {
        signal_.add(SIGINT);
        signal_.add(SIGTERM);
        signal_.async_wait([&pool](const boost::system::error_code &, int) { pool.stop(); });
    }

  private:
    boost::asio::signal_set signal_;
};

template <typename Session>
    class asio_generic_server final : private boost::noncopyable
{
    using shared_handler_t = std::shared_ptr<Session>;
    using tcp = boost::asio::ip::tcp;

  public:
    asio_generic_server(std::size_t num_pool, uint16_t port)
        : io_pool_(num_pool), acceptor_(io_pool_.get_io_context(), tcp::endpoint(tcp::v4(), port)), signal_(io_pool_)
    {
        do_accept();
    }
    void run() { io_pool_.run(); }

  private:
    void handle_connect(shared_handler_t handler, const boost::system::error_code &ec)
    {
        //std::cerr << ec.message() << "\n";
        if (!ec)
        {
            handler->start();
            do_accept();
        }
    }
    void do_accept()
    {
        auto handler = std::make_shared<Session>(io_pool_.get_io_context());
        acceptor_.async_accept(handler->socket(),
                              [this, handler](auto ec) { this->handle_connect(handler, ec); });
    }

  private:
    io_context_pool io_pool_;
    
    tcp::acceptor   acceptor_;
    signal_handle   signal_;
};

#endif // GENERIC_SERVICE_HPP
