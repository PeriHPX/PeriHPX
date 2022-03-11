////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2019 Prashant K. Jha
//  Copyright (c) 2019 Patrick Diehl
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#include "neighbor.h"

#include <hpx/include/parallel_algorithm.hpp>
#include <nanoflann.hpp>

#include "inp/decks/neighborDeck.h"
#include "util/compare.h"
#include "util/parallel.h"
#include "util/utilIO.h"

geometry::Neighbor::Neighbor(const double &horizon, inp::NeighborDeck *deck,
                             const std::vector<util::Point3> *nodes)
    : d_neighborDeck_p(deck) {
  d_neighbors = std::vector<std::vector<size_t>>(nodes->size());

  PointCloud cloud;

  cloud.pts = *nodes;

  nanoflann::KDTreeSingleIndexAdaptor<
      nanoflann::L2_Simple_Adaptor<double, PointCloud>, PointCloud, 3 /* dim */
      >
      index(3 /*dim*/, cloud, {50 /* max leaf */});

  index.buildIndex();

  const double search_radius = static_cast<double>(horizon * horizon);

  nanoflann::SearchParams params;

  hpx::for_loop(
      hpx::execution::par, 0, nodes->size(),
      [this, nodes, search_radius, params, &index](boost::uint64_t i) {
        std::vector<std::pair<uint32_t, double>> ret_matches;

        const double query_pt[3] = {(*nodes)[i].d_x, (*nodes)[i].d_y,
                                    (*nodes)[i].d_z};

        this->d_neighbors[i] = std::vector<size_t>();

        const size_t nMatches = index.radiusSearch(&query_pt[0], search_radius,
                                                   ret_matches, params);

        for (std::size_t j = 0; j < nMatches; ++j)
          if (ret_matches[j].first != i) {
            this->d_neighbors[i].push_back(size_t(ret_matches[j].first));
          }
      });  // end of parallel for loop

  cloud.pts.clear();
}

const std::vector<size_t> &geometry::Neighbor::getNeighbors(const size_t &i) {
  return d_neighbors[i];
}

std::vector<std::vector<size_t>> *geometry::Neighbor::getNeighborsListP() {
  return &d_neighbors;
}
const std::vector<std::vector<size_t>> *geometry::Neighbor::getNeighborsListP()
    const {
  return &d_neighbors;
}

std::vector<std::vector<size_t>> &geometry::Neighbor::getNeighborsList() {
  return d_neighbors;
}
const std::vector<std::vector<size_t>> &geometry::Neighbor::getNeighborsList()
    const {
  return d_neighbors;
}

size_t geometry::Neighbor::getNeighbor(const size_t &i, const size_t &j) const {
  return d_neighbors[i][j];
}

std::string geometry::Neighbor::printStr(int nt, int lvl) const {
  auto tabS = util::io::getTabS(nt);
  std::ostringstream oss;
  oss << tabS << "------- Neighbor --------" << std::endl << std::endl;
  oss << tabS << "Neighbor deck address = " << d_neighborDeck_p << std::endl;
  oss << tabS << "Number of data = " << d_neighbors.size() << std::endl;
  oss << tabS << std::endl;

  return oss.str();
}
