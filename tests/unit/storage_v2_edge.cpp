#include <gtest/gtest.h>

#include <limits>

#include "storage/v2/storage.hpp"

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeCreateFromSmallerCommit) {
  storage::Storage store;
  storage::Gid gid_from =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_to =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertices
  {
    auto acc = store.Access();
    auto vertex_from = acc.CreateVertex();
    auto vertex_to = acc.CreateVertex();
    gid_from = vertex_from.Gid();
    gid_to = vertex_to.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex_from, &*vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex_from);
    ASSERT_EQ(edge.ToVertex(), *vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeCreateFromLargerCommit) {
  storage::Storage store;
  storage::Gid gid_from =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_to =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertices
  {
    auto acc = store.Access();
    auto vertex_to = acc.CreateVertex();
    auto vertex_from = acc.CreateVertex();
    gid_to = vertex_to.Gid();
    gid_from = vertex_from.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex_from, &*vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex_from);
    ASSERT_EQ(edge.ToVertex(), *vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeCreateFromSameCommit) {
  storage::Storage store;
  storage::Gid gid_vertex =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertex
  {
    auto acc = store.Access();
    auto vertex = acc.CreateVertex();
    gid_vertex = vertex.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex, &*vertex, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex);
    ASSERT_EQ(edge.ToVertex(), *vertex);

    // Check edges without filters
    ASSERT_EQ(vertex->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    ASSERT_EQ(vertex->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::NEW)->size(), 1);
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::NEW)->size(), 1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    {
      auto ret = vertex->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::NEW)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::NEW)->size(), 1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeCreateFromSmallerAbort) {
  storage::Storage store;
  storage::Gid gid_from =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_to =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertices
  {
    auto acc = store.Access();
    auto vertex_from = acc.CreateVertex();
    auto vertex_to = acc.CreateVertex();
    gid_from = vertex_from.Gid();
    gid_to = vertex_to.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge, but abort the transaction
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex_from, &*vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex_from);
    ASSERT_EQ(edge.ToVertex(), *vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    acc.Abort();
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex_from, &*vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex_from);
    ASSERT_EQ(edge.ToVertex(), *vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeCreateFromLargerAbort) {
  storage::Storage store;
  storage::Gid gid_from =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_to =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertices
  {
    auto acc = store.Access();
    auto vertex_to = acc.CreateVertex();
    auto vertex_from = acc.CreateVertex();
    gid_to = vertex_to.Gid();
    gid_from = vertex_from.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge, but abort the transaction
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex_from, &*vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex_from);
    ASSERT_EQ(edge.ToVertex(), *vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    acc.Abort();
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex_from, &*vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex_from);
    ASSERT_EQ(edge.ToVertex(), *vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeCreateFromSameAbort) {
  storage::Storage store;
  storage::Gid gid_vertex =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertex
  {
    auto acc = store.Access();
    auto vertex = acc.CreateVertex();
    gid_vertex = vertex.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge, but abort the transaction
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex, &*vertex, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex);
    ASSERT_EQ(edge.ToVertex(), *vertex);

    // Check edges without filters
    ASSERT_EQ(vertex->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    ASSERT_EQ(vertex->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::NEW)->size(), 1);
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::NEW)->size(), 1);

    acc.Abort();
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    // Check edges without filters
    ASSERT_EQ(vertex->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex, &*vertex, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex);
    ASSERT_EQ(edge.ToVertex(), *vertex);

    // Check edges without filters
    ASSERT_EQ(vertex->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    ASSERT_EQ(vertex->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::NEW)->size(), 1);
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::NEW)->size(), 1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    {
      auto ret = vertex->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::NEW)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::NEW)->size(), 1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeDeleteFromSmallerCommit) {
  storage::Storage store;
  storage::Gid gid_from =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_to =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertices
  {
    auto acc = store.Access();
    auto vertex_from = acc.CreateVertex();
    auto vertex_to = acc.CreateVertex();
    gid_from = vertex_from.Gid();
    gid_to = vertex_to.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex_from, &*vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex_from);
    ASSERT_EQ(edge.ToVertex(), *vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Delete edge
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto edge = vertex_from->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto res = acc.DeleteEdge(&edge);
    ASSERT_TRUE(res.HasValue());
    ASSERT_TRUE(res.GetValue());

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeDeleteFromLargerCommit) {
  storage::Storage store;
  storage::Gid gid_from =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_to =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertices
  {
    auto acc = store.Access();
    auto vertex_to = acc.CreateVertex();
    auto vertex_from = acc.CreateVertex();
    gid_from = vertex_from.Gid();
    gid_to = vertex_to.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex_from, &*vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex_from);
    ASSERT_EQ(edge.ToVertex(), *vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Delete edge
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto edge = vertex_from->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto res = acc.DeleteEdge(&edge);
    ASSERT_TRUE(res.HasValue());
    ASSERT_TRUE(res.GetValue());

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeDeleteFromSameCommit) {
  storage::Storage store;
  storage::Gid gid_vertex =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertex
  {
    auto acc = store.Access();
    auto vertex = acc.CreateVertex();
    gid_vertex = vertex.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex, &*vertex, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex);
    ASSERT_EQ(edge.ToVertex(), *vertex);

    // Check edges without filters
    ASSERT_EQ(vertex->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    ASSERT_EQ(vertex->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::NEW)->size(), 1);
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::NEW)->size(), 1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    {
      auto ret = vertex->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::NEW)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::NEW)->size(), 1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Delete edge
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto res = acc.DeleteEdge(&edge);
    ASSERT_TRUE(res.HasValue());
    ASSERT_TRUE(res.GetValue());

    // Check edges without filters
    {
      auto ret = vertex->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    ASSERT_EQ(vertex->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    ASSERT_EQ(vertex->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::OLD)->size(), 1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    // Check edges without filters
    ASSERT_EQ(vertex->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeDeleteFromSmallerAbort) {
  storage::Storage store;
  storage::Gid gid_from =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_to =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertices
  {
    auto acc = store.Access();
    auto vertex_from = acc.CreateVertex();
    auto vertex_to = acc.CreateVertex();
    gid_from = vertex_from.Gid();
    gid_to = vertex_to.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex_from, &*vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex_from);
    ASSERT_EQ(edge.ToVertex(), *vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Delete the edge, but abort the transaction
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto edge = vertex_from->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto res = acc.DeleteEdge(&edge);
    ASSERT_TRUE(res.HasValue());
    ASSERT_TRUE(res.GetValue());

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);

    acc.Abort();
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Delete the edge
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto edge = vertex_from->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto res = acc.DeleteEdge(&edge);
    ASSERT_TRUE(res.HasValue());
    ASSERT_TRUE(res.GetValue());

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeDeleteFromLargerAbort) {
  storage::Storage store;
  storage::Gid gid_from =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_to =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertices
  {
    auto acc = store.Access();
    auto vertex_from = acc.CreateVertex();
    auto vertex_to = acc.CreateVertex();
    gid_from = vertex_from.Gid();
    gid_to = vertex_to.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex_from, &*vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex_from);
    ASSERT_EQ(edge.ToVertex(), *vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Delete the edge, but abort the transaction
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto edge = vertex_from->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto res = acc.DeleteEdge(&edge);
    ASSERT_TRUE(res.HasValue());
    ASSERT_TRUE(res.GetValue());

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);

    acc.Abort();
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::NEW)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::NEW)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Delete the edge
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    auto edge = vertex_from->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto res = acc.DeleteEdge(&edge);
    ASSERT_TRUE(res.HasValue());
    ASSERT_TRUE(res.GetValue());

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex_from->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_from->OutEdges({et, other_et}, storage::View::OLD)->size(),
              1);
    ASSERT_EQ(vertex_to->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex_to->InEdges({et, other_et}, storage::View::OLD)->size(),
              1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    // Check edges without filters
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgeDeleteFromSameAbort) {
  storage::Storage store;
  storage::Gid gid_vertex =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create vertex
  {
    auto acc = store.Access();
    auto vertex = acc.CreateVertex();
    gid_vertex = vertex.Gid();
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Create edge
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&*vertex, &*vertex, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), *vertex);
    ASSERT_EQ(edge.ToVertex(), *vertex);

    // Check edges without filters
    ASSERT_EQ(vertex->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 0);
    {
      auto ret = vertex->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    ASSERT_EQ(vertex->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 0);
    {
      auto ret = vertex->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::NEW)->size(), 1);
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::NEW)->size(), 1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    {
      auto ret = vertex->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::NEW)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::NEW)->size(), 1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Delete the edge, but abort the transaction
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto res = acc.DeleteEdge(&edge);
    ASSERT_TRUE(res.HasValue());
    ASSERT_TRUE(res.GetValue());

    // Check edges without filters
    {
      auto ret = vertex->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    ASSERT_EQ(vertex->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    ASSERT_EQ(vertex->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::OLD)->size(), 1);

    acc.Abort();
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    // Check edges without filters
    {
      auto ret = vertex->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    {
      auto ret = vertex->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::NEW)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::NEW)->size(), 1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Delete the edge
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    auto et = acc.NameToEdgeType("et5");

    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto res = acc.DeleteEdge(&edge);
    ASSERT_TRUE(res.HasValue());
    ASSERT_TRUE(res.GetValue());

    // Check edges without filters
    {
      auto ret = vertex->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    ASSERT_EQ(vertex->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex);
      ASSERT_EQ(e.ToVertex(), *vertex);
    }
    ASSERT_EQ(vertex->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 0);

    auto other_et = acc.NameToEdgeType("other");

    // Check edges with filters
    ASSERT_EQ(vertex->InEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->InEdges({et, other_et}, storage::View::OLD)->size(), 1);
    ASSERT_EQ(vertex->OutEdges({other_et}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(vertex->OutEdges({et, other_et}, storage::View::OLD)->size(), 1);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check whether the edge exists
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid_vertex, storage::View::NEW);
    ASSERT_TRUE(vertex);

    // Check edges without filters
    ASSERT_EQ(vertex->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, VertexDetachDeleteSingleCommit) {
  storage::Storage store;
  storage::Gid gid_from =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_to =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create dataset
  {
    auto acc = store.Access();
    auto vertex_from = acc.CreateVertex();
    auto vertex_to = acc.CreateVertex();

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&vertex_from, &vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), vertex_from);
    ASSERT_EQ(edge.ToVertex(), vertex_to);

    gid_from = vertex_from.Gid();
    gid_to = vertex_to.Gid();

    // Check edges
    ASSERT_EQ(vertex_from.InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from.InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from.OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from.OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), vertex_from);
      ASSERT_EQ(e.ToVertex(), vertex_to);
    }
    {
      auto ret = vertex_to.InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to.InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), vertex_from);
      ASSERT_EQ(e.ToVertex(), vertex_to);
    }
    ASSERT_EQ(vertex_to.OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to.OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Detach delete vertex
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Delete must fail
    {
      auto ret = acc.DeleteVertex(&*vertex_from);
      ASSERT_TRUE(ret.HasError());
      ASSERT_EQ(ret.GetError(), storage::Error::VERTEX_HAS_EDGES);
    }

    // Detach delete vertex
    {
      auto ret = acc.DetachDeleteVertex(&*vertex_from);
      ASSERT_TRUE(ret.HasValue());
      ASSERT_TRUE(ret.GetValue());
    }

    // Check edges
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex_from->InDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex_from->OutDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check dataset
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_FALSE(vertex_from);
    ASSERT_TRUE(vertex_to);

    // Check edges
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, VertexDetachDeleteMultipleCommit) {
  storage::Storage store;
  storage::Gid gid_vertex1 =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_vertex2 =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create dataset
  {
    auto acc = store.Access();
    auto vertex1 = acc.CreateVertex();
    auto vertex2 = acc.CreateVertex();

    gid_vertex1 = vertex1.Gid();
    gid_vertex2 = vertex2.Gid();

    auto et1 = acc.NameToEdgeType("et1");
    auto et2 = acc.NameToEdgeType("et2");
    auto et3 = acc.NameToEdgeType("et3");
    auto et4 = acc.NameToEdgeType("et4");

    auto res1 = acc.CreateEdge(&vertex1, &vertex2, et1);
    ASSERT_TRUE(res1.HasValue());
    auto edge1 = res1.GetValue();
    ASSERT_EQ(edge1.EdgeType(), et1);
    ASSERT_EQ(edge1.FromVertex(), vertex1);
    ASSERT_EQ(edge1.ToVertex(), vertex2);

    auto res2 = acc.CreateEdge(&vertex2, &vertex1, et2);
    ASSERT_TRUE(res2.HasValue());
    auto edge2 = res2.GetValue();
    ASSERT_EQ(edge2.EdgeType(), et2);
    ASSERT_EQ(edge2.FromVertex(), vertex2);
    ASSERT_EQ(edge2.ToVertex(), vertex1);

    auto res3 = acc.CreateEdge(&vertex1, &vertex1, et3);
    ASSERT_TRUE(res3.HasValue());
    auto edge3 = res3.GetValue();
    ASSERT_EQ(edge3.EdgeType(), et3);
    ASSERT_EQ(edge3.FromVertex(), vertex1);
    ASSERT_EQ(edge3.ToVertex(), vertex1);

    auto res4 = acc.CreateEdge(&vertex2, &vertex2, et4);
    ASSERT_TRUE(res4.HasValue());
    auto edge4 = res4.GetValue();
    ASSERT_EQ(edge4.EdgeType(), et4);
    ASSERT_EQ(edge4.FromVertex(), vertex2);
    ASSERT_EQ(edge4.ToVertex(), vertex2);

    // Check edges
    {
      auto ret = vertex1.InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1.InDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), vertex2);
        ASSERT_EQ(e.ToVertex(), vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), vertex1);
        ASSERT_EQ(e.ToVertex(), vertex1);
      }
    }
    {
      auto ret = vertex1.OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1.OutDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), vertex1);
        ASSERT_EQ(e.ToVertex(), vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), vertex1);
        ASSERT_EQ(e.ToVertex(), vertex1);
      }
    }
    {
      auto ret = vertex2.InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2.InDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), vertex1);
        ASSERT_EQ(e.ToVertex(), vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), vertex2);
        ASSERT_EQ(e.ToVertex(), vertex2);
      }
    }
    {
      auto ret = vertex2.OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2.OutDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), vertex2);
        ASSERT_EQ(e.ToVertex(), vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), vertex2);
        ASSERT_EQ(e.ToVertex(), vertex2);
      }
    }

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Detach delete vertex
  {
    auto acc = store.Access();
    auto vertex1 = acc.FindVertex(gid_vertex1, storage::View::NEW);
    auto vertex2 = acc.FindVertex(gid_vertex2, storage::View::NEW);
    ASSERT_TRUE(vertex1);
    ASSERT_TRUE(vertex2);

    auto et1 = acc.NameToEdgeType("et1");
    auto et2 = acc.NameToEdgeType("et2");
    auto et3 = acc.NameToEdgeType("et3");
    auto et4 = acc.NameToEdgeType("et4");

    // Delete must fail
    {
      auto ret = acc.DeleteVertex(&*vertex1);
      ASSERT_TRUE(ret.HasError());
      ASSERT_EQ(ret.GetError(), storage::Error::VERTEX_HAS_EDGES);
    }

    // Detach delete vertex
    {
      auto ret = acc.DetachDeleteVertex(&*vertex1);
      ASSERT_TRUE(ret.HasValue());
      ASSERT_TRUE(ret.GetValue());
    }

    // Check edges
    {
      auto ret = vertex1->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1->InDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
    }
    ASSERT_EQ(vertex1->InEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex1->InDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex1->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1->OutDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
    }
    ASSERT_EQ(vertex1->OutEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex1->OutDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex2->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2->InDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
    }
    {
      auto ret = vertex2->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check dataset
  {
    auto acc = store.Access();
    auto vertex1 = acc.FindVertex(gid_vertex1, storage::View::NEW);
    auto vertex2 = acc.FindVertex(gid_vertex2, storage::View::NEW);
    ASSERT_FALSE(vertex1);
    ASSERT_TRUE(vertex2);

    auto et4 = acc.NameToEdgeType("et4");

    // Check edges
    {
      auto ret = vertex2->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }
    {
      auto ret = vertex2->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, VertexDetachDeleteSingleAbort) {
  storage::Storage store;
  storage::Gid gid_from =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_to =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create dataset
  {
    auto acc = store.Access();
    auto vertex_from = acc.CreateVertex();
    auto vertex_to = acc.CreateVertex();

    auto et = acc.NameToEdgeType("et5");

    auto res = acc.CreateEdge(&vertex_from, &vertex_to, et);
    ASSERT_TRUE(res.HasValue());
    auto edge = res.GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), vertex_from);
    ASSERT_EQ(edge.ToVertex(), vertex_to);

    gid_from = vertex_from.Gid();
    gid_to = vertex_to.Gid();

    // Check edges
    ASSERT_EQ(vertex_from.InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from.InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from.OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from.OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), vertex_from);
      ASSERT_EQ(e.ToVertex(), vertex_to);
    }
    {
      auto ret = vertex_to.InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to.InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), vertex_from);
      ASSERT_EQ(e.ToVertex(), vertex_to);
    }
    ASSERT_EQ(vertex_to.OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to.OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Detach delete vertex, but abort the transaction
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Delete must fail
    {
      auto ret = acc.DeleteVertex(&*vertex_from);
      ASSERT_TRUE(ret.HasError());
      ASSERT_EQ(ret.GetError(), storage::Error::VERTEX_HAS_EDGES);
    }

    // Detach delete vertex
    {
      auto ret = acc.DetachDeleteVertex(&*vertex_from);
      ASSERT_TRUE(ret.HasValue());
      ASSERT_TRUE(ret.GetValue());
    }

    // Check edges
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex_from->InDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex_from->OutDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    acc.Abort();
  }

  // Check dataset
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Check edges
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::NEW), 0);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    {
      auto ret = vertex_to->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Detach delete vertex
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_TRUE(vertex_from);
    ASSERT_TRUE(vertex_to);

    auto et = acc.NameToEdgeType("et5");

    // Delete must fail
    {
      auto ret = acc.DeleteVertex(&*vertex_from);
      ASSERT_TRUE(ret.HasError());
      ASSERT_EQ(ret.GetError(), storage::Error::VERTEX_HAS_EDGES);
    }

    // Detach delete vertex
    {
      auto ret = acc.DetachDeleteVertex(&*vertex_from);
      ASSERT_TRUE(ret.HasValue());
      ASSERT_TRUE(ret.GetValue());
    }

    // Check edges
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_from->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_from->InEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex_from->InDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex_from->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_from->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_from->OutEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex_from->OutDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex_to->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et);
      ASSERT_EQ(e.FromVertex(), *vertex_from);
      ASSERT_EQ(e.ToVertex(), *vertex_to);
    }
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check dataset
  {
    auto acc = store.Access();
    auto vertex_from = acc.FindVertex(gid_from, storage::View::NEW);
    auto vertex_to = acc.FindVertex(gid_to, storage::View::NEW);
    ASSERT_FALSE(vertex_from);
    ASSERT_TRUE(vertex_to);

    // Check edges
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->InEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->InDegree(storage::View::NEW), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::OLD)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::OLD), 0);
    ASSERT_EQ(vertex_to->OutEdges({}, storage::View::NEW)->size(), 0);
    ASSERT_EQ(*vertex_to->OutDegree(storage::View::NEW), 0);
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, VertexDetachDeleteMultipleAbort) {
  storage::Storage store;
  storage::Gid gid_vertex1 =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  storage::Gid gid_vertex2 =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create dataset
  {
    auto acc = store.Access();
    auto vertex1 = acc.CreateVertex();
    auto vertex2 = acc.CreateVertex();

    gid_vertex1 = vertex1.Gid();
    gid_vertex2 = vertex2.Gid();

    auto et1 = acc.NameToEdgeType("et1");
    auto et2 = acc.NameToEdgeType("et2");
    auto et3 = acc.NameToEdgeType("et3");
    auto et4 = acc.NameToEdgeType("et4");

    auto res1 = acc.CreateEdge(&vertex1, &vertex2, et1);
    ASSERT_TRUE(res1.HasValue());
    auto edge1 = res1.GetValue();
    ASSERT_EQ(edge1.EdgeType(), et1);
    ASSERT_EQ(edge1.FromVertex(), vertex1);
    ASSERT_EQ(edge1.ToVertex(), vertex2);

    auto res2 = acc.CreateEdge(&vertex2, &vertex1, et2);
    ASSERT_TRUE(res2.HasValue());
    auto edge2 = res2.GetValue();
    ASSERT_EQ(edge2.EdgeType(), et2);
    ASSERT_EQ(edge2.FromVertex(), vertex2);
    ASSERT_EQ(edge2.ToVertex(), vertex1);

    auto res3 = acc.CreateEdge(&vertex1, &vertex1, et3);
    ASSERT_TRUE(res3.HasValue());
    auto edge3 = res3.GetValue();
    ASSERT_EQ(edge3.EdgeType(), et3);
    ASSERT_EQ(edge3.FromVertex(), vertex1);
    ASSERT_EQ(edge3.ToVertex(), vertex1);

    auto res4 = acc.CreateEdge(&vertex2, &vertex2, et4);
    ASSERT_TRUE(res4.HasValue());
    auto edge4 = res4.GetValue();
    ASSERT_EQ(edge4.EdgeType(), et4);
    ASSERT_EQ(edge4.FromVertex(), vertex2);
    ASSERT_EQ(edge4.ToVertex(), vertex2);

    // Check edges
    {
      auto ret = vertex1.InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1.InDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), vertex2);
        ASSERT_EQ(e.ToVertex(), vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), vertex1);
        ASSERT_EQ(e.ToVertex(), vertex1);
      }
    }
    {
      auto ret = vertex1.OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1.OutDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), vertex1);
        ASSERT_EQ(e.ToVertex(), vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), vertex1);
        ASSERT_EQ(e.ToVertex(), vertex1);
      }
    }
    {
      auto ret = vertex2.InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2.InDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), vertex1);
        ASSERT_EQ(e.ToVertex(), vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), vertex2);
        ASSERT_EQ(e.ToVertex(), vertex2);
      }
    }
    {
      auto ret = vertex2.OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2.OutDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), vertex2);
        ASSERT_EQ(e.ToVertex(), vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), vertex2);
        ASSERT_EQ(e.ToVertex(), vertex2);
      }
    }

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Detach delete vertex, but abort the transaction
  {
    auto acc = store.Access();
    auto vertex1 = acc.FindVertex(gid_vertex1, storage::View::NEW);
    auto vertex2 = acc.FindVertex(gid_vertex2, storage::View::NEW);
    ASSERT_TRUE(vertex1);
    ASSERT_TRUE(vertex2);

    auto et1 = acc.NameToEdgeType("et1");
    auto et2 = acc.NameToEdgeType("et2");
    auto et3 = acc.NameToEdgeType("et3");
    auto et4 = acc.NameToEdgeType("et4");

    // Delete must fail
    {
      auto ret = acc.DeleteVertex(&*vertex1);
      ASSERT_TRUE(ret.HasError());
      ASSERT_EQ(ret.GetError(), storage::Error::VERTEX_HAS_EDGES);
    }

    // Detach delete vertex
    {
      auto ret = acc.DetachDeleteVertex(&*vertex1);
      ASSERT_TRUE(ret.HasValue());
      ASSERT_TRUE(ret.GetValue());
    }

    // Check edges
    {
      auto ret = vertex1->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1->InDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
    }
    ASSERT_EQ(vertex1->InEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex1->InDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex1->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1->OutDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
    }
    ASSERT_EQ(vertex1->OutEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex1->OutDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex2->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2->InDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
    }
    {
      auto ret = vertex2->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }

    acc.Abort();
  }

  // Check dataset
  {
    auto acc = store.Access();
    auto vertex1 = acc.FindVertex(gid_vertex1, storage::View::NEW);
    auto vertex2 = acc.FindVertex(gid_vertex2, storage::View::NEW);
    ASSERT_TRUE(vertex1);
    ASSERT_TRUE(vertex2);

    auto et1 = acc.NameToEdgeType("et1");
    auto et2 = acc.NameToEdgeType("et2");
    auto et3 = acc.NameToEdgeType("et3");
    auto et4 = acc.NameToEdgeType("et4");

    // Check edges
    {
      auto ret = vertex1->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1->InDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
    }
    {
      auto ret = vertex1->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1->InDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
    }
    {
      auto ret = vertex1->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1->OutDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
    }
    {
      auto ret = vertex1->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1->OutDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
    }
    {
      auto ret = vertex2->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2->InDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
    }
    {
      auto ret = vertex2->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2->InDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::NEW), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
    }

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Detach delete vertex
  {
    auto acc = store.Access();
    auto vertex1 = acc.FindVertex(gid_vertex1, storage::View::NEW);
    auto vertex2 = acc.FindVertex(gid_vertex2, storage::View::NEW);
    ASSERT_TRUE(vertex1);
    ASSERT_TRUE(vertex2);

    auto et1 = acc.NameToEdgeType("et1");
    auto et2 = acc.NameToEdgeType("et2");
    auto et3 = acc.NameToEdgeType("et3");
    auto et4 = acc.NameToEdgeType("et4");

    // Delete must fail
    {
      auto ret = acc.DeleteVertex(&*vertex1);
      ASSERT_TRUE(ret.HasError());
      ASSERT_EQ(ret.GetError(), storage::Error::VERTEX_HAS_EDGES);
    }

    // Detach delete vertex
    {
      auto ret = acc.DetachDeleteVertex(&*vertex1);
      ASSERT_TRUE(ret.HasValue());
      ASSERT_TRUE(ret.GetValue());
    }

    // Check edges
    {
      auto ret = vertex1->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1->InDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
    }
    ASSERT_EQ(vertex1->InEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex1->InDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex1->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex1->OutDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et3);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
    }
    ASSERT_EQ(vertex1->OutEdges({}, storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    ASSERT_EQ(vertex1->OutDegree(storage::View::NEW).GetError(),
              storage::Error::DELETED_OBJECT);
    {
      auto ret = vertex2->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2->InDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et1);
        ASSERT_EQ(e.FromVertex(), *vertex1);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
    }
    {
      auto ret = vertex2->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      std::sort(edges.begin(), edges.end(), [](const auto &a, const auto &b) {
        return a.EdgeType() < b.EdgeType();
      });
      ASSERT_EQ(edges.size(), 2);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::OLD), 2);
      {
        auto e = edges[0];
        ASSERT_EQ(e.EdgeType(), et2);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex1);
      }
      {
        auto e = edges[1];
        ASSERT_EQ(e.EdgeType(), et4);
        ASSERT_EQ(e.FromVertex(), *vertex2);
        ASSERT_EQ(e.ToVertex(), *vertex2);
      }
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check dataset
  {
    auto acc = store.Access();
    auto vertex1 = acc.FindVertex(gid_vertex1, storage::View::NEW);
    auto vertex2 = acc.FindVertex(gid_vertex2, storage::View::NEW);
    ASSERT_FALSE(vertex1);
    ASSERT_TRUE(vertex2);

    auto et4 = acc.NameToEdgeType("et4");

    // Check edges
    {
      auto ret = vertex2->InEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->InDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }
    {
      auto ret = vertex2->InEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->InDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::OLD);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::OLD), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }
    {
      auto ret = vertex2->OutEdges({}, storage::View::NEW);
      ASSERT_TRUE(ret.HasValue());
      auto edges = ret.GetValue();
      ASSERT_EQ(edges.size(), 1);
      ASSERT_EQ(*vertex2->OutDegree(storage::View::NEW), 1);
      auto e = edges[0];
      ASSERT_EQ(e.EdgeType(), et4);
      ASSERT_EQ(e.FromVertex(), *vertex2);
      ASSERT_EQ(e.ToVertex(), *vertex2);
    }
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgePropertyCommit) {
  storage::Storage store;
  storage::Gid gid =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  {
    auto acc = store.Access();
    auto vertex = acc.CreateVertex();
    gid = vertex.Gid();
    auto et = acc.NameToEdgeType("et5");
    auto edge = acc.CreateEdge(&vertex, &vertex, et).GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), vertex);
    ASSERT_EQ(edge.ToVertex(), vertex);

    auto property = acc.NameToProperty("property5");

    ASSERT_TRUE(edge.GetProperty(property, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::NEW)->size(), 0);

    {
      auto res =
          edge.SetProperty(property, storage::PropertyValue("temporary"));
      ASSERT_TRUE(res.HasValue());
      ASSERT_TRUE(res.GetValue());
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::NEW)->ValueString(),
              "temporary");
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "temporary");
    }

    {
      auto res = edge.SetProperty(property, storage::PropertyValue("nandare"));
      ASSERT_TRUE(res.HasValue());
      ASSERT_FALSE(res.GetValue());
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::NEW)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property = acc.NameToProperty("property5");

    ASSERT_EQ(edge.GetProperty(property, storage::View::OLD)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::OLD).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::NEW)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    auto other_property = acc.NameToProperty("other");

    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::NEW)->IsNull());

    acc.Abort();
  }
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property = acc.NameToProperty("property5");

    {
      auto res = edge.SetProperty(property, storage::PropertyValue());
      ASSERT_TRUE(res.HasValue());
      ASSERT_FALSE(res.GetValue());
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::OLD)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::OLD).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    ASSERT_TRUE(edge.GetProperty(property, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::NEW)->size(), 0);

    {
      auto res = edge.SetProperty(property, storage::PropertyValue());
      ASSERT_TRUE(res.HasValue());
      ASSERT_TRUE(res.GetValue());
    }

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property = acc.NameToProperty("property5");

    ASSERT_TRUE(edge.GetProperty(property, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(property, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::OLD)->size(), 0);
    ASSERT_EQ(edge.Properties(storage::View::NEW)->size(), 0);

    auto other_property = acc.NameToProperty("other");

    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::NEW)->IsNull());

    acc.Abort();
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgePropertyAbort) {
  storage::Storage store;
  storage::Gid gid =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());

  // Create the vertex.
  {
    auto acc = store.Access();
    auto vertex = acc.CreateVertex();
    gid = vertex.Gid();
    auto et = acc.NameToEdgeType("et5");
    auto edge = acc.CreateEdge(&vertex, &vertex, et).GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), vertex);
    ASSERT_EQ(edge.ToVertex(), vertex);
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Set property 5 to "nandare", but abort the transaction.
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property = acc.NameToProperty("property5");

    ASSERT_TRUE(edge.GetProperty(property, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::NEW)->size(), 0);

    {
      auto res =
          edge.SetProperty(property, storage::PropertyValue("temporary"));
      ASSERT_TRUE(res.HasValue());
      ASSERT_TRUE(res.GetValue());
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::NEW)->ValueString(),
              "temporary");
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "temporary");
    }

    {
      auto res = edge.SetProperty(property, storage::PropertyValue("nandare"));
      ASSERT_TRUE(res.HasValue());
      ASSERT_FALSE(res.GetValue());
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::NEW)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    acc.Abort();
  }

  // Check that property 5 is null.
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property = acc.NameToProperty("property5");

    ASSERT_TRUE(edge.GetProperty(property, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(property, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::OLD)->size(), 0);
    ASSERT_EQ(edge.Properties(storage::View::NEW)->size(), 0);

    auto other_property = acc.NameToProperty("other");

    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::NEW)->IsNull());

    acc.Abort();
  }

  // Set property 5 to "nandare".
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property = acc.NameToProperty("property5");

    ASSERT_TRUE(edge.GetProperty(property, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::NEW)->size(), 0);

    {
      auto res =
          edge.SetProperty(property, storage::PropertyValue("temporary"));
      ASSERT_TRUE(res.HasValue());
      ASSERT_TRUE(res.GetValue());
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::NEW)->ValueString(),
              "temporary");
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "temporary");
    }

    {
      auto res = edge.SetProperty(property, storage::PropertyValue("nandare"));
      ASSERT_TRUE(res.HasValue());
      ASSERT_FALSE(res.GetValue());
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::NEW)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check that property 5 is "nandare".
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property = acc.NameToProperty("property5");

    ASSERT_EQ(edge.GetProperty(property, storage::View::OLD)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::OLD).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::NEW)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    auto other_property = acc.NameToProperty("other");

    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::NEW)->IsNull());

    acc.Abort();
  }

  // Set property 5 to null, but abort the transaction.
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property = acc.NameToProperty("property5");

    ASSERT_EQ(edge.GetProperty(property, storage::View::OLD)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::OLD).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::NEW)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    {
      auto res = edge.SetProperty(property, storage::PropertyValue());
      ASSERT_TRUE(res.HasValue());
      ASSERT_FALSE(res.GetValue());
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::OLD)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::OLD).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    ASSERT_TRUE(edge.GetProperty(property, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::NEW)->size(), 0);

    acc.Abort();
  }

  // Check that property 5 is "nandare".
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property = acc.NameToProperty("property5");

    ASSERT_EQ(edge.GetProperty(property, storage::View::OLD)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::OLD).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::NEW)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    auto other_property = acc.NameToProperty("other");

    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::NEW)->IsNull());

    acc.Abort();
  }

  // Set property 5 to null.
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property = acc.NameToProperty("property5");

    ASSERT_EQ(edge.GetProperty(property, storage::View::OLD)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::OLD).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::NEW)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    {
      auto res = edge.SetProperty(property, storage::PropertyValue());
      ASSERT_TRUE(res.HasValue());
      ASSERT_FALSE(res.GetValue());
    }

    ASSERT_EQ(edge.GetProperty(property, storage::View::OLD)->ValueString(),
              "nandare");
    {
      auto properties = edge.Properties(storage::View::OLD).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property].ValueString(), "nandare");
    }

    ASSERT_TRUE(edge.GetProperty(property, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::NEW)->size(), 0);

    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  // Check that property 5 is null.
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property = acc.NameToProperty("property5");

    ASSERT_TRUE(edge.GetProperty(property, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(property, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::OLD)->size(), 0);
    ASSERT_EQ(edge.Properties(storage::View::NEW)->size(), 0);

    auto other_property = acc.NameToProperty("other");

    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(other_property, storage::View::NEW)->IsNull());

    acc.Abort();
  }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StorageV2, EdgePropertySerializationError) {
  storage::Storage store;
  storage::Gid gid =
      storage::Gid::FromUint(std::numeric_limits<uint64_t>::max());
  {
    auto acc = store.Access();
    auto vertex = acc.CreateVertex();
    gid = vertex.Gid();
    auto et = acc.NameToEdgeType("et5");
    auto edge = acc.CreateEdge(&vertex, &vertex, et).GetValue();
    ASSERT_EQ(edge.EdgeType(), et);
    ASSERT_EQ(edge.FromVertex(), vertex);
    ASSERT_EQ(edge.ToVertex(), vertex);
    ASSERT_EQ(acc.Commit(), std::nullopt);
  }

  auto acc1 = store.Access();
  auto acc2 = store.Access();

  // Set property 1 to 123 in accessor 1.
  {
    auto vertex = acc1.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property1 = acc1.NameToProperty("property1");
    auto property2 = acc1.NameToProperty("property2");

    ASSERT_TRUE(edge.GetProperty(property1, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(property1, storage::View::NEW)->IsNull());
    ASSERT_TRUE(edge.GetProperty(property2, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(property2, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::OLD)->size(), 0);
    ASSERT_EQ(edge.Properties(storage::View::NEW)->size(), 0);

    {
      auto res = edge.SetProperty(property1, storage::PropertyValue(123));
      ASSERT_TRUE(res.HasValue());
      ASSERT_TRUE(res.GetValue());
    }

    ASSERT_TRUE(edge.GetProperty(property1, storage::View::OLD)->IsNull());
    ASSERT_EQ(edge.GetProperty(property1, storage::View::NEW)->ValueInt(), 123);
    ASSERT_TRUE(edge.GetProperty(property2, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(property2, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::OLD)->size(), 0);
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property1].ValueInt(), 123);
    }
  }

  // Set property 2 to "nandare" in accessor 2.
  {
    auto vertex = acc2.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property1 = acc2.NameToProperty("property1");
    auto property2 = acc2.NameToProperty("property2");

    ASSERT_TRUE(edge.GetProperty(property1, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(property1, storage::View::NEW)->IsNull());
    ASSERT_TRUE(edge.GetProperty(property2, storage::View::OLD)->IsNull());
    ASSERT_TRUE(edge.GetProperty(property2, storage::View::NEW)->IsNull());
    ASSERT_EQ(edge.Properties(storage::View::OLD)->size(), 0);
    ASSERT_EQ(edge.Properties(storage::View::NEW)->size(), 0);

    {
      auto res = edge.SetProperty(property2, storage::PropertyValue("nandare"));
      ASSERT_TRUE(res.HasError());
      ASSERT_EQ(res.GetError(), storage::Error::SERIALIZATION_ERROR);
    }
  }

  // Finalize both accessors.
  ASSERT_EQ(acc1.Commit(), std::nullopt);
  acc2.Abort();

  // Check which properties exist.
  {
    auto acc = store.Access();
    auto vertex = acc.FindVertex(gid, storage::View::OLD);
    ASSERT_TRUE(vertex);
    auto edge = vertex->OutEdges({}, storage::View::NEW).GetValue()[0];

    auto property1 = acc.NameToProperty("property1");
    auto property2 = acc.NameToProperty("property2");

    ASSERT_EQ(edge.GetProperty(property1, storage::View::OLD)->ValueInt(), 123);
    ASSERT_TRUE(edge.GetProperty(property2, storage::View::OLD)->IsNull());
    {
      auto properties = edge.Properties(storage::View::OLD).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property1].ValueInt(), 123);
    }

    ASSERT_EQ(edge.GetProperty(property1, storage::View::NEW)->ValueInt(), 123);
    ASSERT_TRUE(edge.GetProperty(property2, storage::View::NEW)->IsNull());
    {
      auto properties = edge.Properties(storage::View::NEW).GetValue();
      ASSERT_EQ(properties.size(), 1);
      ASSERT_EQ(properties[property1].ValueInt(), 123);
    }

    acc.Abort();
  }
}