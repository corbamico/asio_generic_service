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
    boost::asio::ip::tcp::socket &socket() { return socket_; }

    chat_session(boost::asio::io_context &io)
        : io_(io), socket_(io), write_strand_(io) {}

    void start() { read_packet(); }
    void send(std::string msg)
    {
        boost::asio::post(io_,
                          bind_executor(write_strand_,
                                        [self = shared_from_this(), msg]() {
                                            self->queue_message(msg);
                                        }));
    }

  private:
    void read_packet()
    {
        boost::asio::async_read_until(socket_, in_packet_, '\n',
                                      [self = shared_from_this()](const boost::system::error_code &ec, std::size_t bytes_read) {
                                          self->read_packet_done(ec, bytes_read);
                                      });
    }
    void read_packet_done(const boost::system::error_code &ec, std::size_t bytes_read)
    {
        boost::ignore_unused(bytes_read);
        if (!ec)
        {
            std::istream stream(&in_packet_);
            std::string  packet_string;
            stream >> packet_string;
            //do something with packet_string;
            send(packet_string);
            read_packet();
        }
    }
    void queue_message(std::string msg)
    {
        bool write_in_progress = !send_packet_queue_.empty();
        send_packet_queue_.emplace_back(std::move(msg));
        if (!write_in_progress)
        {
            start_packet_send();
        }
    }
    void start_packet_send()
    {
        send_packet_queue_.front() += "\0";
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(send_packet_queue_.front()),
                                 boost::asio::bind_executor(
                                   write_strand_,
                                   [self = shared_from_this()](const boost::system::error_code& ec,std::size_t){
                                     self->packet_send_done(ec);
                                   }
                                 ));                                 
    }
    void packet_send_done(const boost::system::error_code& ec)
    {
      if(!ec)
      {
        send_packet_queue_.pop_front();
        if(!send_packet_queue_.empty())
        {
          start_packet_send();
        }
      }
    }

  private:
    boost::asio::io_context &       io_;
    boost::asio::ip::tcp::socket    socket_;
    boost::asio::io_context::strand write_strand_;
    boost::asio::streambuf          in_packet_;
    std::deque<std::string>         send_packet_queue_;
};

#endif //GENERIC_SESSION_HPP
