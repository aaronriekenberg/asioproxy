#ifndef BOOST_SYSTEM_UTIL_H
#define BOOST_SYSTEM_UTIL_H

#include <boost/system/error_code.hpp>
#include <string>

namespace asioproxy {

class BoostSystemUtil {
public:

	static std::string buildErrorCodeString(
			const boost::system::error_code& errorCode);

private:

	BoostSystemUtil() = delete;

	~BoostSystemUtil() = delete;

};

}
;

#endif
