#ifndef PROXY_OPTIONS_H
#define PROXY_OPTIONS_H

#include <chrono>
#include <string>
#include <tuple>
#include <vector>

namespace asioproxy {

class ProxyOptions {
public:

	static const ProxyOptions& createInstance(int argc, char** argv);

	static const ProxyOptions& getInstance();

	typedef std::tuple<std::string, std::string> AddressAndPort;

	const std::vector<AddressAndPort>& getLocalAddressPortVector() const;

	const AddressAndPort& getRemoteAddressPort() const;

	size_t getBufferSize() const;

	std::chrono::milliseconds getConnectTimeout() const;

	size_t getNumThreads() const;

	bool isDebug() const;

	bool isNoDelay() const;

private:

	~ProxyOptions() = delete;

	ProxyOptions(const ProxyOptions& rhs) = delete;

	ProxyOptions& operator=(const ProxyOptions& rhs) = delete;

	ProxyOptions(int argc, char** argv);

	AddressAndPort parseAddressPortString(const std::string& addressPortString);

	static ProxyOptions* m_pInstance;

	std::vector<AddressAndPort> m_localAddressPortVector;

	AddressAndPort m_remoteAddressPort;

	size_t m_bufferSize = 0;

	std::chrono::milliseconds m_connectTimeoutMilliseconds;

	size_t m_numThreads = 0;

	bool m_debug = false;

	bool m_noDelay = false;

};

}

#endif
