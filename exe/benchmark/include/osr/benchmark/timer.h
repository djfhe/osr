#pragma once

#include <chrono>
#include <string>

namespace benchmark {
  namespace chrono = std::chrono;

  struct timer {

    explicit timer(std::string name)
        : name_{std::move(name)}, start_{chrono::steady_clock::now()} {
    }

    chrono::duration<double> elapsed() const {
      auto const stop = chrono::steady_clock::now();
      return stop - start_;
    }

    unsigned long long elapsed_micro() const {
      auto const stop = chrono::steady_clock::now();
      return chrono::duration_cast<chrono::microseconds>(stop - start_).count();
    }

    std::string name_;
    chrono::time_point<chrono::steady_clock> start_;
  };
}  // namespace benchmark