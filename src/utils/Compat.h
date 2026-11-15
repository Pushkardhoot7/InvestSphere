#ifndef INVESTSPHERE_COMPAT_H
#define INVESTSPHERE_COMPAT_H

#if __has_include(<optional>)
#  include <optional>
   namespace InvestSphere {
       template <typename T>
       using Optional = std::optional<T>;
   }
#elif __has_include(<experimental/optional>)
#  include <experimental/optional>
   namespace InvestSphere {
       template <typename T>
       using Optional = std::experimental::optional<T>;
   }
#endif

#include <string>

#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#  include <direct.h>
#  include <io.h>
#  define InvestSphere_mkdir(dir) _mkdir(dir)

namespace InvestSphere {
class Mutex {
public:
    Mutex() { InitializeCriticalSection(&m_cs); }
    ~Mutex() { DeleteCriticalSection(&m_cs); }
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    void lock() { EnterCriticalSection(&m_cs); }
    void unlock() { LeaveCriticalSection(&m_cs); }
private:
    CRITICAL_SECTION m_cs;
};

template <typename M>
class LockGuard {
public:
    explicit LockGuard(M &m) : m_(m) { m_.lock(); }
    ~LockGuard() { m_.unlock(); }
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
private:
    M &m_;
};
} // namespace InvestSphere

#else
#  include <mutex>
#  include <sys/stat.h>
#  include <sys/types.h>
#  define InvestSphere_mkdir(dir) mkdir(dir, 0755)

namespace InvestSphere {
using Mutex = std::mutex;
template <typename M>
using LockGuard = std::lock_guard<M>;
} // namespace InvestSphere
#endif


namespace InvestSphere {
namespace Utils {

inline bool ensureDirectoryExists(const std::string &path)
{
    if (path.empty()) return true;
    std::string current;
    for (size_t i = 0; i < path.size(); ++i) {
        char c = path[i];
        if (c == '/' || c == '\\') {
            if (!current.empty() && current != "." && current != "..") {
                InvestSphere_mkdir(current.c_str());
            }
        }
        current += c;
    }
    return true;
}

} // namespace Utils
} // namespace InvestSphere

#endif // INVESTSPHERE_COMPAT_H
