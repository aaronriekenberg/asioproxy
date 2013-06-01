#include "BoostSystemUtil.h"
#include "IoServicePool.h"
#include "Log.h"
#include "TcpProxyClientAcceptor.h"

namespace asioproxy
{

TcpProxyClientAcceptor::SharedPtr
TcpProxyClientAcceptor::create(
  IoServicePool& ioServicePool,
  const boost::asio::ip::tcp::endpoint& localEndpoint,
  const boost::asio::ip::tcp::endpoint& remoteEndpoint)
{
  return SharedPtr(
    new TcpProxyClientAcceptor(
      ioServicePool,
      localEndpoint,
      remoteEndpoint));
}

TcpProxyClientAcceptor::~TcpProxyClientAcceptor()
{
  if (Log::isDebugEnabled())
  {
    Log::getDebugInstance()
      << "TcpProxyClientAcceptor::~TcpProxyClientAcceptor " << this;
  }
}

void TcpProxyClientAcceptor::start()
{
  Log::getInfoInstance() << "listening on " << m_acceptor.local_endpoint();
  registerForAccept();
}

TcpProxyClientAcceptor::TcpProxyClientAcceptor(
  IoServicePool& ioServicePool,
  const boost::asio::ip::tcp::endpoint& localEndpoint,
  const boost::asio::ip::tcp::endpoint& remoteEndpoint) :
  m_ioServicePool(ioServicePool),
  m_acceptor(ioServicePool.getIoService(), localEndpoint),
  m_remoteEndpoint(remoteEndpoint)
{
  if (Log::isDebugEnabled())
  {
    Log::getDebugInstance()
     << "TcpProxyClientAcceptor::TcpProxyClientAcceptor " << this;
  }
}

void TcpProxyClientAcceptor::registerForAccept()
{
  auto pSession =
    TcpProxySession::create(
      m_ioServicePool.getIoService(),
      m_remoteEndpoint);
  auto sharedThis = shared_from_this();
  m_acceptor.async_accept(
    pSession->getClientSocket(),
    [=] (const boost::system::error_code& error)
    {
      sharedThis->handleAccept(pSession, error);
    });
}

void TcpProxyClientAcceptor::handleAccept(
  TcpProxySession::SharedPtr pSession,
  const boost::system::error_code& error)
{
  if (error)
  {
    if (Log::isDebugEnabled())
    {
      Log::getDebugInstance()
        << "TcpProxyClientAcceptor::handleAccept error = "
        << BoostSystemUtil::buildErrorCodeString(error);
    }
  }
  else
  {
    pSession->handleClientSocketAccepted();
  }

  registerForAccept();
}

}
