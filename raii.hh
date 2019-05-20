#pragma once

#include <unistd.h>
#include <system_error>
#include "branch.hh"
#include "branchless.hh"

namespace dtl::raii {

    class fd {

        constexpr static int invalid = -1;
        int handle;

        inline void
        close() noexcept(false) {

            if (unlikely(handle == invalid)) throw std::system_error(EBADF, std::system_category());
            int result;
            while (true) {
                result = ::close(handle);
                if (likely(!result)) break;
                if (unlikely(errno == EINTR)) continue;
                throw std::system_error(errno, std::system_category());
            }
            handle = invalid;

        } // fd::close()

    public:

        inline
        fd(
            int fd = invalid
            ) noexcept
            : handle(fd) {}

        inline
        fd(
            fd && other
            ) noexcept {

            handle = other.handle;
            other.handle = invalid;

        } // fd::fd(fd &&)

        inline fd &
        operator=(
            fd && other
            ) noexcept(false) {

            if (handle != invalid) close();
            handle = other.handle;
            other.handle = invalid;
            return *this;

        } // fd::operator=(fd &&)

        inline fd &
        operator=(
            int fd
            ) noexcept(false) {

            if (handle != invalid) close();
            handle = fd;
            return *this;

        } // fd::operator=(int)

        inline
        ~fd() noexcept(false) {

            if (handle != invalid) close();

        } // fd::~fd()

        fd(fd const & other) = delete;
        fd & operator=(fd const & other) = delete;

        inline int
        release() noexcept {

            int tmp = handle;
            handle = invalid;
            return tmp;

        } // fd::release()

        inline void
        reset(int fd = invalid) noexcept(false) {

            if (handle != invalid) close();
            handle = fd;

        } // fd::reset()

        inline void
        swap(fd & other) noexcept {

            branchless::swap(handle, other.handle);

        } // fd::swap(fd &)

        inline int
        get() const noexcept {

            return handle;

        } // fd::get()

        inline
        operator int() const {

            return handle;

        } // fd::operator bool()

        inline
        operator bool() const {

            return (handle != invalid);

        } // fd::operator bool()

    }; // class dtl::raii::fd

} // namespace dtl::raii

static inline bool
operator==(dtl::raii::fd const & lhs, dtl::raii::fd const & rhs) {

    return (lhs.get() == rhs.get());

}

static inline bool
operator!=(dtl::raii::fd const & lhs, dtl::raii::fd const & rhs) {

    return (lhs.get() != rhs.get());

}
