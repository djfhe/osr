#include <osr/routing/profiles/car_parking.h>
#include <osr/routing/profiles/combi_profile.h>
#include <osr/routing/profiles/foot.h>

#include <filesystem>
#include <thread>
#include <vector>

#include "boost/asio/executor_work_guard.hpp"
#include "boost/asio/io_context.hpp"

#include "fmt/core.h"
#include "fmt/std.h"

#include "conf/options_parser.h"

#include "osr/lookup.h"
#include "osr/routing/profiles/car.h"
#include "osr/routing/route.h"
#include "osr/ways.h"

#include "osr/benchmark/test_case.h"
#include "osr/benchmark/csv_writer.h"

namespace fs = std::filesystem;
using namespace osr;

class settings : public conf::configuration {
public:
  explicit settings() : configuration("Options") {
    param(data_dir_, "data,d", "Data directory");
    param(threads_, "threads,t", "Number of routing threads");
    param(n_queries_, "n", "Number of queries");
    param(max_dist_, "r", "Radius");
  }

  fs::path data_dir_{"osr"};
  unsigned n_queries_{500};
  unsigned max_dist_{1200};
  unsigned threads_{std::thread::hardware_concurrency()};
};


template<typename Profile>
void runBenchmark(const fs::path& output_path, unsigned threads, unsigned iterations, unsigned max_dist, const osr::ways& w) {
  auto run = benchmark::test_case<Profile>{static_cast<cost_t>(max_dist), w};
  auto count = std::max(1U, threads);
  run.template runMany<direction::kForward, false>(count, iterations);
  run.wait();

  benchmark::csv_writer writer(output_path, {"thread index", "duration in microseconds", "visited nodes"});

  run.write(writer);
}

int main(int argc, char const* argv[]) {
  auto opt = settings{};
  auto parser = conf::options_parser({&opt});
  parser.read_command_line_args(argc, argv);

  if (parser.help()) {
    parser.print_help(std::cout);
    return 0;
  }

  if (parser.version()) {
    return 0;
  }

  parser.read_configuration_file();
  parser.print_unrecognized(std::cout);
  parser.print_used(std::cout);

  if (!fs::is_directory(opt.data_dir_)) {
    fmt::println("directory not found: {}", opt.data_dir_);
    return 1;
  }

  auto const w = ways{opt.data_dir_, cista::mmap::protection::READ};

  //determine number of nodes and ways accessable by car, foot
  std::cout << "Total nodes: " << w.r_->node_properties_.size() << std::endl;
      std::cout << "Total ways: " << w.r_->way_properties_.size() << std::endl;

  long long car_nodes = 0;
  long long foot_nodes = 0;
  long long car_ways = 0;
  long long foot_ways = 0;

  for (auto node_property : w.r_->node_properties_) {
    car_nodes += node_property.is_car_accessible();
    foot_nodes += node_property.is_walk_accessible();
  }

  for (auto way_property : w.r_->way_properties_) {
    car_ways += way_property.is_car_accessible();
    foot_ways += way_property.is_foot_accessible();
  }

  std::cout << "Car nodes: " << car_nodes << std::endl;
  std::cout << "Foot nodes: " << foot_nodes << std::endl;
  std::cout << "Car ways: " << car_ways << std::endl;
        std::cout << "Foot ways: " << foot_ways << std::endl;

  using generic_car_foot_profile = combi_profile<std::tuple<car, foot<false>>, std::tuple<transitions::is_parking>>;
  using specialized_car_foot_profile = car_parking<false>;
  using car_profile = car;
  using foot_profile = foot<false>;

  fs::path result_path = "./results/";
  fs::create_directories(result_path);

  fs::path foot_result_path = result_path / "foot.csv";
  fs::path car_result_path = result_path / "car.csv";
  fs::path specialized_result_path = result_path / "specialized.csv";
  fs::path generic_result_path = result_path / "generic.csv";


  runBenchmark<foot_profile>(foot_result_path, opt.threads_, opt.n_queries_, opt.max_dist_, w);
  runBenchmark<car_profile>(car_result_path, opt.threads_, opt.n_queries_, opt.max_dist_, w);
  runBenchmark<specialized_car_foot_profile>(specialized_result_path, opt.threads_, opt.n_queries_, opt.max_dist_, w);
  runBenchmark<generic_car_foot_profile>(generic_result_path, opt.threads_, opt.n_queries_, opt.max_dist_, w);
}
