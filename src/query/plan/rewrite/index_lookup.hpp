/// @file
/// This file provides a plan rewriter which replaces `Filter` and `ScanAll`
/// operations with `ScanAllBy<Index>` if possible. The public entrypoint is
/// `RewriteWithIndexLookup`.

#pragma once

#include <algorithm>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <gflags/gflags.h>

#include "query/plan/operator.hpp"
#include "query/plan/preprocess.hpp"

DECLARE_int64(query_vertex_count_to_expand_existing);

namespace query::plan {

namespace impl {

// Return the new root expression after removing the given expressions from the
// given expression tree.
Expression *RemoveAndExpressions(
    Expression *expr, const std::unordered_set<Expression *> &exprs_to_remove);

template <class TDbAccessor>
class IndexLookupRewriter final : public HierarchicalLogicalOperatorVisitor {
 public:
  IndexLookupRewriter(const SymbolTable *symbol_table, AstStorage *ast_storage,
                      TDbAccessor *db)
      : symbol_table_(symbol_table), ast_storage_(ast_storage), db_(db) {}

  using HierarchicalLogicalOperatorVisitor::PostVisit;
  using HierarchicalLogicalOperatorVisitor::PreVisit;
  using HierarchicalLogicalOperatorVisitor::Visit;

  bool Visit(Once &) override { return true; }

  bool PreVisit(Filter &op) override {
    prev_ops_.push_back(&op);
    filters_.CollectFilterExpression(op.expression_, *symbol_table_);
    return true;
  }

  // Remove no longer needed Filter in PostVisit, this should be the last thing
  // Filter::Accept does, so it should be safe to remove the last reference and
  // free the memory.
  bool PostVisit(Filter &op) override {
    prev_ops_.pop_back();
    op.expression_ =
        RemoveAndExpressions(op.expression_, filter_exprs_for_removal_);
    if (!op.expression_ ||
        utils::Contains(filter_exprs_for_removal_, op.expression_)) {
      SetOnParent(op.input());
    }
    return true;
  }

  bool PreVisit(ScanAll &op) override {
    prev_ops_.push_back(&op);
    return true;
  }

  // Replace ScanAll with ScanAllBy<Index> in PostVisit, because removal of
  // ScanAll may remove the last reference and thus free the memory. PostVisit
  // should be the last thing ScanAll::Accept does, so it should be safe.
  bool PostVisit(ScanAll &scan) override {
    prev_ops_.pop_back();
    auto indexed_scan = GenScanByIndex(scan);
    if (indexed_scan) {
      SetOnParent(std::move(indexed_scan));
    }
    return true;
  }

  bool PreVisit(Expand &op) override {
    prev_ops_.push_back(&op);
    return true;
  }

  // See if it might be better to do ScanAllBy<Index> of the destination and
  // then do Expand to existing.
  bool PostVisit(Expand &expand) override {
    prev_ops_.pop_back();
    if (expand.common_.existing_node) {
      return true;
    }
    ScanAll dst_scan(expand.input(), expand.common_.node_symbol,
                     expand.graph_view_);
    auto indexed_scan =
        GenScanByIndex(dst_scan, FLAGS_query_vertex_count_to_expand_existing);
    if (indexed_scan) {
      expand.set_input(std::move(indexed_scan));
      expand.common_.existing_node = true;
    }
    return true;
  }

  // The following operators may only use index lookup in filters inside of
  // their own branches. So we handle them all the same.
  //  * Input operator is visited with the current visitor.
  //  * Custom operator branches are visited with a new visitor.

  bool PreVisit(Merge &op) override {
    prev_ops_.push_back(&op);
    op.input()->Accept(*this);
    RewriteBranch(&op.merge_match_);
    return false;
  }

  bool PostVisit(Merge &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(Optional &op) override {
    prev_ops_.push_back(&op);
    op.input()->Accept(*this);
    RewriteBranch(&op.optional_);
    return false;
  }

  bool PostVisit(Optional &) override {
    prev_ops_.pop_back();
    return true;
  }

  // Rewriting Cartesian assumes that the input plan will have Filter operations
  // as soon as they are possible. Therefore we do not track filters above
  // Cartesian because they should be irrelevant.
  //
  // For example, the following plan is not expected to be an input to
  // IndexLookupRewriter.
  //
  // Filter n.prop = 16
  // |
  // Cartesian
  // |
  // |\
  // | ScanAll (n)
  // |
  // ScanAll (m)
  //
  // Instead, the equivalent set of operations should be done this way:
  //
  // Cartesian
  // |
  // |\
  // | Filter n.prop = 16
  // | |
  // | ScanAll (n)
  // |
  // ScanAll (m)
  bool PreVisit(Cartesian &op) override {
    prev_ops_.push_back(&op);
    RewriteBranch(&op.left_op_);
    RewriteBranch(&op.right_op_);
    return false;
  }

  bool PostVisit(Cartesian &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(Union &op) override {
    prev_ops_.push_back(&op);
    RewriteBranch(&op.left_op_);
    RewriteBranch(&op.right_op_);
    return false;
  }

  bool PostVisit(Union &) override {
    prev_ops_.pop_back();
    return true;
  }

  // The remaining operators should work by just traversing into their input.

  bool PreVisit(CreateNode &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(CreateNode &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(CreateExpand &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(CreateExpand &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(ScanAllByLabel &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(ScanAllByLabel &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(ScanAllByLabelPropertyRange &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(ScanAllByLabelPropertyRange &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(ScanAllByLabelPropertyValue &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(ScanAllByLabelPropertyValue &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(ExpandVariable &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(ExpandVariable &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(ConstructNamedPath &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(ConstructNamedPath &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(Produce &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(Produce &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(Delete &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(Delete &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(SetProperty &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(SetProperty &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(SetProperties &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(SetProperties &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(SetLabels &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(SetLabels &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(RemoveProperty &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(RemoveProperty &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(RemoveLabels &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(RemoveLabels &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(EdgeUniquenessFilter &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(EdgeUniquenessFilter &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(Accumulate &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(Accumulate &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(Aggregate &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(Aggregate &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(Skip &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(Skip &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(Limit &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(Limit &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(OrderBy &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(OrderBy &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(Unwind &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(Unwind &) override {
    prev_ops_.pop_back();
    return true;
  }

  bool PreVisit(Distinct &op) override {
    prev_ops_.push_back(&op);
    return true;
  }
  bool PostVisit(Distinct &) override {
    prev_ops_.pop_back();
    return true;
  }

  std::shared_ptr<LogicalOperator> new_root_;

 private:
  const SymbolTable *symbol_table_;
  AstStorage *ast_storage_;
  TDbAccessor *db_;
  Filters filters_;
  std::unordered_set<Expression *> filter_exprs_for_removal_;
  std::vector<LogicalOperator *> prev_ops_;

  struct LabelPropertyIndex {
    LabelIx label;
    // FilterInfo with PropertyFilter.
    FilterInfo filter;
    int64_t vertex_count;
  };

  bool DefaultPreVisit() override {
    throw utils::NotYetImplemented("optimizing index lookup");
  }

  void SetOnParent(const std::shared_ptr<LogicalOperator> &input) {
    CHECK(input);
    if (prev_ops_.empty()) {
      CHECK(!new_root_);
      new_root_ = input;
      return;
    }
    prev_ops_.back()->set_input(input);
  }

  void RewriteBranch(std::shared_ptr<LogicalOperator> *branch) {
    IndexLookupRewriter<TDbAccessor> rewriter(symbol_table_, ast_storage_, db_);
    (*branch)->Accept(rewriter);
    if (rewriter.new_root_) {
      *branch = rewriter.new_root_;
    }
  }

  storage::Label GetLabel(LabelIx label) { return db_->Label(label.name); }

  storage::Property GetProperty(PropertyIx prop) {
    return db_->Property(prop.name);
  }

  LabelIx FindBestLabelIndex(const std::unordered_set<LabelIx> &labels) {
    CHECK(!labels.empty())
        << "Trying to find the best label without any labels.";
    return *std::min_element(labels.begin(), labels.end(),
                             [this](const auto &label1, const auto &label2) {
                               return db_->VerticesCount(GetLabel(label1)) <
                                      db_->VerticesCount(GetLabel(label2));
                             });
  }

  // Finds the label-property combination which has indexed the lowest amount of
  // vertices. If the index cannot be found, nullopt is returned.
  std::optional<LabelPropertyIndex> FindBestLabelPropertyIndex(
      const Symbol &symbol, const std::unordered_set<Symbol> &bound_symbols) {
    auto are_bound = [&bound_symbols](const auto &used_symbols) {
      for (const auto &used_symbol : used_symbols) {
        if (!utils::Contains(bound_symbols, used_symbol)) {
          return false;
        }
      }
      return true;
    };
    std::optional<LabelPropertyIndex> found;
    for (const auto &label : filters_.FilteredLabels(symbol)) {
      for (const auto &filter : filters_.PropertyFilters(symbol)) {
        if (filter.property_filter->is_symbol_in_value_ ||
            !are_bound(filter.used_symbols)) {
          // Skip filter expressions which use the symbol whose property we are
          // looking up or aren't bound. We cannot scan by such expressions. For
          // example, in `n.a = 2 + n.b` both sides of `=` refer to `n`, so we
          // cannot scan `n` by property index.
          continue;
        }
        const auto &property = filter.property_filter->property_;
        if (!db_->LabelPropertyIndexExists(GetLabel(label),
                                           GetProperty(property))) {
          continue;
        }
        int64_t vertex_count =
            db_->VerticesCount(GetLabel(label), GetProperty(property));
        auto is_better_type = [&found](PropertyFilter::Type type) {
          // Order the types by the most preferred index lookup type.
          static const PropertyFilter::Type kFilterTypeOrder[] = {
              PropertyFilter::Type::EQUAL, PropertyFilter::Type::RANGE,
              PropertyFilter::Type::REGEX_MATCH};
          auto *found_sort_ix =
              std::find(kFilterTypeOrder, kFilterTypeOrder + 3,
                        found->filter.property_filter->type_);
          auto *type_sort_ix =
              std::find(kFilterTypeOrder, kFilterTypeOrder + 3, type);
          return type_sort_ix < found_sort_ix;
        };
        if (!found || vertex_count < found->vertex_count ||
            (vertex_count == found->vertex_count &&
             is_better_type(filter.property_filter->type_))) {
          found = LabelPropertyIndex{label, filter, vertex_count};
        }
      }
    }
    return found;
  }

  // Creates a ScanAll by the best possible index for the `node_symbol`. Best
  // index is defined as the index with least number of vertices. If the node
  // does not have at least a label, no indexed lookup can be created and
  // `nullptr` is returned. The operator is chained after `input`. Optional
  // `max_vertex_count` controls, whether no operator should be created if the
  // vertex count in the best index exceeds this number. In such a case,
  // `nullptr` is returned and `input` is not chained.
  std::unique_ptr<ScanAll> GenScanByIndex(
      const ScanAll &scan,
      const std::optional<int64_t> &max_vertex_count = std::nullopt) {
    const auto &input = scan.input();
    const auto &node_symbol = scan.output_symbol_;
    const auto &graph_view = scan.graph_view_;
    const auto labels = filters_.FilteredLabels(node_symbol);
    if (labels.empty()) {
      // Without labels, we cannot generate any indexed ScanAll.
      return nullptr;
    }
    // First, try to see if we can use label+property index. If not, use just
    // the label index (which ought to exist).
    const auto &modified_symbols = scan.ModifiedSymbols(*symbol_table_);
    std::unordered_set<Symbol> bound_symbols(modified_symbols.begin(),
                                             modified_symbols.end());
    auto found_index = FindBestLabelPropertyIndex(node_symbol, bound_symbols);
    if (found_index &&
        // Use label+property index if we satisfy max_vertex_count.
        (!max_vertex_count || *max_vertex_count >= found_index->vertex_count)) {
      // Copy the property filter and then erase it from filters.
      const auto prop_filter = *found_index->filter.property_filter;
      if (prop_filter.type_ != PropertyFilter::Type::REGEX_MATCH) {
        // Remove the original expression from Filter operation only if it's not
        // a regex match. In such a case we need to perform the matching even
        // after we've scanned the index.
        filter_exprs_for_removal_.insert(found_index->filter.expression);
      }
      filters_.EraseFilter(found_index->filter);
      std::vector<Expression *> removed_expressions;
      filters_.EraseLabelFilter(node_symbol, found_index->label,
                                &removed_expressions);
      filter_exprs_for_removal_.insert(removed_expressions.begin(),
                                       removed_expressions.end());
      if (prop_filter.lower_bound_ || prop_filter.upper_bound_) {
        return std::make_unique<ScanAllByLabelPropertyRange>(
            input, node_symbol, GetLabel(found_index->label),
            GetProperty(prop_filter.property_), prop_filter.property_.name,
            prop_filter.lower_bound_, prop_filter.upper_bound_, graph_view);
      } else if (prop_filter.type_ == PropertyFilter::Type::REGEX_MATCH) {
        // Generate index scan using the empty string as a lower bound.
        Expression *empty_string = ast_storage_->Create<PrimitiveLiteral>("");
        auto lower_bound = utils::MakeBoundInclusive(empty_string);
        return std::make_unique<ScanAllByLabelPropertyRange>(
            input, node_symbol, GetLabel(found_index->label),
            GetProperty(prop_filter.property_), prop_filter.property_.name,
            std::make_optional(lower_bound), std::nullopt, graph_view);
      } else {
        CHECK(prop_filter.value_) << "Property filter should either have "
                                     "bounds or a value expression.";
        return std::make_unique<ScanAllByLabelPropertyValue>(
            input, node_symbol, GetLabel(found_index->label),
            GetProperty(prop_filter.property_), prop_filter.property_.name,
            prop_filter.value_, graph_view);
      }
    }
    auto label = FindBestLabelIndex(labels);
    if (max_vertex_count &&
        db_->VerticesCount(GetLabel(label)) > *max_vertex_count) {
      // Don't create an indexed lookup, since we have more labeled vertices
      // than the allowed count.
      return nullptr;
    }
    std::vector<Expression *> removed_expressions;
    filters_.EraseLabelFilter(node_symbol, label, &removed_expressions);
    filter_exprs_for_removal_.insert(removed_expressions.begin(),
                                     removed_expressions.end());
    return std::make_unique<ScanAllByLabel>(input, node_symbol, GetLabel(label),
                                            graph_view);
  }
};

}  // namespace impl

template <class TDbAccessor>
std::unique_ptr<LogicalOperator> RewriteWithIndexLookup(
    std::unique_ptr<LogicalOperator> root_op, const SymbolTable &symbol_table,
    AstStorage *ast_storage, TDbAccessor *db) {
  impl::IndexLookupRewriter<TDbAccessor> rewriter(&symbol_table, ast_storage,
                                                  db);
  root_op->Accept(rewriter);
  if (rewriter.new_root_) {
    // This shouldn't happen in real use case, because IndexLookupRewriter
    // removes Filter operations and they cannot be the root op. In case we
    // somehow missed this, raise NotYetImplemented instead of CHECK crashing
    // the application.
    throw utils::NotYetImplemented("optimizing index lookup");
  }
  return root_op;
}

}  // namespace query::plan
