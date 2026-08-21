#ifndef CONTAINER_H
#define CONTAINER_H
#include <algorithm>
#include <array>
#include <optional>
#include <utility>

template <class Key, class Value>
struct Entry {
  Key key;
  Value value;
  constexpr Entry(Key k, Value v) : key(std::move(k)), value(std::move(v)) {}
};

template <class Key, class Value, size_t N>
/// Compile Time Hashmap that stores Key->Value pairs as a sorted array
class StaticMap {
 public:
  constexpr explicit StaticMap(
      Entry<Key, Value> (&&items)[N])  // NOLINT(modernize-avoid-c-arrays)
      : items_(std::to_array(items)) {
    std::ranges::sort(items_, {}, &Entry<Key, Value>::key);
    if (std::ranges::adjacent_find(items_, {}, &Entry<Key, Value>::key) !=
        items_.end()) {
      throw std::logic_error("Map initializer must not contain duplicate keys");
    }
  }

  [[nodiscard]] constexpr auto find(const Key& key) const
      -> std::optional<std::reference_wrapper<const Value>> {
    auto it =
        std::ranges::lower_bound(items_, key, {}, &Entry<Key, Value>::key);
    if (it != items_.end() && it->key == key) return std::cref(it->value);
    return std::nullopt;
  }

  [[nodiscard]] constexpr auto contains(const Key& key) const -> bool {
    return find(key).has_value();
  }

 private:
  std::array<Entry<Key, Value>, N> items_;
};

template <class Value, size_t N>
class StaticSet {
 public:
  constexpr explicit StaticSet(
      Value (&&items)[N])  // NOLINT(modernize-avoid-c-arrays)
      : items_(std::to_array(items)) {
    std::ranges::sort(items_);
    if (std::ranges::adjacent_find(items_, {}, {}) != items_.end()) {
      throw std::logic_error("Set initializer must not contain duplicates");
    }
  }

  [[nodiscard]] constexpr auto find(const Value& value) const
      -> std::optional<std::reference_wrapper<const Value>> {
    auto it = std::ranges::lower_bound(items_, value);

    if (it != items_.end() && *it == value) return std::cref(*it);
    return std::nullopt;
  }

  [[nodiscard]] constexpr auto contains(const Value& key) const -> bool {
    return find(key).has_value();
  }

 private:
  std::array<Value, N> items_;
};

template <class Key, class Value, size_t N>
constexpr auto to_static_map(
    Entry<Key, Value> (&&items)[N]) {  // NOLINT(modernize-avoid-c-arrays)
  return StaticMap<Key, Value, N>(std::move(items));
}

template <class Value, size_t N>
constexpr auto to_static_set(
    Value (&&items)[N]) {  // NOLINT(modernize-avoid-c-arrays)
  return StaticSet<Value, N>(std::move(items));
}

#endif  // CONTAINER_H
