#include "ThreadName.h"
#include "Log.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

namespace
{

std::string timeString()
{
  std::stringstream ss;

  const std::chrono::time_point<std::chrono::system_clock> now =
    std::chrono::system_clock::now();
  const std::time_t nowTimeT = std::chrono::system_clock::to_time_t(now);

  struct tm tmStruct;
  ::localtime_r(&nowTimeT, &tmStruct);

  // should use std::put_time, but it's not supported by g++ yet.
  char charBuffer[80];
  std::strftime(charBuffer, 80, "%Y-%b-%d %H:%M:%S", &tmStruct);
  ss << charBuffer
     << '.'
     << std::right << std::setw(6) << std::setfill('0')
     << (std::chrono::duration_cast<std::chrono::microseconds>(
           now.time_since_epoch()).count() %
         std::micro::den);
  return ss.str();
}

}

namespace asioproxy
{

bool Log::m_debugEnabled = true;

std::mutex Log::m_logMutex;

Log Log::getInfoInstance()
{
  return Log(true);
}

Log Log::getDebugInstance()
{
  return Log(m_debugEnabled);
}

bool Log::isDebugEnabled()
{
  return m_debugEnabled;
}

void Log::setDebugEnabled(bool debugEnabled)
{
  m_debugEnabled = debugEnabled;
}

Log::Log(Log&& rhs)
{
  *this = std::move(rhs);
}

Log& Log::operator=(Log&& rhs)
{
  if (this != &rhs)
  {
    m_stringstream.str("");
    m_stringstream << rhs.m_stringstream.str();
    m_enabled = rhs.m_enabled;
    rhs.m_enabled = false;
  }
  return *this;
}

Log::~Log()
{
  if (m_enabled)
  {
    std::lock_guard<std::mutex> lock(m_logMutex);
    std::cout << m_stringstream.str() << std::endl;
  }
}

Log::Log(bool enabled) :
  m_enabled(enabled)
{
  if (m_enabled)
  {
    m_stringstream << timeString()
                   << " ["
                   << ThreadName::get()
                   << "] ";
  }
}

}
