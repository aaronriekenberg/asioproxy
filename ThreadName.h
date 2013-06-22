#ifndef THREAD_NAME_H
#define THREAD_NAME_H

#include <string>

namespace asioproxy {

class ThreadName {
public:

	static void set(const std::string& name);

	static const std::string& get();

private:

	static const std::string m_unknownThreadName;

	// Should use std::thread_local, but it's not supported until g++ 4.8
	// which I don't have yet.
	static __thread std::string* m_pThreadName;

	ThreadName() = delete;

	~ThreadName() = delete;

};

}

#endif
