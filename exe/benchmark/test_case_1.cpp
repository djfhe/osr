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
    param(output_path_, "output,o", "Output path");
  }

  fs::path data_dir_{"osr"};
  unsigned n_queries_{50};
  unsigned max_dist_{1200};
  unsigned threads_{std::thread::hardware_concurrency()};
  fs::path output_path_{"results.csv"};
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

  //using profile = combi_profile<std::tuple<foot<false>>, std::tuple<>>;
  using profile = combi_profile<std::tuple<foot<false>, car, foot<false>>, std::tuple<transitions::is_parking, transitions::is_parking>>;

  auto count = std::max(1U, opt.threads_);

  auto run = benchmark::test_case<profile>{static_cast<cost_t>(opt.max_dist_), w};
  run.runMany<direction::kForward, false>(count, opt.n_queries_);
  run.wait();

  benchmark::csv_writer writer(opt.output_path_, {"thread index", "duration in microseconds"});

  run.write(writer);
}
