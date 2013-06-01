#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <sstream>

namespace asioproxy
{

class Log
{
public:

  static Log getInfoInstance();

  static Log getDebugInstance();

  static bool isDebugEnabled();

  static void setDebugEnabled(bool debugEnabled);

  template <typename T>
  Log& operator<<(const T& t)
  {
    if (m_enabled)
    {
      m_stringstream << t;
    }
    return *this;
  }

  Log(Log&& rhs);

  Log& operator=(Log&& rhs);

  ~Log();

private:

  explicit Log(bool enabled);

  Log(const Log& rhs) = delete;

  Log& operator=(const Log& rhs) = delete;

  static bool m_debugEnabled;

  static std::mutex m_logMutex;

  bool m_enabled;

  std::stringstream m_stringstream;

};

}

#endif
