#include "Log.h"
#include "TcpProxy.h"
#include "TcpProxyClientAcceptor.h"
#include "TcpResolver.h"
#include "ProxyOptions.h"

namespace asioproxy
{

TcpProxy::SharedPtr TcpProxy::create()
{
  return SharedPtr(new TcpProxy);
}

TcpProxy::~TcpProxy()
{

}

void TcpProxy::run()
{
  logStartupInfo();
  createAcceptors();
  runIoThreads();
}

TcpProxy::TcpProxy() :
  m_ioServicePool(ProxyOptions::getInstance().getNumThreads())
{

}

void TcpProxy::logStartupInfo()
{
  const ProxyOptions& proxyOptions = ProxyOptions::getInstance();

  Log::getInfoInstance() << "BOOST_LIB_VERSION = " << BOOST_LIB_VERSION;
  Log::getInfoInstance() << "buffer size = "
    << proxyOptions.getBufferSize() << " bytes";
  Log::getInfoInstance() << "connect timeout = "
    << proxyOptions.getConnectTimeout().count()
    << " milliseconds";
  Log::getInfoInstance() << "no delay = " << proxyOptions.isNoDelay();
  Log::getInfoInstance() << "num threads = " << proxyOptions.getNumThreads();
}

void TcpProxy::createAcceptors()
{
  const ProxyOptions& proxyOptions = ProxyOptions::getInstance();

  TcpResolver tcpResolver;

  // resolve local endpoints
  std::vector<boost::asio::ip::tcp::endpoint> localEndpointVector;
  localEndpointVector.reserve(
    proxyOptions.getLocalAddressPortVector().size());
  std::transform(proxyOptions.getLocalAddressPortVector().begin(),
                 proxyOptions.getLocalAddressPortVector().end(),
                 std::back_inserter(localEndpointVector),
                 [&] (const ProxyOptions::AddressAndPort& localAddressAndPort)
                 {
                   return tcpResolver.resolve(localAddressAndPort);
                 });

  // create acceptors
  std::vector<TcpProxyClientAcceptor::SharedPtr> acceptorVector;
  acceptorVector.reserve(localEndpointVector.size());
  std::transform(localEndpointVector.begin(),
                 localEndpointVector.end(),
                 std::back_inserter(acceptorVector),
                 [&] (const boost::asio::ip::tcp::endpoint& localEndpoint)
                 {
                   return TcpProxyClientAcceptor::create(
                     m_ioServicePool, localEndpoint,
                     proxyOptions.getRemoteAddressPort());
                 });

  // start acceptors
  for (auto pAcceptor : acceptorVector)
  {
    pAcceptor->start();
  }
}

void TcpProxy::runIoThreads()
{
  m_ioServicePool.runIoThreads();
}

}
