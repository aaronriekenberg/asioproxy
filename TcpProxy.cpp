#include "Log.h"
#include "TcpProxy.h"
#include "TcpProxyClientAcceptor.h"
#include "TcpResolver.h"
#include "ProxyOptions.h"

namespace asioproxy {

TcpProxy::SharedPtr TcpProxy::create() {
	return SharedPtr(new TcpProxy);
}

void TcpProxy::run() {
	logStartupInfo();
	createAcceptors();
	runIoThreads();
}

TcpProxy::TcpProxy() :
		m_ioServicePool(ProxyOptions::getInstance().getNumThreads()) {

}

void TcpProxy::logStartupInfo() {
	const ProxyOptions& proxyOptions = ProxyOptions::getInstance();

	Log::getInfoInstance() << "BOOST_LIB_VERSION = " << BOOST_LIB_VERSION;
	Log::getInfoInstance() << "buffer size = " << proxyOptions.getBufferSize()
			<< " bytes";
	Log::getInfoInstance() << "connect timeout = "
			<< proxyOptions.getConnectTimeout().count() << " milliseconds";
	Log::getInfoInstance() << "no delay = " << proxyOptions.isNoDelay();
	Log::getInfoInstance() << "num threads = " << proxyOptions.getNumThreads();
}

void TcpProxy::createAcceptors() {
	TcpResolver tcpResolver;
	for (const auto& localAddressPort : ProxyOptions::getInstance().getLocalAddressPortVector()) {
		auto localEndpoint = tcpResolver.resolve(localAddressPort);
		TcpProxyClientAcceptor::create(m_ioServicePool, localEndpoint)->start();
	}
}

void TcpProxy::runIoThreads() {
	m_ioServicePool.runIoThreads();
}

}
