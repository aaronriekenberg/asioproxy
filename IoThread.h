#ifndef IO_THREAD_H
#define IO_THREAD_H

#include <boost/asio.hpp>
#include <memory>
#include <thread>

namespace asioproxy
{

class IoThread
{
public:

  static std::shared_ptr<std::thread> create(
    size_t ioThreadNumber,
    boost::asio::io_service& ioService);

  virtual ~IoThread() = default;

  IoThread(const IoThread& rhs) = default;

  void operator()();

private:

  IoThread& operator=(const IoThread& rhs) = delete;

  IoThread(
    size_t ioThreadNumber,
    boost::asio::io_service& ioService);

  const size_t m_ioThreadNumber;

  boost::asio::io_service& m_ioService;

};

}

#endif
