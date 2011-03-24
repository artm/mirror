#include "mutextrylocker.h"

#include <QMutex>

namespace Mirror {

MutexTryLocker::MutexTryLocker(QMutex * mutex)
{
    m_mutex = mutex;
    m_locked = m_mutex->tryLock();
}


MutexTryLocker::~MutexTryLocker()
{
    if (m_locked) {
        m_mutex->unlock();
    }
}

void MutexTryLocker::unlock()
{
    Q_ASSERT(m_locked);
    m_mutex->unlock();
    m_locked = false;
}

void MutexTryLocker::relock()
{
    Q_ASSERT(!m_locked);
    m_mutex->lock();
    m_locked = true;
}


} // namespace Mirror
