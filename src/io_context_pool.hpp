#ifndef IO_CONTEXT_POOL_HPP
#define IO_CONTEXT_POOL_HPP
#include <boost/asio.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/core/noncopyable.hpp>
#include <thread>
#include <vector>

class single_policy
{
    using io_context = boost::asio::io_context;

  protected:
    single_policy() = default;

  public:
    void init(std::size_t num_threads)
    {
        boost::ignore_unused(num_threads);
    }
    void run()
    {
        io_context_.run();
    }
    void stop()
    {
        io_context_.stop();
    }

  private:
    io_context io_context_;
};

class single_io_multithread_policy
{
    using io_context = boost::asio::io_context;

  protected:
    single_io_multithread_policy() = default;

  public:
    void init(std::size_t num_threads)
    {
        num_threads_ = num_threads;
    }
    void run()
    {
        std::vector<std::shared_ptr<std::thread>> threads;
        for (std::size_t i = 0; i < num_threads_; ++i)
        {
            threads.emplace_back(std::make_shared<std::thread>(
                [this]() {
                    this->io_context_.run();
                }));
        }

        for (auto &&t : threads)
        {
            t->join();
        }
    }
    void stop()
    {
        io_context_.stop();
    }
    io_context &get_io_context()
    {
        return io_context_;
    }

  private:
    std::size_t num_threads_ = {0};
    io_context  io_context_;
};

class multi_io_multithread_policy
{
    using io_context = boost::asio::io_context;

  protected:
    multi_io_multithread_policy() = default;

  public:
    void init(std::size_t num_pool)
    {
    }

    void stop() {}

    void        run() {}
    io_context &get_io_context() {}

  private:
};

template <class pool_policy>
class io_context_pool_base
    : public pool_policy,
      private boost::noncopyable
{
  public:
    io_context_pool_base(std::size_t pool_size)
    {
        pool_policy::init(pool_size);
    }
};

using io_context_single = io_context_pool_base<single_policy>;
using io_context_threads = io_context_pool_base<single_io_multithread_policy>;
using io_context_pool = io_context_pool_base<multi_io_multithread_policy>;

#endif