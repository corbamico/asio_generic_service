#ifndef GENERIC_SESSION_HPP
#define GENERIC_SESSION_HPP

#include <boost/asio.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/core/noncopyable.hpp>
#include <deque>

class chat_session
    : public std::enable_shared_from_this<chat_session>,
      private boost::noncopyable
{
  public:
    chat_session(boost::asio::io_context &io)
        : io_(io), socket_(io), write_strand_(io) {}

    void start() { read_packet(); }
    void read_packet(){}

    boost::asio::ip::tcp::socket &socket() { return socket_; }

  private:
    boost::asio::io_context &       io_;
    boost::asio::ip::tcp::socket    socket_;
    boost::asio::io_context::strand write_strand_;
    boost::asio::streambuf          in_packet_;
    std::deque<std::string>         send_packet_queue_;
};

#endif //GENERIC_SESSION_HPP
