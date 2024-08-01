#pragma once

#include <variant>

#include "osr/types.h"
#include "osr/ways.h"

#include "osr/routing/profiles/foot_car.h"

template<typename, typename = void>
struct has_source_node : std::false_type { };

template<typename T>
struct has_source_node<T, std::void_t<typename T::source_node>> : std::true_type { };

template <typename T, bool = has_source_node<T>::value>
struct get_source_node {
  using type = typename T::source_node;
};

template<typename T>
struct get_source_node<T, false> {
  using type = typename T::node;
};

template<typename, typename = void>
struct has_destination_node : std::false_type { };

template<typename T>
struct has_destination_node<T, std::void_t<typename T::destination_node>> : std::true_type { };

template<typename T, bool = has_destination_node<T>::value>
struct get_destination_node {
  using type = typename T::destination_node;
};

template<typename T>
struct get_destination_node<T, false> {
  using type = typename T::node;
};

namespace osr {

// Metafunction to map entry types to their corresponding profile types
template <typename... profiles>
struct combi_profile {

  using profile_variant = std::variant<profiles...>;

  static std::array<profile_variant, sizeof...(profiles)> constexpr profiles_ = {profiles{}...};

  static_assert(sizeof...(profiles) > 0, "At least one profile must be provided.");

  using first_profile = std::tuple_element_t<0, std::tuple<profiles...>>;
  using last_profile = std::tuple_element_t<sizeof...(profiles) - 1, std::tuple<profiles...>>;
  using last_profile_index = std::integral_constant<size_t, sizeof...(profiles) - 1>;

  using profile_node = std::variant<typename profiles::node...>;
  using profile_node_tuple = std::tuple<typename profiles::node...>;

  template <typename entry, typename... rest>
  struct node_to_profile_impl;

  template<typename node, typename profile, typename... rest>
  struct node_to_profile_impl<node, profile, rest...> {
    using type = std::conditional_t<
            std::is_same_v<node, typename profile::node>,
            profile,
            typename node_to_profile_impl<node, rest...>::type
    >;
  };

  template<typename node, typename profile>
  struct node_to_profile_impl<node, profile> {
    using type = std::conditional_t<
            std::is_same_v<node, typename profile::node>,
            profile,
            void
    >;
    static_assert(!std::is_same_v<node, type>, "Node type does not match any profile.");
  };

  template<typename node>
  struct node_to_profile : node_to_profile_impl<node, profiles...> {};

  struct node {
    friend bool operator==(node, node) = default;

    boost::json::object geojson_properties(ways const& w) const {
      return std::visit([&](auto const& n) { return n.geojson_properties(w); }, n_);
    }

    std::ostream& print(std::ostream& out, ways const& w) const {
      return std::visit([&](auto const& n) -> std::ostream& { return n.print(out, w); }, n_);
    }


    static constexpr node invalid() noexcept { return { first_profile::node::invalid(), 0 }; }
    constexpr node_idx_t get_node() const noexcept {
        return std::visit([](auto const& n) { return n.get_node(); }, n_);
    }
    constexpr node get_key() const noexcept {
      return *this;
    }

    profile_node n_;
    std::size_t template_idx;
  };

  using profile_key = std::variant<typename profiles::key...>;
  using key = node;

  struct hash {
    using is_avalanching = void;
    using hash_variant = std::variant<typename profiles::hash...>;

    std::array<hash_variant, sizeof...(profiles)> hashes_ = {typename profiles::hash{}...};

    template <typename hash, typename... rest>
    struct hash_to_profile_impl;

    template <typename hash, typename profile, typename... rest>
    struct hash_to_profile_impl<hash, profile, rest...> {
      using type = std::conditional_t<
              std::is_same_v<hash, typename profile::hash>,
              profile,
              typename hash_to_profile_impl<hash, rest...>::type
      >;
    };

    template <typename hash, typename profile>
    struct hash_to_profile_impl<hash, profile> {
      using type = std::conditional_t<
              std::is_same_v<hash, typename profile::hash>,
              profile,
              void
      >;
      static_assert(!std::is_same_v<hash, type>, "Hash type does not match any profile.");
    };

    auto operator()(key const n) const noexcept -> std::uint64_t {
      auto const idx = n.template_idx;
      return std::visit([&](auto const& h) {
        using HashType = std::decay_t<decltype(h)>;
        using NodeType = typename hash_to_profile_impl<HashType, profiles...>::type::node;
        return h(std::get<NodeType>(n.n_).get_key());
      }, hashes_[idx]);
    }
  };

  using profile_label = std::variant<typename profiles::label...>;

  struct label {
    label(node const n, cost_t const c) : n_{n}, cost_{c} {}

    constexpr node get_node() const noexcept { return n_; }
    constexpr cost_t cost() const noexcept { return cost_; }

    void track(label const&, ways::routing const&, way_idx_t, node_idx_t) {}

    node n_;
    cost_t cost_;
  };

  using profile_entry = std::variant<typename profiles::entry...>;
  using profile_entry_tuple = std::tuple<typename profiles::entry...>;

  // Metafunction to map entry types to their corresponding profile types
  template <typename entry, typename... rest>
  struct entry_to_profile_impl;

  template <typename entry, typename profile, typename... rest>
  struct entry_to_profile_impl<entry, profile, rest...> {
    using type = std::conditional_t<
        std::is_same_v<entry, typename profile::entry>,
        profile,
        typename entry_to_profile_impl<entry, rest...>::type
    >;
  };

  template <typename entry, typename profile>
  struct entry_to_profile_impl<entry, profile> {
    using type = std::conditional_t<
        std::is_same_v<entry, typename profile::entry>,
        profile,
        void
    >;
    static_assert(!std::is_same_v<entry, type>, "Entry type does not match any profile.");
  };

  template <typename entry>
  struct entry_to_profile : entry_to_profile_impl<entry, profiles...> {};

  struct entry {

    entry() {
      //fill entries with default constructed entries
      entries = {typename profiles::entry{}...};
    }

    constexpr cost_t cost(node const n) const noexcept {
      auto const idx = n.template_idx;
      return std::visit([&](auto const& e) {
        using EntryType = std::decay_t<decltype(e)>;
        using NodeType = typename entry_to_profile<EntryType>::type::node;
        return e.cost(std::get<NodeType>(n.n_));
      }, entries[idx]);
    }

    constexpr bool update(label const& l,
                          node const n,
                          cost_t const c,
                          node const pred) noexcept {
      auto const idx = n.template_idx;

      return std::visit([&](auto& e) {
        using EntryType = std::decay_t<decltype(e)>;
        using NodeType = typename entry_to_profile<EntryType>::type::node;
        return e.update({std::get<NodeType>(l.n_.n_), l.cost_}, std::get<NodeType>(n.n_), c, std::get<NodeType>(pred.n_));
      }, entries[idx]);
    }

    constexpr std::optional<node> pred(node const n) const noexcept {
      auto const idx = n.template_idx;

      return std::visit([&](auto const& e) -> std::optional<node> {
          using EntryType = std::decay_t<decltype(e)>;
          using NodeType = typename entry_to_profile<EntryType>::type::node;

          if (auto const p = e.pred(std::get<NodeType>(n.n_))) {
              return std::optional{node{*p, n.template_idx}};
          }

          return std::nullopt;
      }, entries[idx]);
    }

    void write(node, path&) const {}

    std::array<profile_entry, sizeof...(profiles)> entries;
  };

  template <typename Fn>
  static void resolve_start_node(ways::routing const& w,
                                 way_idx_t const way,
                                 node_idx_t const n,
                                 level_t const level,
                                 direction const dir,
                                 Fn&& f) {
    if (dir == direction::kForward) {
      first_profile::resolve_start_node(w, way, n, level, dir, [&](typename first_profile::node first_profile_node) {
        f(node{first_profile_node, 0});
      });
    } else {
      last_profile::resolve_start_node(w, way, n, level, dir, [&](typename last_profile::node last_profile_node) {
        f(node{last_profile_node, last_profile_index::value});
      });
    }
  }

  template <typename Fn>
  static void resolve_all(ways::routing const& w,
                          node_idx_t const n,
                          level_t const level,
                          Fn&& fn) {
    (profiles::resolve_all(w, n, level, [&](auto&& profile_node) {
      fn(node{profile_node, 0});
    }), ...);
  }

  template <direction SearchDir, typename CurrentProfile, typename NextProfile, bool HasDestinationNode = has_destination_node<CurrentProfile>::value, bool HasSourceNode = has_source_node<NextProfile>::value>
  struct next_node_impl;

  template<direction SearchDir, typename CurrentProfile, typename NextProfile>
  struct next_node_impl<SearchDir, CurrentProfile, NextProfile, false, false> {
    static typename get_source_node<NextProfile>::type next_node(typename CurrentProfile::node const n) {
      return n;
    }
  };

  template<direction SearchDir, typename CurrentProfile, typename NextProfile>
  struct next_node_impl<SearchDir, CurrentProfile, NextProfile, true, false> {
    static typename get_destination_node<CurrentProfile>::type next_node(typename CurrentProfile::node const n) {
      return CurrentProfile::destination_node(n);
    }
  };

  template<direction SearchDir, typename CurrentProfile, typename NextProfile>
  struct next_node_impl<SearchDir, CurrentProfile, NextProfile, false, true> {
    static typename NextProfile::node next_node(typename CurrentProfile::node const n) {
      return NextProfile::to_node(n);
    }
  };

  template<direction SearchDir, typename CurrentProfile, typename NextProfile>
  struct next_node_impl<SearchDir, CurrentProfile, NextProfile, true, true> {
    static typename NextProfile::node next_node(typename CurrentProfile::node const n) {
      return NextProfile::to_node(CurrentProfile::destination_node(n));
    }
  };

  template <direction SearchDir, typename CurrentProfile, typename NextProfile>
  static auto next_node(typename CurrentProfile::node const n) {
    return next_node_impl<SearchDir, CurrentProfile, NextProfile>::next_node(n);
  }

  template <direction SearchDir, bool WithBlocked, typename Fn>
   static void adjacent(ways::routing const& w,
                        node const n,
                        bitvec<node_idx_t> const* blocked,
                        Fn&& fn) {
    std::visit([&](auto const& profile_node) {
      using NodeType = std::decay_t<decltype(profile_node)>;
      using Profile = typename node_to_profile<NodeType>::type;
      Profile::template adjacent<SearchDir, WithBlocked>(w, profile_node, blocked, [&](
        NodeType adjacent_profile_node,
        cost_t const cost,
        distance_t const dist,
        way_idx_t const way,
        std::uint16_t const from,
        std::uint16_t const to
      ) {
        fn(node{adjacent_profile_node, n.template_idx}, cost, dist, way, from, to);
      });
    }, n.n_);

    if constexpr (SearchDir == direction::kBackward) {
      return;
    }

    if (n.template_idx == last_profile_index::value) {
      return;
    }

    auto const next_idx = n.template_idx + (SearchDir == direction::kForward ? 1 : 1);
    auto const next_profile = profiles_[next_idx];

    std::visit([&](auto const& profile) {
      using NextProfile = std::decay_t<decltype(profile)>;

      static_assert(!std::is_same_v<NextProfile, first_profile>, "profile should not match first profile.");

      std::visit([&](auto const& profile_node) {
        using NodeType = std::decay_t<decltype(profile_node)>;
        using CurrentProfile = typename node_to_profile<NodeType>::type;

        auto next_node = combi_profile::next_node<SearchDir, CurrentProfile, NextProfile>(profile_node);

        using NextNodeType = std::decay_t<decltype(next_node)>;

        if constexpr (!std::is_same_v<NextNodeType, typename NextProfile::node>) {
          return;
        }

        static_assert(std::is_same_v<NextNodeType, typename NextProfile::node>, "Node type does not match any profile.");
        NextProfile::template adjacent<SearchDir, WithBlocked>(w, next_node, blocked, [&](
          typename NextProfile::node adjacent_profile_node,
          cost_t const cost,
          distance_t const dist,
          way_idx_t const way,
          std::uint16_t const from,
          std::uint16_t const to
        ) {
          fn(node{adjacent_profile_node, next_idx}, cost, dist, way, from, to);
        });
      }, n.n_);
    }, next_profile);
  }

  static bool is_dest_reachable(ways::routing const& w,
                              node const n,
                              way_idx_t const way,
                              direction const way_dir,
                              direction const search_dir) {
    if (search_dir == direction::kForward) {
      return first_profile::is_dest_reachable(w, std::get<typename first_profile::node>(n.n_), way, way_dir, search_dir);
    }

    return last_profile::is_dest_reachable(w, std::get<typename last_profile::node>(n.n_), way, way_dir, search_dir);
  }


  static constexpr cost_t way_cost(way_properties const& e,
                                   direction const dir,
                                   direction const search_dir,
                                   std::uint16_t const dist) {
    if (search_dir == direction::kForward) {
      return first_profile::way_cost(e, dir, search_dir, dist);
    }

    return last_profile::way_cost(e, dir, search_dir, dist);
  }
};

using test_profile = combi_profile<foot<false>, foot_car<false>, car>;
}