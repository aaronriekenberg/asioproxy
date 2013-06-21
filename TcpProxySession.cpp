#include "BoostSystemUtil.h"
#include "Log.h"
#include "TcpProxySession.h"
#include "ProxyOptions.h"

namespace asioproxy
{

TcpProxySession::SharedPtr
TcpProxySession::create(
  boost::asio::io_service& ioService)
{
  return SharedPtr(
    new TcpProxySession(ioService));
}

TcpProxySession::~TcpProxySession()
{
  if (Log::isDebugEnabled())
  {
    Log::getDebugInstance() << "TcpProxySession::~TcpProxySession " << this;
  }
  terminate();
}

boost::asio::ip::tcp::socket& TcpProxySession::getClientSocket()
{
  return m_clientSocket;
}

void TcpProxySession::handleClientSocketAccepted()
{
  if (Log::isDebugEnabled())
  {
    Log::getDebugInstance() << "handleClientSocketAccepted";
  }

  if (ProxyOptions::getInstance().isNoDelay())
  {
    boost::asio::ip::tcp::no_delay noDelayOption(true);
    m_clientSocket.set_option(noDelayOption);
  }

  std::stringstream clientToProxySS;
  clientToProxySS << m_clientSocket.remote_endpoint()
                  << " -> "
                  << m_clientSocket.local_endpoint();
  m_clientToProxyString = clientToProxySS.str();

  Log::getInfoInstance()
    << "connected client to proxy "
    << m_clientToProxyString;

  const ProxyOptions::AddressAndPort& remoteAddressPort =
    ProxyOptions::getInstance().getRemoteAddressPort();

  Log::getInfoInstance() 
    << "begin resolving "
    << std::get<0>(remoteAddressPort)
    << ":"
    << std::get<1>(remoteAddressPort);

  boost::asio::ip::tcp::resolver::query query(
    std::get<0>(remoteAddressPort),
    std::get<1>(remoteAddressPort));
  auto sharedThis = shared_from_this();
  m_resolver.async_resolve(
    query,
    [=] (const boost::system::error_code& error, 
         boost::asio::ip::tcp::resolver::iterator iterator) 
    { 
      sharedThis->handleRemoteEndpointResolved(error, iterator);
    }
  );
}

TcpProxySession::TcpProxySession(
  boost::asio::io_service& ioService) :
  m_ioService(ioService),
  m_clientSocket(ioService),
  m_resolver(ioService),
  m_remoteSocket(ioService),
  m_connectTimeoutTimer(ioService)
{
  if (Log::isDebugEnabled())
  {
    Log::getDebugInstance() << "TcpProxySession::TcpProxySession " << this;
  }
}

void TcpProxySession::terminate()
{
  if (Log::isDebugEnabled())
  {
    Log::getDebugInstance() << "TcpProxySession::terminate";
  }

  if ((!m_clientSocketClosed) && (!m_writingToClientSocket))
  {
    m_clientSocket.close();
    m_clientSocketClosed = true;
    if (!m_clientToProxyString.empty())
    {
      Log::getInfoInstance()
        << "disconnect client to proxy "
        << m_clientToProxyString;
    }
  }

  if ((!m_remoteSocketClosed) && (!m_writingToRemoteSocket))
  {
    m_remoteSocket.close();
    m_remoteSocketClosed = true;
    if (!m_proxyToRemoteString.empty())
    {
      Log::getInfoInstance()
        << "disconnect proxy to remote "
        << m_proxyToRemoteString;
    }
  }

  m_connectTimeoutTimer.cancel();
}

void TcpProxySession::handleRemoteEndpointResolved(
  const boost::system::error_code& error,
  boost::asio::ip::tcp::resolver::iterator iterator)
{
  if (Log::isDebugEnabled())
  {
    Log::getDebugInstance() << "handleRemoteEndpointResolved";
  }

  if (error)
  {
    const ProxyOptions::AddressAndPort& remoteAddressPort =
      ProxyOptions::getInstance().getRemoteAddressPort();
    Log::getInfoInstance() 
      << "failed to resolve "
      << std::get<0>(remoteAddressPort)
      << ":"
      << std::get<1>(remoteAddressPort);
    terminate();
  }
  else
  {
    boost::asio::ip::tcp::endpoint remoteEndpoint = *iterator;

    Log::getInfoInstance()
      << "begin connect to remote "
      << remoteEndpoint;

    auto sharedThis = shared_from_this();
    m_remoteSocket.async_connect(
      remoteEndpoint,
      [=] (const boost::system::error_code& error) 
      { 
        sharedThis->handleConnectFinished(error);
      });

    m_connectTimeoutTimer.expires_from_now(
      ProxyOptions::getInstance().getConnectTimeout());
    m_connectTimeoutTimer.async_wait(
      [=] (const boost::system::error_code& error)
      {
        sharedThis->handleConnectTimeout();
      });
  }
}

void TcpProxySession::handleConnectTimeout()
{
  if (Log::isDebugEnabled())
  {
    Log::getDebugInstance() << "TcpProxySession::handleConnectTimeout";
  }
  m_connectTimeoutTimerPopped = true;

  if (!m_connectToRemoteFinished)
  {
    Log::getInfoInstance() << "connect timeout";

    terminate();
  }
}

void TcpProxySession::handleConnectFinished(
  const boost::system::error_code& error)
{
  if (Log::isDebugEnabled())
  {
    Log::getDebugInstance() << "TcpProxySession::handleConnectFinished";
  }
  m_connectToRemoteFinished = true;

  if (m_connectTimeoutTimerPopped)
  {
    terminate();
  }
  else if (error)
  {
    Log::getInfoInstance()
      << "error connecting to remote endpoint: "
      << BoostSystemUtil::buildErrorCodeString(error);
    terminate();
  }
  else
  {
    m_connectTimeoutTimer.cancel();

    // allocate buffers
    const size_t bufferSize = ProxyOptions::getInstance().getBufferSize();
    m_dataFromClientBuffer.resize(bufferSize, 0);
    m_dataFromRemoteBuffer.resize(bufferSize, 0);

    if (ProxyOptions::getInstance().isNoDelay())
    {
      boost::asio::ip::tcp::no_delay noDelayOption(true);
      m_remoteSocket.set_option(noDelayOption);
    }

    std::stringstream proxyToRemoteSS;
    proxyToRemoteSS << m_remoteSocket.local_endpoint()
                    << " -> "
                    << m_remoteSocket.remote_endpoint();
    m_proxyToRemoteString = proxyToRemoteSS.str();

    Log::getInfoInstance()
      << "connected proxy to remote "
      << m_proxyToRemoteString;

    asyncReadFromClient();
    asyncReadFromRemote();
  }
}

void TcpProxySession::asyncReadFromClient()
{
  auto sharedThis = shared_from_this();
  m_clientSocket.async_read_some(
    boost::asio::buffer(m_dataFromClientBuffer),
    [=] (const boost::system::error_code& error,
         size_t bytesTransferred)
    {
      sharedThis->handleClientRead(error, bytesTransferred);
    });
}

void TcpProxySession::asyncReadFromRemote()
{
  auto sharedThis = shared_from_this();
  m_remoteSocket.async_read_some(
    boost::asio::buffer(m_dataFromRemoteBuffer),
    [=] (const boost::system::error_code& error,
         size_t bytesTransferred)
    { 
      sharedThis->handleRemoteRead(error, bytesTransferred);
    });
}

void TcpProxySession::handleClientRead(
  const boost::system::error_code& error,
  size_t bytesRead)
{
  if (m_remoteSocketClosed)
  {
    terminate();
  }
  else if (error)
  {
    if (Log::isDebugEnabled())
    {
      Log::getDebugInstance()
        << "TcpProxySession::handleClientRead error = "
        << BoostSystemUtil::buildErrorCodeString(error);
    }
    terminate();
  }
  else
  {
    m_writingToRemoteSocket = true;
    auto sharedThis = shared_from_this();
    boost::asio::async_write(
      m_remoteSocket,
      boost::asio::buffer(m_dataFromClientBuffer,
                          bytesRead),
      [=] (const boost::system::error_code& error,
           size_t bytesWritten)
      {
        sharedThis->handleRemoteWriteFinished(error);
      });
  }
}

void TcpProxySession::handleRemoteRead(
  const boost::system::error_code& error,
  size_t bytesRead)
{
  if (m_clientSocketClosed)
  {
    terminate();
  }
  else if (error)
  {
    if (Log::isDebugEnabled())
    {
      Log::getDebugInstance()
        << "TcpProxySession::handleRemoteRead error = "
        << BoostSystemUtil::buildErrorCodeString(error);
    }
    terminate();
  }
  else
  {
    m_writingToClientSocket = true;
    auto sharedThis = shared_from_this();
    boost::asio::async_write(
      m_clientSocket,
      boost::asio::buffer(m_dataFromRemoteBuffer,
                          bytesRead),
      [=] (const boost::system::error_code& error,
           size_t bytesWritten)
      {
        sharedThis->handleClientWriteFinished(error);
      });
  }
}

void TcpProxySession::handleRemoteWriteFinished(
  const boost::system::error_code& error)
{
  m_writingToRemoteSocket = false;
  if (m_clientSocketClosed)
  {
    terminate();
  }
  else if (error)
  {
    if (Log::isDebugEnabled())
    {
      Log::getDebugInstance()
        << "TcpProxySession::handleRemoteWriteFinished error = "
        << BoostSystemUtil::buildErrorCodeString(error);
    }
    terminate();
  }
  else
  {
    asyncReadFromClient();
  }
}

void TcpProxySession::handleClientWriteFinished(
  const boost::system::error_code& error)
{
  m_writingToClientSocket = false;
  if (m_remoteSocketClosed)
  {
    terminate();
  }
  else if (error)
  {
    if (Log::isDebugEnabled())
    {
      Log::getDebugInstance()
        << "TcpProxySession::handleClientWriteFinished error = "
        << BoostSystemUtil::buildErrorCodeString(error);
    }
    terminate();
  }
  else
  {
    asyncReadFromRemote();
  }
}

}
