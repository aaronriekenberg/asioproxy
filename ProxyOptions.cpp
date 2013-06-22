#include "ProxyOptions.h"
#include <boost/program_options.hpp>
#include <thread>

namespace {
const size_t DEFAULT_BUFFER_SIZE = 64 * 1024;
const long DEFAULT_CONNECT_TIMEOUT_MS = 10 * 1000;
}

namespace asioproxy {

ProxyOptions* ProxyOptions::m_pInstance = 0;

const ProxyOptions& ProxyOptions::createInstance(int argc, char** argv) {
	if (m_pInstance) {
		throw std::runtime_error("ProxyOptions::createInstance called twice");
	}
	m_pInstance = new ProxyOptions(argc, argv);
	return *m_pInstance;
}

const ProxyOptions& ProxyOptions::getInstance() {
	if (!m_pInstance) {
		throw std::runtime_error(
				"ProxyOptions::getInstance called before ProxyOptions::createInstance");
	}
	return *m_pInstance;
}

const std::vector<ProxyOptions::AddressAndPort>&
ProxyOptions::getLocalAddressPortVector() const {
	return m_localAddressPortVector;
}

const ProxyOptions::AddressAndPort&
ProxyOptions::getRemoteAddressPort() const {
	return m_remoteAddressPort;
}

size_t ProxyOptions::getBufferSize() const {
	return m_bufferSize;
}

std::chrono::milliseconds ProxyOptions::getConnectTimeout() const {
	return m_connectTimeoutMilliseconds;
}

size_t ProxyOptions::getNumThreads() const {
	return m_numThreads;
}

bool ProxyOptions::isDebug() const {
	return m_debug;
}

bool ProxyOptions::isNoDelay() const {
	return m_noDelay;
}

ProxyOptions::ProxyOptions(int argc, char** argv) {
	long connectTimeoutMS = 0;

	boost::program_options::options_description desc("Allowed options");
	desc.add_options()("help,h", "show help message")("local-address,l",
			boost::program_options::value<std::vector<std::string> >(),
			"local address")("remote-address,r",
			boost::program_options::value<std::string>(), "remote address")(
			"buffer-size,b",
			boost::program_options::value<size_t>(&m_bufferSize)->default_value(
					DEFAULT_BUFFER_SIZE), "buffer size (bytes)")(
			"connect-timeout,c",
			boost::program_options::value<long>(&connectTimeoutMS)->default_value(
					DEFAULT_CONNECT_TIMEOUT_MS),
			"connect timeout (milliseconds)")("num-threads,t",
			boost::program_options::value<size_t>(&m_numThreads)->default_value(
					std::thread::hardware_concurrency()), "number of threads")(
			"debug,d", "debug")("no-delay,n", "no delay");

	boost::program_options::variables_map vm;
	boost::program_options::store(
			boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		throw std::runtime_error("");
	}

	if (!vm.count("local-address")) {
		throw std::runtime_error("missing local address");
	}
	std::vector<std::string> localAddresses = vm["local-address"].as<
			std::vector<std::string> >();
	m_localAddressPortVector.reserve(localAddresses.size());
	std::transform(localAddresses.begin(), localAddresses.end(),
			std::back_inserter(m_localAddressPortVector),
			[&] (const std::string& localAddress)
			{
				return parseAddressPortString(localAddress);
			});

	if (!vm.count("remote-address")) {
		throw std::runtime_error("missing remote address");
	}
	m_remoteAddressPort = parseAddressPortString(
			vm["remote-address"].as<std::string>());

	if (m_bufferSize <= 0) {
		throw std::runtime_error("invalid buffer size");
	}

	if (connectTimeoutMS <= 0) {
		throw std::runtime_error("invalid connect timeout");
	}
	m_connectTimeoutMilliseconds = std::chrono::milliseconds(connectTimeoutMS);

	if (m_numThreads <= 0) {
		throw std::runtime_error("invalid num threads");
	}

	if (vm.count("debug")) {
		m_debug = true;
	}

	if (vm.count("no-delay")) {
		m_noDelay = true;
	}
}

ProxyOptions::AddressAndPort ProxyOptions::parseAddressPortString(
		const std::string& addressPortString) {
	bool error = false;
	const std::string::size_type colonPos = addressPortString.find_last_of(':');
	if (colonPos == std::string::npos) {
		error = true;
	}

	std::string addressString;
	std::string portString;

	if (!error) {
		addressString = addressPortString.substr(0, colonPos);
		portString = addressPortString.substr(colonPos + 1);
		error = (addressString.empty() || portString.empty());
	}

	if (error) {
		throw std::runtime_error(
				std::string("invalid address:port ") + addressPortString);
	}

	return AddressAndPort(addressString, portString);
}

}
