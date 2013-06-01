#include "ThreadName.h"

namespace asioproxy
{

const std::string ThreadName::m_unknownThreadName = "Unknown";

__thread std::string* ThreadName::m_pThreadName = nullptr;

void ThreadName::set(const std::string& name)
{
  if (m_pThreadName)
  {
    delete m_pThreadName;
  }
  m_pThreadName = new std::string(name);
}

const std::string& ThreadName::get()
{
  const std::string* pThreadName = m_pThreadName;
  if (pThreadName)
  {
    return *pThreadName;
  }
  return m_unknownThreadName;
}

}
