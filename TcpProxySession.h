#ifndef TCP_PROXY_SESSION_H
#define TCP_PROXY_SESSION_H

#include <boost/asio.hpp>
#include <boost/asio/system_timer.hpp>
#include <memory>
#include <vector>

namespace asioproxy
{

class TcpProxySession :
  public std::enable_shared_from_this<TcpProxySession>
{
public:

  typedef std::shared_ptr<TcpProxySession> SharedPtr;

  static SharedPtr create(
    boost::asio::io_service& ioService);

  virtual ~TcpProxySession();

  boost::asio::ip::tcp::socket& getClientSocket();

  void handleClientSocketAccepted();

private:

  TcpProxySession(
    boost::asio::io_service& ioService);

  TcpProxySession(const TcpProxySession& rhs) = delete;

  TcpProxySession& operator=(const TcpProxySession& rhs) = delete;

  void terminate();

  void handleRemoteEndpointResolved(
    const boost::system::error_code& error, 
    boost::asio::ip::tcp::resolver::iterator iterator);

  void handleConnectTimeout();

  void handleConnectFinished(
    const boost::system::error_code& error);

  void asyncReadFromClient();

  void asyncReadFromRemote();

  void handleClientRead(
    const boost::system::error_code& error,
    size_t bytesRead);

  void handleRemoteRead(
    const boost::system::error_code& error,
    size_t bytesRead);

  void handleRemoteWriteFinished(
    const boost::system::error_code& error);

  void handleClientWriteFinished(
    const boost::system::error_code& error);

  boost::asio::io_service& m_ioService;

  boost::asio::ip::tcp::socket m_clientSocket;

  boost::asio::ip::tcp::resolver m_resolver;

  boost::asio::ip::tcp::socket m_remoteSocket;

  boost::asio::system_timer m_connectTimeoutTimer;

  std::vector<unsigned char> m_dataFromClientBuffer;

  std::vector<unsigned char> m_dataFromRemoteBuffer;

  std::string m_clientToProxyString;

  std::string m_proxyToRemoteString;

  bool m_connectToRemoteFinished = false;

  bool m_connectTimeoutTimerPopped = false;

  bool m_writingToClientSocket = false;

  bool m_writingToRemoteSocket = false;

  bool m_clientSocketClosed = false;

  bool m_remoteSocketClosed = false;

};

}

#endif
