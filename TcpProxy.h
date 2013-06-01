#ifndef TCP_PROXY_H
#define TCP_PROXY_H

#include <memory>
#include "IoServicePool.h"

namespace asioproxy
{

class TcpProxy
{
public:

  typedef std::shared_ptr<TcpProxy> SharedPtr;

  static SharedPtr create();

  virtual ~TcpProxy();

  void run();

private:

  TcpProxy();

  TcpProxy(const TcpProxy& rhs) = delete;

  TcpProxy& operator=(const TcpProxy& rhs) = delete;

  void logStartupInfo();

  void createAcceptors();

  void runIoThreads();

  IoServicePool m_ioServicePool;

};

}

#endif
