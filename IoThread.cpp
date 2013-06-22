#include "IoThread.h"
#include "Log.h"
#include "ThreadName.h"

namespace asioproxy {

std::shared_ptr<std::thread> IoThread::create(size_t ioThreadNumber,
		boost::asio::io_service& ioService) {
	return std::shared_ptr < std::thread
			> (new std::thread(IoThread(ioThreadNumber, ioService)));
}

void IoThread::operator()() {
	try {
		ThreadName::set(std::string("io-") + std::to_string(m_ioThreadNumber));

		m_ioService.run();
	} catch (const std::exception& e) {
		Log::getInfoInstance() << "io thread caught exception: " << e.what();
	} catch (...) {
		Log::getInfoInstance() << "io thread caught unknown exception";
	}
}

IoThread::IoThread(size_t ioThreadNumber, boost::asio::io_service& ioService) :
		m_ioThreadNumber(ioThreadNumber), m_ioService(ioService) {

}

}
