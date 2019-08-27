#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "storage/v2/storage.hpp"

using testing::UnorderedElementsAre;

// TODO: The point of these is not to test GC fully, these are just simple
// sanity checks. These will be superseded by a more sophisticated stress test
// which will verify that GC is working properly in a multithreaded environment.

// A simple test trying to get GC to run while a transaction is still alive and
// then verify that GC didn't delete anything it shouldn't have.
// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2Gc, Sanity) {
  storage::Storage storage(
      storage::StorageGcConfig{.type = storage::StorageGcConfig::Type::PERIODIC,
                               .interval = std::chrono::milliseconds(100)});

  std::vector<storage::Gid> vertices;

  {
    auto acc = storage.Access();
    // Create some vertices, but delete some of them immediately.
    for (uint64_t i = 0; i < 1000; ++i) {
      auto vertex = acc.CreateVertex();
      vertices.push_back(vertex.Gid());
    }

    acc.AdvanceCommand();

    for (uint64_t i = 0; i < 1000; ++i) {
      auto vertex = acc.FindVertex(vertices[i], storage::View::OLD);
      ASSERT_TRUE(vertex.has_value());
      if (i % 5 == 0) {
        EXPECT_FALSE(acc.DeleteVertex(&vertex.value()).HasError());
      }
    }

    // Wait for GC.
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    for (uint64_t i = 0; i < 1000; ++i) {
      auto vertex_old = acc.FindVertex(vertices[i], storage::View::OLD);
      auto vertex_new = acc.FindVertex(vertices[i], storage::View::NEW);
      EXPECT_TRUE(vertex_old.has_value());
      EXPECT_EQ(vertex_new.has_value(), i % 5 != 0);
    }

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Verify existing vertices and add labels to some of them.
  {
    auto acc = storage.Access();
    for (uint64_t i = 0; i < 1000; ++i) {
      auto vertex = acc.FindVertex(vertices[i], storage::View::OLD);
      EXPECT_EQ(vertex.has_value(), i % 5 != 0);

      if (vertex.has_value()) {
        EXPECT_FALSE(
            vertex->AddLabel(storage::LabelId::FromUint(3 * i)).HasError());
        EXPECT_FALSE(
            vertex->AddLabel(storage::LabelId::FromUint(3 * i + 1)).HasError());
        EXPECT_FALSE(
            vertex->AddLabel(storage::LabelId::FromUint(3 * i + 2)).HasError());
      }
    }

    // Wait for GC.
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Verify labels.
    for (uint64_t i = 0; i < 1000; ++i) {
      auto vertex = acc.FindVertex(vertices[i], storage::View::NEW);
      EXPECT_EQ(vertex.has_value(), i % 5 != 0);

      if (vertex.has_value()) {
        auto labels_old = vertex->Labels(storage::View::OLD);
        EXPECT_TRUE(labels_old.HasValue());
        EXPECT_TRUE(labels_old->empty());

        auto labels_new = vertex->Labels(storage::View::NEW);
        EXPECT_TRUE(labels_new.HasValue());
        EXPECT_THAT(
            labels_new.GetValue(),
            UnorderedElementsAre(storage::LabelId::FromUint(3 * i),
                                 storage::LabelId::FromUint(3 * i + 1),
                                 storage::LabelId::FromUint(3 * i + 2)));
      }
    }

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Add and remove some edges.
  {
    auto acc = storage.Access();
    for (uint64_t i = 0; i < 1000; ++i) {
      auto from_vertex = acc.FindVertex(vertices[i], storage::View::OLD);
      auto to_vertex =
          acc.FindVertex(vertices[(i + 1) % 1000], storage::View::OLD);
      EXPECT_EQ(from_vertex.has_value(), i % 5 != 0);
      EXPECT_EQ(to_vertex.has_value(), (i + 1) % 5 != 0);

      if (from_vertex.has_value() && to_vertex.has_value()) {
        EXPECT_FALSE(acc.CreateEdge(&from_vertex.value(), &to_vertex.value(),
                                    storage::EdgeTypeId::FromUint(i))
                         .HasError());
      }
    }

    // Detach delete some vertices.
    for (uint64_t i = 0; i < 1000; ++i) {
      auto vertex = acc.FindVertex(vertices[i], storage::View::NEW);
      EXPECT_EQ(vertex.has_value(), i % 5 != 0);
      if (vertex.has_value()) {
        if (i % 3 == 0) {
          EXPECT_FALSE(acc.DetachDeleteVertex(&vertex.value()).HasError());
        }
      }
    }

    // Wait for GC.
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Vertify edges.
    for (uint64_t i = 0; i < 1000; ++i) {
      auto vertex = acc.FindVertex(vertices[i], storage::View::NEW);
      EXPECT_EQ(vertex.has_value(), i % 5 != 0 && i % 3 != 0);
      if (vertex.has_value()) {
        auto out_edges = vertex->OutEdges({}, storage::View::NEW);
        if (i % 5 != 4 && i % 3 != 2) {
          EXPECT_EQ(out_edges.GetValue().size(), 1);
          EXPECT_EQ(*vertex->OutDegree(storage::View::NEW), 1);
          EXPECT_EQ(out_edges.GetValue().at(0).EdgeType().AsUint(), i);
        } else {
          EXPECT_TRUE(out_edges->empty());
        }

        auto in_edges = vertex->InEdges({}, storage::View::NEW);
        if (i % 5 != 1 && i % 3 != 1) {
          EXPECT_EQ(in_edges.GetValue().size(), 1);
          EXPECT_EQ(*vertex->InDegree(storage::View::NEW), 1);
          EXPECT_EQ(in_edges.GetValue().at(0).EdgeType().AsUint(),
                    (i + 999) % 1000);
        } else {
          EXPECT_TRUE(in_edges->empty());
        }
      }
    }

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// A simple sanity check for index GC:
// 1. Start transaction 0, create some vertices, add a label to them and
//    commit.
// 2. Start transaction 1.
// 3. Start transaction 2, remove the labels and commit;
// 4. Wait for GC. GC shouldn't remove the vertices from index because
//    transaction 1 can still see them with that label.
// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2Gc, Indices) {
  storage::Storage storage(
      storage::StorageGcConfig{.type = storage::StorageGcConfig::Type::PERIODIC,
                               .interval = std::chrono::milliseconds(100)});

  {
    auto acc0 = storage.Access();
    for (uint64_t i = 0; i < 1000; ++i) {
      auto vertex = acc0.CreateVertex();
      ASSERT_TRUE(*vertex.AddLabel(acc0.NameToLabel("label")));
    }
    ASSERT_EQ(acc0.Commit(), std::nullopt);
  }
  {
    auto acc1 = storage.Access();

    auto acc2 = storage.Access();
    for (auto vertex : acc2.Vertices(storage::View::OLD)) {
      ASSERT_TRUE(*vertex.RemoveLabel(acc2.NameToLabel("label")));
    }
    ASSERT_EQ(acc2.Commit(), std::nullopt);

    // Wait for GC.
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    std::set<storage::Gid> gids;
    for (auto vertex :
         acc1.Vertices(acc1.NameToLabel("label"), storage::View::OLD)) {
      gids.insert(vertex.Gid());
    }
    EXPECT_EQ(gids.size(), 1000);
  }
}