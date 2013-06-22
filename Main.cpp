#include "Log.h"
#include "ProxyOptions.h"
#include "TcpProxy.h"
#include "ThreadName.h"

int main(int argc, char** argv) {
	try {
		asioproxy::ThreadName::set("main");

		const asioproxy::ProxyOptions& proxyOptions =
				asioproxy::ProxyOptions::createInstance(argc, argv);

		asioproxy::Log::setDebugEnabled(proxyOptions.isDebug());

		asioproxy::TcpProxy::SharedPtr pTcpProxy(asioproxy::TcpProxy::create());
		pTcpProxy->run();
	} catch (const std::exception& e) {
		asioproxy::Log::getInfoInstance() << "main caught exception: "
				<< e.what();
		return 1;
	} catch (...) {
		asioproxy::Log::getInfoInstance() << "main caught unknown exception";
		return 1;
	}
	return 0;
}
