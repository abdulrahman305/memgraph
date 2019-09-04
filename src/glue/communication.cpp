#include "glue/communication.hpp"

#include <map>
#include <string>
#include <vector>

#include "database/graph_db_accessor.hpp"

using communication::bolt::Value;

namespace glue {

query::TypedValue ToTypedValue(const Value &value) {
  switch (value.type()) {
    case Value::Type::Null:
      return query::TypedValue();
    case Value::Type::Bool:
      return query::TypedValue(value.ValueBool());
    case Value::Type::Int:
      return query::TypedValue(value.ValueInt());
    case Value::Type::Double:
      return query::TypedValue(value.ValueDouble());
    case Value::Type::String:
      return query::TypedValue(value.ValueString());
    case Value::Type::List: {
      std::vector<query::TypedValue> list;
      list.reserve(value.ValueList().size());
      for (const auto &v : value.ValueList()) list.push_back(ToTypedValue(v));
      return query::TypedValue(std::move(list));
    }
    case Value::Type::Map: {
      std::map<std::string, query::TypedValue> map;
      for (const auto &kv : value.ValueMap())
        map.emplace(kv.first, ToTypedValue(kv.second));
      return query::TypedValue(std::move(map));
    }
    case Value::Type::Vertex:
    case Value::Type::Edge:
    case Value::Type::UnboundedEdge:
    case Value::Type::Path:
      throw communication::bolt::ValueException(
          "Unsupported conversion from Value to TypedValue");
  }
}

Value ToBoltValue(const query::TypedValue &value, storage::View view) {
  switch (value.type()) {
    case query::TypedValue::Type::Null:
      return Value();
    case query::TypedValue::Type::Bool:
      return Value(value.ValueBool());
    case query::TypedValue::Type::Int:
      return Value(value.ValueInt());
    case query::TypedValue::Type::Double:
      return Value(value.ValueDouble());
    case query::TypedValue::Type::String:
      return Value(std::string(value.ValueString()));
    case query::TypedValue::Type::List: {
      std::vector<Value> values;
      values.reserve(value.ValueList().size());
      for (const auto &v : value.ValueList()) {
        values.push_back(ToBoltValue(v, view));
      }
      return Value(std::move(values));
    }
    case query::TypedValue::Type::Map: {
      std::map<std::string, Value> map;
      for (const auto &kv : value.ValueMap()) {
        map.emplace(kv.first, ToBoltValue(kv.second, view));
      }
      return Value(std::move(map));
    }
    case query::TypedValue::Type::Vertex:
      return Value(ToBoltVertex(value.ValueVertex(), view));
    case query::TypedValue::Type::Edge:
      return Value(ToBoltEdge(value.ValueEdge(), view));
    case query::TypedValue::Type::Path:
      return Value(ToBoltPath(value.ValuePath(), view));
  }
}

communication::bolt::Vertex ToBoltVertex(const VertexAccessor &vertex,
                                         storage::View view) {
  // NOTE: This hack will be removed when we switch to storage v2 API.
  switch (view) {
    case storage::View::OLD:
      const_cast<VertexAccessor &>(vertex).SwitchOld();
      break;
    case storage::View::NEW:
      const_cast<VertexAccessor &>(vertex).SwitchNew();
      break;
  }
  auto id = communication::bolt::Id::FromUint(vertex.gid().AsUint());
  std::vector<std::string> labels;
  labels.reserve(vertex.labels().size());
  for (const auto &label : vertex.labels()) {
    labels.push_back(vertex.db_accessor().LabelName(label));
  }
  std::map<std::string, Value> properties;
  for (const auto &prop : vertex.Properties()) {
    properties[vertex.db_accessor().PropertyName(prop.first)] =
        ToBoltValue(prop.second);
  }
  return communication::bolt::Vertex{id, std::move(labels),
                                     std::move(properties)};
}

communication::bolt::Edge ToBoltEdge(const EdgeAccessor &edge,
                                     storage::View view) {
  // NOTE: This hack will be removed when we switch to storage v2 API.
  switch (view) {
    case storage::View::OLD:
      const_cast<EdgeAccessor &>(edge).SwitchOld();
      break;
    case storage::View::NEW:
      const_cast<EdgeAccessor &>(edge).SwitchNew();
      break;
  }
  auto id = communication::bolt::Id::FromUint(edge.gid().AsUint());
  auto from = communication::bolt::Id::FromUint(edge.from().gid().AsUint());
  auto to = communication::bolt::Id::FromUint(edge.to().gid().AsUint());
  auto type = edge.db_accessor().EdgeTypeName(edge.EdgeType());
  std::map<std::string, Value> properties;
  for (const auto &prop : edge.Properties()) {
    properties[edge.db_accessor().PropertyName(prop.first)] =
        ToBoltValue(prop.second);
  }
  return communication::bolt::Edge{id, from, to, type, std::move(properties)};
}

communication::bolt::Path ToBoltPath(const query::Path &path,
                                     storage::View view) {
  std::vector<communication::bolt::Vertex> vertices;
  vertices.reserve(path.vertices().size());
  for (const auto &v : path.vertices()) {
    vertices.push_back(ToBoltVertex(v, view));
  }
  std::vector<communication::bolt::Edge> edges;
  edges.reserve(path.edges().size());
  for (const auto &e : path.edges()) {
    edges.push_back(ToBoltEdge(e, view));
  }
  return communication::bolt::Path(vertices, edges);
}

PropertyValue ToPropertyValue(const Value &value) {
  switch (value.type()) {
    case Value::Type::Null:
      return PropertyValue();
    case Value::Type::Bool:
      return PropertyValue(value.ValueBool());
    case Value::Type::Int:
      return PropertyValue(value.ValueInt());
    case Value::Type::Double:
      return PropertyValue(value.ValueDouble());
    case Value::Type::String:
      return PropertyValue(value.ValueString());
    case Value::Type::List: {
      std::vector<PropertyValue> vec;
      vec.reserve(value.ValueList().size());
      for (const auto &value : value.ValueList())
        vec.emplace_back(ToPropertyValue(value));
      return PropertyValue(std::move(vec));
    }
    case Value::Type::Map: {
      std::map<std::string, PropertyValue> map;
      for (const auto &kv : value.ValueMap())
        map.emplace(kv.first, ToPropertyValue(kv.second));
      return PropertyValue(std::move(map));
    }
    case Value::Type::Vertex:
    case Value::Type::Edge:
    case Value::Type::UnboundedEdge:
    case Value::Type::Path:
      throw communication::bolt::ValueException(
          "Unsupported conversion from Value to PropertyValue");
  }
}

Value ToBoltValue(const PropertyValue &value) {
  switch (value.type()) {
    case PropertyValue::Type::Null:
      return Value();
    case PropertyValue::Type::Bool:
      return Value(value.ValueBool());
    case PropertyValue::Type::Int:
      return Value(value.ValueInt());
      break;
    case PropertyValue::Type::Double:
      return Value(value.ValueDouble());
    case PropertyValue::Type::String:
      return Value(value.ValueString());
    case PropertyValue::Type::List: {
      const auto &values = value.ValueList();
      std::vector<Value> vec;
      vec.reserve(values.size());
      for (const auto &v : values) {
        vec.push_back(ToBoltValue(v));
      }
      return Value(std::move(vec));
    }
    case PropertyValue::Type::Map: {
      const auto &map = value.ValueMap();
      std::map<std::string, Value> dv_map;
      for (const auto &kv : map) {
        dv_map.emplace(kv.first, ToBoltValue(kv.second));
      }
      return Value(std::move(dv_map));
    }
  }
}

}  // namespace glue
