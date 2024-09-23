#pragma once

#include "csv_writer.h"

#include <chrono>
#include <future>
#include "osr/routing/route.h"
#include "osr/ways.h"

namespace benchmark {

template <typename T>
struct test_case {

  test_case() = default;

  explicit test_case(
    osr::cost_t const max_dist,
    osr::ways const& ways
  ) : max_dist_{max_dist}, ways_{ways}, routing_(*ways_.r_) {

  }

  template <osr::direction SearchDir, bool WithBlocked>
  void runMany(unsigned const threads, unsigned const iterations) {
    threads_.clear();
    threads_.reserve(threads);

    futures_.clear();
    futures_.reserve(threads);

    auto max_dist = max_dist_;
    auto& routing = routing_;
    auto& ways = ways_;
    auto& blocked = blocked_;

    for (unsigned i = 0; i < threads; ++i) {
      std::promise<std::vector<std::chrono::duration<double>>> p;
      futures_.emplace_back(std::move(p.get_future()));

      auto thread = std::thread([&max_dist, &routing, &ways, &iterations, &i, &blocked](auto promise) {
        auto d = osr::dijkstra<T>{};
        auto h = cista::BASE_HASH;

        auto runs = std::vector<std::chrono::duration<double>>{iterations};
        std::cout << "Thread " << i << " started iterations: " << iterations << std::endl;
        for (unsigned j = 0; j < iterations; ++j) {
          d.reset(max_dist);

          auto const startNode = osr::node_idx_t{cista::hash_combine(h, i, j) % ways.n_nodes()};

          T::resolve_all(routing, startNode, osr::level_t::invalid(), [&](auto const& n) {
            d.add_start<SearchDir>(typename T::label{n, 0});
          });

          auto start = std::chrono::steady_clock::now();
          d.template run<SearchDir, WithBlocked>(routing, max_dist, blocked);
          runs[j] = std::chrono::steady_clock::now() - start;
        }

        promise.set_value(runs);
      }, std::move(p));

      threads_.emplace_back(std::move(thread));
    }
  }

  void wait() {
    for (auto& t : threads_) {
      t.join();
    }
  }

  void write(benchmark::csv_writer& writer) {
    for (size_t i = 0; i < futures_.size(); ++i) {
      auto const& runs = futures_[i].get();
      for (auto const& run : runs) {
            writer.write_row<std::string>({std::to_string(i), std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(run).count())});
      }
    }
  }

  osr::cost_t const max_dist_;
  osr::ways const& ways_;
  osr::ways::routing const& routing_;
  osr::bitvec<osr::node_idx_t> const* blocked_ = nullptr;
  std::vector<std::thread> threads_{};
  std::vector<std::future<std::vector<std::chrono::duration<double>>>> futures_{};
};
}
