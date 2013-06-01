#ifndef IO_SERVICE_POOL_H
#define IO_SERVICE_POOL_H

#include <boost/asio.hpp>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace asioproxy
{

class IoServicePool
{
public:

  explicit IoServicePool(size_t poolSize);

  virtual ~IoServicePool() = default;

  void runIoThreads();

  boost::asio::io_service& getIoService();

private:

  typedef std::shared_ptr<boost::asio::io_service> IoServicePtr;

  typedef std::shared_ptr<boost::asio::io_service::work> WorkPtr;

  typedef std::shared_ptr<std::thread> ThreadPtr;

  IoServicePool(const IoServicePool& rhs) = delete;

  IoServicePool& operator=(const IoServicePool& rhs) = delete;

  std::vector<IoServicePtr> m_ioServiceVector;

  std::vector<WorkPtr> m_workVector;

  size_t m_nextIoServiceIndex = 0;

  std::mutex m_mutex;

};

}

#endif
