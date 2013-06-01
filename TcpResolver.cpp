#include "BoostSystemUtil.h"
#include "TcpResolver.h"

namespace asioproxy
{

TcpResolver::TcpResolver() :
  m_ioService(1),
  m_resolver(m_ioService)
{

}

boost::asio::ip::tcp::endpoint
TcpResolver::resolve(
  const std::tuple<std::string, std::string>& addressPortPair)
{
  boost::asio::ip::tcp::resolver::query query(
    std::get<0>(addressPortPair),
    std::get<1>(addressPortPair));
  boost::asio::ip::tcp::resolver::iterator resolverIter;
  boost::system::error_code errorCode;
  resolverIter = m_resolver.resolve(query, errorCode);
  if (errorCode)
  {
    throw std::runtime_error(
      std::string("error resolving address '") +
      std::get<0>(addressPortPair) + std::string(":") +
      std::get<1>(addressPortPair) + std::string("': ") +
      BoostSystemUtil::buildErrorCodeString(errorCode));
  }
  return *resolverIter;
}

}
