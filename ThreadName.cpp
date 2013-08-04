#include "ThreadName.h"

namespace asioproxy {

thread_local std::string ThreadName::m_threadName = "unknown";

void ThreadName::set(const std::string& name) {
	m_threadName = name;
}

const std::string& ThreadName::get() {
	return m_threadName;
}

}
