/* 
 * Free FFT and convolution (C++)
 * 
 * Copyright (c) 2021 Project Nayuki. (MIT License)
 * https://www.nayuki.io/page/free-small-fft-in-multiple-languages
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * - The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * - The Software is provided "as is", without warranty of any kind, express or
 *   implied, including but not limited to the warranties of merchantability,
 *   fitness for a particular purpose and noninfringement. In no event shall the
 *   authors or copyright holders be liable for any claim, damages or other
 *   liability, whether in an action of contract, tort or otherwise, arising from,
 *   out of or in connection with the Software or the use or other dealings in the
 *   Software.
 */

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include "Kaixo/SpectralRotator/Processing/Utils/Fft.hpp"

// ------------------------------------------------

namespace Kaixo::Processing {

    // ------------------------------------------------

    using std::complex;
    using std::size_t;
    using std::uintmax_t;
    using std::vector;

    // ------------------------------------------------

    constexpr size_t reverseBits(size_t val, int width) {
        size_t result = 0;
        for (int i = 0; i < width; i++, val >>= 1)
            result = (result << 1) | (val & 1U);
        return result;
    }

    // ------------------------------------------------

    void Fft::transform(vector<complex<float> >& vec, bool inverse) {
        size_t n = vec.size();
        if (n == 0)
            return;
        else if ((n & (n - 1)) == 0)  // Is power of 2
            transformRadix2(vec, inverse);
        else  // More complicated algorithm for arbitrary sizes
            transformBluestein(vec, inverse);
    }

    // ------------------------------------------------

    void Fft::transformRadix2(vector<complex<float> >& vec, bool inverse) {
        // Length variables
        size_t n = vec.size();
        int levels = 0;  // Compute levels = floor(log2(n))
        for (size_t temp = n; temp > 1U; temp >>= 1)
            levels++;
        if (static_cast<size_t>(1U) << levels != n)
            throw std::domain_error("Length is not a power of 2");

        // Trigonometric table
        vector<complex<float> > expTable(n / 2);
        for (size_t i = 0; i < n / 2; i++) {
            step();
            expTable[i] = std::polar(1.0f, (inverse ? 2 : -2) * (std::numbers::pi_v<float>) * i / n);
        }

        // Bit-reversed addressing permutation
        for (size_t i = 0; i < n; i++) {
            step();
            size_t j = reverseBits(i, levels);
            if (j > i)
                std::swap(vec[i], vec[j]);
        }

        // Cooley-Tukey decimation-in-time radix-2 FFT
        for (size_t size = 2; size <= n; size *= 2) {
            size_t halfsize = size / 2;
            size_t tablestep = n / size;
            for (size_t i = 0; i < n; i += size) {
                for (size_t j = i, k = 0; j < i + halfsize; j++, k += tablestep) {
                    complex<float> temp = vec[j + halfsize] * expTable[k];
                    vec[j + halfsize] = vec[j] - temp;
                    vec[j] += temp;
                    step();
                }
            }
            if (size == n)  // Prevent overflow in 'size *= 2'
                break;
        }
    }

    // ------------------------------------------------

    void Fft::transformBluestein(vector<complex<float> >& vec, bool inverse) {
        // Find a power-of-2 convolution length m such that m >= n * 2 + 1
        size_t n = vec.size();
        size_t m = std::bit_ceil(n * 2 + 1);

        // Trigonometric table
        vector<complex<float> > expTable(n);
        for (size_t i = 0; i < n; i++) {
            uintmax_t temp = static_cast<uintmax_t>(i) * i;
            temp %= static_cast<uintmax_t>(n) * 2;
            float angle = (inverse ? (std::numbers::pi_v<float>) : -(std::numbers::pi_v<float>)) * temp / n;
            expTable[i] = std::polar(1.0f, angle);
            step();
        }

        // Temporary vectors and preprocessing
        vector<complex<float> > avec(m);
        for (size_t i = 0; i < n; i++) {
            avec[i] = vec[i] * expTable[i];
            step();
        }
        vector<complex<float> > bvec(m);
        bvec[0] = expTable[0];
        for (size_t i = 1; i < n; i++) {
            step();
            bvec[i] = bvec[m - i] = std::conj(expTable[i]);
        }

        // Convolution
        vector<complex<float> > cvec = convolve(std::move(avec), std::move(bvec));

        // Postprocessing
        for (size_t i = 0; i < n; i++) {
            step();
            vec[i] = cvec[i] * expTable[i];
        }
    }

    // ------------------------------------------------

    vector<complex<float> > Fft::convolve(
        vector<complex<float> > xvec,
        vector<complex<float> > yvec) {

        size_t n = xvec.size();
        if (n != yvec.size())
            throw std::domain_error("Mismatched lengths");
        transform(xvec, false);
        transform(yvec, false);
        for (size_t i = 0; i < n; i++)
            xvec[i] *= yvec[i];
        transform(xvec, true);
        for (size_t i = 0; i < n; i++)  // Scaling (because this FFT implementation omits it)
            xvec[i] /= static_cast<float>(n);
        return xvec;
    }

    // ------------------------------------------------

    std::size_t estimateBluestein(std::size_t size, bool inverse);
    std::size_t estimateRadix2(std::size_t size, bool inverse);
    std::size_t estimateFft(std::size_t size, bool inverse) {
        size_t n = size;
        if (n == 0) return 0;
        else if ((n & (n - 1)) == 0) return estimateRadix2(size, inverse);
        else return estimateBluestein(size, inverse);
    }

    std::size_t estimateRadix2(std::size_t size, bool inverse) {
        std::size_t n = size;
        std::size_t steps = 0;
        for (size_t size = 2; size <= n; size *= 2) {
            size_t halfsize = size / 2;
            size_t tablestep = n / size;
            for (size_t i = 0; i < n; i += size) {
                for (size_t j = i, k = 0; j < i + halfsize; j++, k += tablestep) {
                    steps++;
                }
            }
            if (size == n)  // Prevent overflow in 'size *= 2'
                break;
        }
        return n / 2 + // Trigonometry table
            n +        // Bit reverse
            steps; // Cooley-Tukey
    }
    
    std::size_t estimateBluestein(std::size_t size, bool inverse) {
        size_t n = size;
        size_t m = std::bit_ceil(n * 2 + 1);

        return n + // Trig table
            n + n - 1 + // Preprocessing
            n + // Postprocessing
            estimateRadix2(m, inverse) * 3; // in the convolve
    }

    std::size_t Fft::estimateSteps(std::size_t size, bool inverse) {
        return estimateFft(size, inverse);
    }

    // ------------------------------------------------

}

// ------------------------------------------------
