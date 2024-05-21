
// ------------------------------------------------

#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Processing/Module.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    struct ComplexBuffer {
        std::vector<std::complex<float>> l;
        std::vector<std::complex<float>> r;

        constexpr std::size_t size() const { return l.size(); }
        constexpr bool empty() const { return l.empty(); }

        constexpr void reserve(std::size_t size) {
            l.reserve(size);
            r.reserve(size);
        }

        constexpr void resize(std::size_t size) {
            l.resize(size);
            r.resize(size);
        }
    };

    // ------------------------------------------------

}

// ------------------------------------------------
