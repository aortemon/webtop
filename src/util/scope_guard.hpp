#include <type_traits>
#include <utility>

namespace webtop::util {

template <typename Func> class ScopeGuard {
  bool active_{true};
  Func deleter_;

public:
  template <typename F>
  explicit ScopeGuard(F &&deleter) noexcept(
      std::is_nothrow_constructible_v<Func, F>)
    requires(!std::is_same_v<std::decay_t<F>, ScopeGuard>)
      : deleter_(std::forward<F>(deleter)) {}
  ScopeGuard(const ScopeGuard &) = delete;
  ScopeGuard(ScopeGuard &&other) noexcept
      : active_(other.active_), deleter_(std::move(other.deleter_)) {
    other.Disable();
  };
  ScopeGuard &operator=(const ScopeGuard &) = delete;
  ScopeGuard &operator=(ScopeGuard &&) = delete;
  ~ScopeGuard() {
    if (active_) {
      deleter_();
    }
  }
  void Disable() { active_ = false; }
};

template <typename Func> auto MakeScopeGuard(Func &&f) {
  return ScopeGuard<std::decay_t<Func>>(std::forward<Func>(f));
}

template <typename T> void Free(T *ptr) {
  free(const_cast<std::remove_const_t<T> *>(ptr));
}
} // namespace webtop::util