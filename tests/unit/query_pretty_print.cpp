#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "database/single_node/graph_db_accessor.hpp"
#include "query/frontend/ast/ast.hpp"
#include "query/frontend/ast/pretty_print.hpp"
#include "query_common.hpp"
#include "utils/string.hpp"

using namespace query;
using query::test_common::ToList;
using query::test_common::ToString;
using testing::ElementsAre;
using testing::UnorderedElementsAre;

namespace {

struct ExpressionPrettyPrinterTest : public ::testing::Test {
  ExpressionPrettyPrinterTest() : pdba{db.Access()}, dba{*pdba} {}

  database::GraphDb db;
  std::unique_ptr<database::GraphDbAccessor> pdba;
  database::GraphDbAccessor &dba;
  AstStorage storage;
};

TEST_F(ExpressionPrettyPrinterTest, Literals) {
  // 1
  EXPECT_EQ(ToString(LITERAL(1)), "1");

  // "hello"
  EXPECT_EQ(ToString(LITERAL("hello")), "\"hello\"");

  // null
  EXPECT_EQ(ToString(LITERAL(TypedValue::Null)), "null");

  // true
  EXPECT_EQ(ToString(LITERAL(true)), "true");

  // false
  EXPECT_EQ(ToString(LITERAL(false)), "false");

  // [1 null "hello"]
  EXPECT_EQ(ToString(LITERAL(
                (std::vector<PropertyValue>{1, PropertyValue::Null, "hello"}))),
            "[1, null, \"hello\"]");

  // {hello: 1, there: 2}
  EXPECT_EQ(ToString(LITERAL((std::map<std::string, PropertyValue>{
                {"hello", 1}, {"there", 2}}))),
            "{\"hello\": 1, \"there\": 2}");
}

TEST_F(ExpressionPrettyPrinterTest, UnaryOperators) {
  // not(false)
  EXPECT_EQ(ToString(NOT(LITERAL(false))), "(Not false)");

  // +1
  EXPECT_EQ(ToString(UPLUS(LITERAL(1))), "(+ 1)");

  // -1
  EXPECT_EQ(ToString(UMINUS(LITERAL(1))), "(- 1)");

  // null IS NULL
  EXPECT_EQ(ToString(IS_NULL(LITERAL(TypedValue::Null))), "(IsNull null)");
}

TEST_F(ExpressionPrettyPrinterTest, BinaryOperators) {
  // and(null, 5)
  EXPECT_EQ(ToString(AND(LITERAL(TypedValue::Null), LITERAL(5))),
            "(And null 5)");

  // or(5, {hello: "there"}["hello"])
  EXPECT_EQ(
      ToString(OR(LITERAL(5),
                  PROPERTY_LOOKUP(MAP(std::make_pair(PROPERTY_PAIR("hello"),
                                                     LITERAL("there"))),
                                  PROPERTY_PAIR("hello")))),
      "(Or 5 (PropertyLookup {\"hello\": \"there\"} \"hello\"))");

  // and(coalesce(null, 1), {hello: "there"})
  EXPECT_EQ(ToString(AND(
                COALESCE(LITERAL(TypedValue::Null), LITERAL(1)),
                MAP(std::make_pair(PROPERTY_PAIR("hello"), LITERAL("there"))))),
            "(And (Coalesce [null, 1]) {\"hello\": \"there\"})");
}

TEST_F(ExpressionPrettyPrinterTest, Coalesce) {
  // coalesce()
  EXPECT_EQ(ToString(COALESCE()), "(Coalesce [])");

  // coalesce(null, null)
  EXPECT_EQ(
      ToString(COALESCE(LITERAL(TypedValue::Null), LITERAL(TypedValue::Null))),
      "(Coalesce [null, null])");

  // coalesce(null, 2, 3)
  EXPECT_EQ(
      ToString(COALESCE(LITERAL(TypedValue::Null), LITERAL(2), LITERAL(3))),
      "(Coalesce [null, 2, 3])");

  // coalesce(null, 2, assert(false), 3)
  EXPECT_EQ(ToString(COALESCE(LITERAL(TypedValue::Null), LITERAL(2),
                              FN("ASSERT", LITERAL(false)), LITERAL(3))),
            "(Coalesce [null, 2, (Function \"ASSERT\" [false]), 3])");

  // coalesce(null, assert(false))
  EXPECT_EQ(ToString(COALESCE(LITERAL(TypedValue::Null),
                              FN("ASSERT", LITERAL(false)))),
            "(Coalesce [null, (Function \"ASSERT\" [false])])");

  // coalesce([null, null])
  EXPECT_EQ(ToString(COALESCE(LITERAL(TypedValue(
                std::vector<TypedValue>{TypedValue::Null, TypedValue::Null})))),
            "(Coalesce [[null, null]])");
}

TEST_F(ExpressionPrettyPrinterTest, ParameterLookup) {
  // and($hello, $there)
  EXPECT_EQ(ToString(AND(PARAMETER_LOOKUP(1), PARAMETER_LOOKUP(2))),
            "(And (ParameterLookup 1) (ParameterLookup 2))");
}

TEST_F(ExpressionPrettyPrinterTest, PropertyLookup) {
  // {hello: "there"}["hello"]
  EXPECT_EQ(ToString(PROPERTY_LOOKUP(
                MAP(std::make_pair(PROPERTY_PAIR("hello"), LITERAL("there"))),
                PROPERTY_PAIR("hello"))),
            "(PropertyLookup {\"hello\": \"there\"} \"hello\")");
}

TEST_F(ExpressionPrettyPrinterTest, NamedExpression) {
  // n AS 1
  EXPECT_EQ(ToString(NEXPR("n", LITERAL(1))), "(NamedExpression \"n\" 1)");
}

}  // namespace