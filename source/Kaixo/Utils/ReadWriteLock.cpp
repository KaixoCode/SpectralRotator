
// ------------------------------------------------

#include "Kaixo/Utils/ReadWriteLock.hpp"

// ------------------------------------------------

namespace Kaixo {
    
    // ------------------------------------------------
    //                  Write Lock
    // ------------------------------------------------

    ReadWriteLock::WriteLock::WriteLock(ReadWriteLock& lock)
        : m_Writing(lock.m_Writing)
    {
        lock.m_Writing.store(true, std::memory_order_release);

        while (lock.m_Reading.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    }

    ReadWriteLock::WriteLock::~WriteLock() {
        m_Writing.store(false, std::memory_order_release);
    }

    // ------------------------------------------------
    //                   Read Lock
    // ------------------------------------------------

    ReadWriteLock::ReadLock::ReadLock(const ReadWriteLock& lock)
        : m_Reading(lock.m_Reading)
    {
        if (lock.m_Writing.load(std::memory_order_acquire)) {
            m_Active = false;
            return;
        }

        lock.m_Reading.store(true, std::memory_order_release);

        if (lock.m_Writing.load(std::memory_order_acquire)) {
            lock.m_Reading.store(false, std::memory_order_release);
            m_Active = false;
            return;
        }

        m_Active = true;
    }

    ReadWriteLock::ReadLock::~ReadLock() {
        m_Reading.store(false, std::memory_order_release);
    }

    // ------------------------------------------------

    ReadWriteLock::ReadLock::operator bool() const { return m_Active; }

    // ------------------------------------------------
    //                Read Write Lock
    // ------------------------------------------------

    ReadWriteLock::WriteLock ReadWriteLock::write() { return { *this }; }
    ReadWriteLock::ReadLock ReadWriteLock::read() const { return { *this }; }

    // ------------------------------------------------

}

// ------------------------------------------------
