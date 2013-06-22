#include "IoServicePool.h"
#include "IoThread.h"

namespace asioproxy {

IoServicePool::IoServicePool(size_t poolSize) {
	m_ioServiceVector.resize(poolSize);
	std::generate(m_ioServiceVector.begin(), m_ioServiceVector.end(), [] ()
	{
		return IoServicePtr(new boost::asio::io_service(1));
	});

	m_workVector.reserve(poolSize);
	std::transform(m_ioServiceVector.begin(), m_ioServiceVector.end(),
			std::back_inserter(m_workVector), [] (IoServicePtr pIOService)
			{
				return WorkPtr(new boost::asio::io_service::work(*pIOService));
			});
}

void IoServicePool::runIoThreads() {
	std::vector<ThreadPtr> threadVector;

	threadVector.reserve(m_ioServiceVector.size());
	for (size_t i = 0; i < m_ioServiceVector.size(); ++i) {
		threadVector.push_back(IoThread::create(i, *(m_ioServiceVector[i])));
	}

	for (auto pThread : threadVector) {
		pThread->join();
	}
}

boost::asio::io_service& IoServicePool::getIoService() {
	std::lock_guard < std::mutex > lock(m_mutex);

	boost::asio::io_service& retVal = *(m_ioServiceVector.at(
			m_nextIoServiceIndex));
	++m_nextIoServiceIndex;
	if (m_nextIoServiceIndex >= m_ioServiceVector.size()) {
		m_nextIoServiceIndex = 0;
	}
	return retVal;
}

}
