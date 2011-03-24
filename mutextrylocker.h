#ifndef MUTEXTRYLOCKER_H
#define MUTEXTRYLOCKER_H

class QMutex;

namespace Mirror {

class MutexTryLocker
{
public:
    MutexTryLocker(QMutex * mutex);
    ~MutexTryLocker();

    void unlock();
    void relock();
    QMutex * mutex() { return m_mutex; }

    operator bool() { return m_locked; }

protected:
    QMutex * m_mutex;
    bool m_locked;
};

} // namespace Mirror

#endif // MUTEXTRYLOCKER_H
