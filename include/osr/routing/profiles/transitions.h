#pragma once

#include "osr/routing/profiles/foot.h"
#include "osr/routing/route.h"
#include "osr/ways.h"

namespace osr::transitions {

// callable object to check if a node is a parking

  template <cost_t cost>
  struct is_parking {
    static constexpr cost_t operator()(osr::ways::routing const& w,
                    node_idx_t n_,
                    bitvec<node_idx_t> const*
    ) {
      if (w.node_properties_[n_].is_parking()) {
        return cost;
      }

      return kInfeasible;
    }
  };

}  // namespace osr