#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"

// ------------------------------------------------

namespace Kaixo {
    
    // ------------------------------------------------

    /** 
        Simple read/write lock, that blocks the writing thread, but
        simply signals the read thread if unable to read. So it does
        not block the audio thread.
     */
    class ReadWriteLock {
    public:

        // ------------------------------------------------

        class WriteLock {
        public:

            // ------------------------------------------------

            WriteLock(ReadWriteLock& lock);
            ~WriteLock();

            // ------------------------------------------------

        private:
            std::atomic_bool& m_Writing;
        };

        // ------------------------------------------------

        class ReadLock {
        public:

            // ------------------------------------------------

            ReadLock(const ReadWriteLock& lock);
            ~ReadLock();

            // ------------------------------------------------

            explicit operator bool() const;

            // ------------------------------------------------

        private:
            bool m_Active = false;
            std::atomic_bool& m_Reading;
        };

        // ------------------------------------------------

        WriteLock write();
        ReadLock read() const;

        // ------------------------------------------------

    private:
        std::atomic_bool m_Writing = false;
        mutable std::atomic_bool m_Reading = false;
    };

    // ------------------------------------------------

}

// ------------------------------------------------
