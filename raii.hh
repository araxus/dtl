#pragma once

#include <sys/mman.h>
#include <sys/stat.h>
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

            if (unlikely(handle == invalid)) return;

            int result;
            while (true) {
                result = ::close(handle);
                if (likely(!result)) break;
                if (unlikely(errno == EINTR)) continue;
                throw std::system_error(errno, std::system_category(), "close");
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

            close();
            handle = other.handle;
            other.handle = invalid;
            return *this;

        } // fd::operator=(fd &&)

        inline fd &
        operator=(
            int fd
            ) noexcept(false) {

            close();
            handle = fd;
            return *this;

        } // fd::operator=(int)

        inline
        ~fd() noexcept(false) {

            close();

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

            close();
            handle = fd;

        } // fd::reset()

        inline void
        swap(fd & other) noexcept {

            branchless::swap(handle, other.handle);

        } // fd::swap(fd &)

        inline auto
        get() const noexcept {

            return handle;

        } // fd::get() const

        inline
        operator int() const {

            return handle;

        } // fd::operator int() const

        inline
        operator bool() const {

            return (handle != invalid);

        } // fd::operator bool() const

    }; // class dtl::raii::fd

    class mmap {

        void * address;
        std::size_t length;

        inline void
        close() noexcept(false) {

            if (unlikely(address == MAP_FAILED)) return;

            auto result = ::munmap(address, length);
            if (unlikely(result == -1)) throw std::system_error(errno, std::system_category(), "munmap");

            address = MAP_FAILED;
            length = 0;

        } // mmap::close()

    public:

        inline explicit
        mmap(
            raii::fd && fd,
            int protection = PROT_READ,
            int flags = MAP_PRIVATE,
            std::size_t offset = 0
            ) noexcept(false) {

            raii::fd handle(std::move(fd)); // sink semantic, defers closure.

            struct ::stat info;
            auto result = ::fstat(handle, &info);
            if (unlikely(result == -1)) throw std::system_error(errno, std::system_category(), "fstat");

            address = ::mmap(nullptr, info.st_size, protection, flags, handle, offset);
            if (unlikely(address == MAP_FAILED)) throw std::system_error(errno, std::system_category(), "mmap");
            length = info.st_size;

        } // mmap::mmap(raii::fd &&, ...)

        inline explicit
        mmap(
            std::size_t length,
            int protection = PROT_READ | PROT_WRITE,
            int flags = MAP_PRIVATE,
            std::size_t offset = 0
            ) noexcept(false)
            : length(0) {

            address = ::mmap(nullptr, length, protection, flags | MAP_ANONYMOUS, -1, offset);
            if (unlikely(address == MAP_FAILED)) throw std::system_error(errno, std::system_category(), "mmap");
            this->length = length;

        } // mmap::mmap(void *, std::size_t, ...)

        inline
        mmap(
            mmap && other
            ) noexcept
            : address(other.address), length(other.length) {

            other.address = MAP_FAILED;
            other.length = 0;

        } // mmap::mmap(mmap &&)

        inline mmap &
        operator=(
            mmap && other
            ) noexcept(false) {

            close();

            address = other.address;
            other.address = MAP_FAILED;
            length = other.length;
            other.length = 0;

            return *this;

        } // fd::operator=(fd &&)

        inline
        ~mmap() noexcept(false) {

            close();

        } // mmap::~mmap()

        mmap(mmap const & other) = delete;
        mmap & operator=(mmap const & other) = delete;

        inline std::pair<void *, std::size_t>
        release() noexcept {

            auto value = std::make_pair(address, length);
            address = MAP_FAILED;
            length = 0;
            return value;

        } // mmap::release()

        inline void
        reset(
            void * address = nullptr,
            std::size_t length = 0
            ) noexcept(false) {

            close();
            this->address = address;
            this->length = length;

        } // mmap::reset()

        inline void
        swap(mmap & other) noexcept {

            branchless::swap(
                *reinterpret_cast<std::uintptr_t *>(&address),
                *reinterpret_cast<std::uintptr_t *>(&other.address)
                );
            branchless::swap(length, other.length);

        } // mmap::swap(mmap &)

        inline auto
        get() const noexcept {

            return address;

        } // mmap::get()

        inline
        operator void *() const {

            return address;

        } // mmap::operator void *() const

        inline auto
        size() const noexcept {

            return length;

        } // mmap::size() const

        inline
        operator bool() const {

            return (address != MAP_FAILED);

        } // mmap::operator bool() const

    }; // class dtl::raii::mmap()

} // namespace dtl::raii

static inline bool
operator==(dtl::raii::fd const & lhs, dtl::raii::fd const & rhs) {

    return (lhs.get() == rhs.get());

}

static inline bool
operator!=(dtl::raii::fd const & lhs, dtl::raii::fd const & rhs) {

    return (lhs.get() != rhs.get());

}

static inline bool
operator==(dtl::raii::mmap const & lhs, dtl::raii::mmap const & rhs) {

    return ((lhs.get() == rhs.get()) & (lhs.size() == rhs.size()));

}

static inline bool
operator!=(dtl::raii::mmap const & lhs, dtl::raii::mmap const & rhs) {

    return ((lhs.get() != rhs.get()) | (lhs.size() != rhs.size()));

}
