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
    param(n_queries_, "n", "Number of queries");
    param(max_dist_, "r", "Radius");
  }

  fs::path data_dir_{"osr"};
  unsigned n_queries_{100};
  unsigned max_dist_{1200};
};

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

  using car_profile = car;
  using generic_car_profile = combi_profile<std::tuple<car>, std::tuple<>>;

  static_assert(sizeof(car_profile::node) == 8);
  static_assert(sizeof(generic_car_profile::node) == 12);
  static_assert(sizeof(car_profile::label) == 8);
  static_assert(sizeof(generic_car_profile::label) == 16);
  static_assert(sizeof(car_profile::entry) == 232);
  static_assert(sizeof(generic_car_profile::entry) == 240);
  static_assert(sizeof(generic_car_profile::entry::pred_) == 1);

  auto run = benchmark::test_case<car_profile>{static_cast<cost_t>(opt.max_dist_), w};
  run.template runSingle<direction::kForward, false>(opt.n_queries_, std::nullopt);
}
