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
        for(std::size_t i = 0;i<num_pool; ++i)
        {
            auto io = io_context_vec_.emplace_back(std::make_shared<io_context>());
            work_guard_vec_.emplace_back(std::make_shared<work_guard_type>(boost::asio::make_work_gurad(*io)));
        }
    }

    void stop() 
    {
        for(auto &io :io_context_vec_)
        {
            io->stop();
        }
    }

    void run()
    {
        std::vector<std::shared_ptr<std::thread> > threads;
        for(auto& io: io_context_vec_)
        {
            threads.emplace_back(std::make_shared(
                [&io]()
                {
                    io->run();
                }
            ));
        }
    }


    io_context &get_io_context()
    {
        auto & io = *io_context_vec_.at(current_io_context_);
        ++current_io_context_;
        if(current_io_context_ == io_context_vec_.size())
        {
            current_io_context_ = 0;
        }
        return io;
    }

  private:
    using io_context_ptr = std::shared_ptr<io_context>;
    using work_guard_type = boost::asio::executor_work_guard<io_context::executor_type>;
    using work_guard_ptr = std::shared_ptr<work_guard_type>;
    
    std::vector<io_context_ptr>  io_context_vec_;
    std::vector<work_guard_ptr>  work_guard_vec_;
    std::size_t current_io_context_ = {0};
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