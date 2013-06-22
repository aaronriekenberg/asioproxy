#ifndef TCP_PROXY_CLIENT_ACCEPTOR_H
#define TCP_PROXY_CLIENT_ACCEPTOR_H

#include <boost/asio.hpp>
#include <memory>
#include "TcpProxySession.h"

namespace asioproxy {

class IoServicePool;

class TcpProxyClientAcceptor: public std::enable_shared_from_this<
		TcpProxyClientAcceptor> {
public:

	typedef std::shared_ptr<TcpProxyClientAcceptor> SharedPtr;

	static SharedPtr create(IoServicePool& ioServicePool,
			const boost::asio::ip::tcp::endpoint& localEndpoint);

	virtual ~TcpProxyClientAcceptor();

	void start();

private:

	TcpProxyClientAcceptor(const TcpProxyClientAcceptor& rhs) = delete;

	TcpProxyClientAcceptor& operator=(const TcpProxyClientAcceptor& rhs) = delete;

	TcpProxyClientAcceptor(IoServicePool& ioServicePool,
			const boost::asio::ip::tcp::endpoint& localEndpoint);

	void registerForAccept();

	void handleAccept(TcpProxySession::SharedPtr pSession,
			const boost::system::error_code& error);

	IoServicePool& m_ioServicePool;

	boost::asio::ip::tcp::acceptor m_acceptor;

};

}

#endif
