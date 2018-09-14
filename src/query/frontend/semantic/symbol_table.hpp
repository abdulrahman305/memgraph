#pragma once

#include <map>
#include <string>

#include "query/frontend/ast/ast.hpp"
#include "query/frontend/semantic/symbol.capnp.h"
#include "query/frontend/semantic/symbol.hpp"

namespace query {

class SymbolTable final {
 public:
  SymbolTable() {}
  Symbol CreateSymbol(const std::string &name, bool user_declared,
                      Symbol::Type type = Symbol::Type::Any,
                      int token_position = -1) {
    int position = position_++;
    return Symbol(name, position, user_declared, type, token_position);
  }

  auto &operator[](const Tree &tree) { return table_[tree.uid()]; }

  Symbol &at(const Tree &tree) { return table_.at(tree.uid()); }
  const Symbol &at(const Tree &tree) const { return table_.at(tree.uid()); }

  int max_position() const { return position_; }

  const auto &table() const { return table_; }

  void Load(const capnp::SymbolTable::Reader &reader) {
    position_ = reader.getPosition();
    table_.clear();
    for (const auto &entry_reader : reader.getTable()) {
      int key = entry_reader.getKey();
      Symbol val;
      val.Load(entry_reader.getVal());
      table_[key] = val;
    }
  }

 private:
  int position_{0};
  std::map<int, Symbol> table_;
};

inline void Save(const SymbolTable &symbol_table,
                 capnp::SymbolTable::Builder *builder) {
  builder->setPosition(symbol_table.max_position());
  auto list_builder = builder->initTable(symbol_table.table().size());
  size_t i = 0;
  for (const auto &entry : symbol_table.table()) {
    auto entry_builder = list_builder[i++];
    entry_builder.setKey(entry.first);
    auto sym_builder = entry_builder.initVal();
    Save(entry.second, &sym_builder);
  }
}

}  // namespace query
