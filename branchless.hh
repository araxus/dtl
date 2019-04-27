#pragma once

#include <cstdint>
#include <type_traits>

namespace dtl::branchless {

    namespace _ {

        // If an integral value's sign bit is set, hiword = 0xFF..FF, otherwise zero.
        // c.f. https://hbfs.wordpress.com/2008/08/05/branchless-equivalents-of-simple-functions/

        union sign_extend_64 {

            __int128 dword;
            struct {
                std::int64_t loword;
                std::int64_t hiword;
            };

            sign_extend_64() = delete;

            inline constexpr
            sign_extend_64(std::int64_t value) noexcept : dword(value) {}

            inline constexpr std::int64_t
            mask() noexcept { return hiword; }

        }; // union dtl::branchless::_::sign_extend_64

        union sign_extend_32 {

            std::int64_t dword;
            struct {
                std::int32_t loword;
                std::int32_t hiword;
            };

            sign_extend_32() = delete;

            inline constexpr
            sign_extend_32(std::int32_t value) noexcept : dword(value) {}

            inline constexpr std::int32_t
            mask() noexcept { return hiword; }

        }; // union dtl::branchless::_::sign_extend_32

        union sign_extend_16 {

            std::int32_t dword;
            struct {
                std::int16_t loword;
                std::int16_t hiword;
            };

            sign_extend_16() = delete;

            inline constexpr
            sign_extend_16(std::int16_t value) noexcept : dword(value) {}

            inline constexpr std::int16_t
            mask() noexcept { return hiword; }

        }; // union dtl::branchless::_::sign_extend_16

        union sign_extend_8 {

            std::int16_t dword;
            struct {
                std::int8_t loword;
                std::int8_t hiword;
            };

            sign_extend_8() = delete;

            inline constexpr
            sign_extend_8(std::int8_t value) noexcept : dword(value) {}

            inline constexpr std::int8_t
            mask() noexcept { return hiword; }

        }; // union dtl::branchless::_::sign_extend_8

        template<typename T, std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, T> = 0>
        using sign_extend =
            std::conditional_t<sizeof(T) == 8, sign_extend_64,
            std::conditional_t<sizeof(T) == 4, sign_extend_32,
            std::conditional_t<sizeof(T) == 2, sign_extend_16,
            std::conditional_t<sizeof(T) == 1, sign_extend_8,
            std::nullptr_t>>>>;

    } // namespace dtl::branchless::_

    template<typename T>
    inline static constexpr auto
    abs(
        T value
        ) noexcept {

        auto mask = _::sign_extend<T>(value).mask();
        return (value ^ mask) - mask;

    } // dtl::abs()

    template<typename T>
    inline static constexpr std::enable_if_t<std::is_integral<T>::value, T>
    min(
        const T & x,
        const T & y
        ) noexcept {

        return y ^ ((x ^ y) & -(x < y));

    } // dtl::min()

    template<typename T>
    inline static constexpr std::enable_if_t<std::is_integral<T>::value, T>
    max(
        const T & x,
        const T & y
        ) noexcept {

        return x ^ ((x ^ y) & -(x < y));

    } // dtl::max()

    namespace power_of_2 {

        // Returns true if the unsigned value is a power of 2.
        template<typename T>
        inline static constexpr std::enable_if_t<std::is_integral<T>::value && !std::is_signed<T>::value, bool>
        isa(
            T value
            ) noexcept {

            return (((value - 1) & value) == 0);

        } // power_of_2::isa()

        // Rounds the unsigned value up to the next power of 2.
        template<typename T>
        inline static constexpr std::enable_if_t<std::is_integral<T>::value && !std::is_signed<T>::value, T>
        roundup(
            T value
            ) noexcept {

            --value;
            value |= value >>  1;
            value |= value >>  2;
            value |= value >>  4;
            value |= value >>  8;
            value |= value >> 16;
            ++value;

            return value;

        } // power_of_2::roundup()

        // Returns true if the unsigned value is a mask one less than a power of 2 (e.g. 0xFF...FF).
        template<typename T>
        inline static constexpr std::enable_if_t<std::is_integral<T>::value && !std::is_signed<T>::value, bool>
        isa_minus_1(
            T value
            ) noexcept {

            return ((value & (value + 1)) == 0);

        } // power_of_2::isa_minus_1()

        // Rounds the unsigned value up to the mask one less than the next power of 2 (e.g 0xFF...FF).
        template<typename T>
        inline static constexpr std::enable_if_t<std::is_integral<T>::value && !std::is_signed<T>::value, T>
        roundup_minus_1(
            T value
            ) noexcept {

            --value;
            value |= value >>  1;
            value |= value >>  2;
            value |= value >>  4;
            value |= value >>  8;
            value |= value >> 16;

            return value;

        } // power_of_2::roundup_minus_1()

    } // namespace power_of_2

} // namespace dtl::branchless
