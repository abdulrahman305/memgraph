#pragma once

#include <experimental/optional>

#include "distributed/serialization.capnp.h"
#include "query/typed_value.hpp"
#include "storage/edge.hpp"
#include "storage/vertex.hpp"
#include "utils/exceptions.hpp"
#include "utils/serialization.capnp.h"

namespace utils {

inline void SaveCapnpTypedValue(
    const query::TypedValue &value,
    distributed::capnp::TypedValue::Builder *builder,
    std::function<void(const query::TypedValue &,
                       distributed::capnp::TypedValue::Builder *)>
        save_graph_element = nullptr) {
  switch (value.type()) {
    case query::TypedValue::Type::Null:
      builder->setNullType();
      return;
    case query::TypedValue::Type::Bool:
      builder->setBool(value.Value<bool>());
      return;
    case query::TypedValue::Type::Int:
      builder->setInteger(value.Value<int64_t>());
      return;
    case query::TypedValue::Type::Double:
      builder->setDouble(value.Value<double>());
      return;
    case query::TypedValue::Type::String:
      builder->setString(value.Value<std::string>());
      return;
    case query::TypedValue::Type::List: {
      const auto &values = value.Value<std::vector<query::TypedValue>>();
      auto list_builder = builder->initList(values.size());
      for (size_t i = 0; i < values.size(); ++i) {
        auto value_builder = list_builder[i];
        SaveCapnpTypedValue(values[i], &value_builder, save_graph_element);
      }
      return;
    }
    case query::TypedValue::Type::Map: {
      const auto &map = value.Value<std::map<std::string, query::TypedValue>>();
      auto map_builder = builder->initMap(map.size());
      size_t i = 0;
      for (const auto &kv : map) {
        auto kv_builder = map_builder[i];
        kv_builder.setKey(kv.first);
        auto value_builder = kv_builder.initValue();
        SaveCapnpTypedValue(kv.second, &value_builder, save_graph_element);
        ++i;
      }
      return;
    }
    case query::TypedValue::Type::Vertex:
    case query::TypedValue::Type::Edge:
    case query::TypedValue::Type::Path:
      if (save_graph_element) {
        save_graph_element(value, builder);
      } else {
        throw utils::BasicException(
            "Unable to serialize TypedValue of type: {}", value.type());
      }
  }
}

inline void LoadCapnpTypedValue(
    const distributed::capnp::TypedValue::Reader &reader,
    query::TypedValue *value,
    std::function<void(const distributed::capnp::TypedValue::Reader &,
                       query::TypedValue *)>
        load_graph_element = nullptr) {
  switch (reader.which()) {
    case distributed::capnp::TypedValue::NULL_TYPE:
      *value = query::TypedValue::Null;
      return;
    case distributed::capnp::TypedValue::BOOL:
      *value = reader.getBool();
      return;
    case distributed::capnp::TypedValue::INTEGER:
      *value = reader.getInteger();
      return;
    case distributed::capnp::TypedValue::DOUBLE:
      *value = reader.getDouble();
      return;
    case distributed::capnp::TypedValue::STRING:
      *value = reader.getString().cStr();
      return;
    case distributed::capnp::TypedValue::LIST: {
      std::vector<query::TypedValue> list;
      list.reserve(reader.getList().size());
      for (const auto &value_reader : reader.getList()) {
        list.emplace_back();
        LoadCapnpTypedValue(value_reader, &list.back(), load_graph_element);
      }
      *value = list;
      return;
    }
    case distributed::capnp::TypedValue::MAP: {
      std::map<std::string, query::TypedValue> map;
      for (const auto &kv_reader : reader.getMap()) {
        auto key = kv_reader.getKey().cStr();
        LoadCapnpTypedValue(kv_reader.getValue(), &map[key],
                            load_graph_element);
      }
      *value = map;
      return;
    }
    case distributed::capnp::TypedValue::VERTEX:
    case distributed::capnp::TypedValue::EDGE:
    case distributed::capnp::TypedValue::PATH:
      if (load_graph_element) {
        load_graph_element(reader, value);
      } else {
        throw utils::BasicException(
            "Unexpected TypedValue type '{}' when loading from archive",
            reader.which());
      }
  }
}

template <typename T>
inline void SaveVector(const std::vector<T> &data,
                       typename ::capnp::List<T>::Builder *list_builder) {
  for (size_t i = 0; i < data.size(); ++i) {
    list_builder->set(i, data[i]);
  }
}

inline void SaveVector(const std::vector<std::string> &data,
                       ::capnp::List<::capnp::Text>::Builder *list_builder) {
  for (size_t i = 0; i < data.size(); ++i) {
    list_builder->set(i, data[i]);
  }
}

template <typename T>
inline void LoadVector(std::vector<T> *data,
                       const typename ::capnp::List<T>::Reader &list_reader) {
  for (const auto e : list_reader) {
    data->emplace_back(e);
  }
}

inline void LoadVector(
    std::vector<std::string> *data,
    const typename ::capnp::List<::capnp::Text>::Reader &list_reader) {
  for (const auto e : list_reader) {
    data->emplace_back(e);
  }
}

template <typename TCapnp, typename T>
inline void SaveVector(
    const std::vector<T> &data,
    typename ::capnp::List<TCapnp>::Builder *list_builder,
    const std::function<void(typename TCapnp::Builder *, const T &)> &save) {
  for (size_t i = 0; i < data.size(); ++i) {
    auto elem_builder = (*list_builder)[i];
    save(&elem_builder, data[i]);
  }
}

template <typename TCapnp, typename T>
inline void LoadVector(
    std::vector<T> *data,
    const typename ::capnp::List<TCapnp>::Reader &list_reader,
    const std::function<T(const typename TCapnp::Reader &reader)> &load) {
  for (const auto reader : list_reader) {
    data->emplace_back(load(reader));
  }
}

template <class TCapnpKey, class TCapnpValue, class TMap>
void SaveMap(const TMap &map,
             typename capnp::Map<TCapnpKey, TCapnpValue>::Builder *map_builder,
             std::function<void(
                 typename capnp::Map<TCapnpKey, TCapnpValue>::Entry::Builder *,
                 const typename TMap::value_type &)>
                 save) {
  auto entries_builder = map_builder->initEntries(map.size());
  size_t i = 0;
  for (const auto &entry : map) {
    auto entry_builder = entries_builder[i];
    save(&entry_builder, entry);
    ++i;
  }
}

template <class TCapnpKey, class TCapnpValue, class TMap>
void LoadMap(
    TMap *map,
    const typename capnp::Map<TCapnpKey, TCapnpValue>::Reader &map_reader,
    std::function<typename TMap::value_type(
        const typename capnp::Map<TCapnpKey, TCapnpValue>::Entry::Reader &)>
        load) {
  for (const auto &entry_reader : map_reader.getEntries()) {
    map->insert(load(entry_reader));
  }
}

template <typename TCapnp, typename T>
inline void SaveOptional(
    const std::experimental::optional<T> &data,
    typename capnp::Optional<TCapnp>::Builder *builder,
    const std::function<void(typename TCapnp::Builder *, const T &)> &save) {
  if (data) {
    auto value_builder = builder->initValue();
    save(&value_builder, data.value());
  } else {
    builder->setNullopt();
  }
}

template <typename TCapnp, typename T>
inline std::experimental::optional<T> LoadOptional(
    const typename capnp::Optional<TCapnp>::Reader &reader,
    const std::function<T(const typename TCapnp::Reader &reader)> &load) {
  switch (reader.which()) {
    case capnp::Optional<TCapnp>::NULLOPT:
      return std::experimental::nullopt;
    case capnp::Optional<TCapnp>::VALUE:
      auto value_reader = reader.getValue();
      return std::experimental::optional<T>{load(value_reader)};
  }
}

template <typename TCapnp, typename T>
inline void SaveUniquePtr(
    const std::unique_ptr<T> &data,
    typename capnp::UniquePtr<TCapnp>::Builder *builder,
    const std::function<void(typename TCapnp::Builder *, const T &)> &save) {
  if (data) {
    auto value_builder = builder->initValue();
    save(&value_builder, *data);
  } else {
    builder->setNullptr();
  }
}

template <typename TCapnp, typename T>
inline std::unique_ptr<T> LoadUniquePtr(
    const typename capnp::UniquePtr<TCapnp>::Reader &reader,
    const std::function<T *(const typename TCapnp::Reader &reader)> &load) {
  switch (reader.which()) {
    case capnp::UniquePtr<TCapnp>::NULLPTR:
      return nullptr;
    case capnp::UniquePtr<TCapnp>::VALUE:
      auto value_reader = reader.getValue();
      return std::unique_ptr<T>(load(value_reader));
  }
}

template <typename TCapnp, typename T>
inline void SaveSharedPtr(
    const std::shared_ptr<T> &data,
    typename capnp::SharedPtr<TCapnp>::Builder *builder,
    const std::function<void(typename TCapnp::Builder *, const T &)> &save,
    std::vector<T *> *saved_pointers) {
  if (!data) {
    builder->setNullptr();
    return;
  }
  auto entry_builder = builder->initEntry();
  auto pointer_id = reinterpret_cast<uintptr_t>(data.get());
  CHECK(pointer_id <= std::numeric_limits<uint64_t>::max());
  entry_builder.setId(pointer_id);
  if (utils::Contains(*saved_pointers, data.get())) {
    return;
  }
  auto value_builder = entry_builder.initValue();
  save(&value_builder, *data);
  saved_pointers->emplace_back(data.get());
}

template <typename TCapnp, typename T>
std::shared_ptr<T> LoadSharedPtr(
    const typename capnp::SharedPtr<TCapnp>::Reader &reader,
    const std::function<T *(const typename TCapnp::Reader &reader)> &load,
    std::vector<std::pair<uint64_t, std::shared_ptr<T>>> *loaded_pointers) {
  std::shared_ptr<T> ret;
  switch (reader.which()) {
    case capnp::SharedPtr<TCapnp>::NULLPTR:
      ret = nullptr;
      break;
    case capnp::SharedPtr<TCapnp>::ENTRY:
      auto entry_reader = reader.getEntry();
      uint64_t pointer_id = entry_reader.getId();
      auto found =
          std::find_if(loaded_pointers->begin(), loaded_pointers->end(),
                       [pointer_id](const auto &e) -> bool {
                         return e.first == pointer_id;
                       });
      if (found != loaded_pointers->end()) return found->second;
      auto value_reader = entry_reader.getValue();
      ret = std::shared_ptr<T>(load(value_reader));
      loaded_pointers->emplace_back(std::make_pair(pointer_id, ret));
  }
  return ret;
}

}  // namespace utils
