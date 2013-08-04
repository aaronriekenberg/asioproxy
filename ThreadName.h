#ifndef THREAD_NAME_H
#define THREAD_NAME_H

#include <string>

namespace asioproxy {

class ThreadName {
public:

	static void set(const std::string& name);

	static const std::string& get();

private:

	static thread_local std::string m_threadName;

	ThreadName() = delete;

	~ThreadName() = delete;

};

}

#endif
