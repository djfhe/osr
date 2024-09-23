#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

namespace benchmark {

class csv_writer {
public:
  csv_writer(const std::filesystem::path& filepath, const std::vector<std::string>& headers)
        : filepath_(filepath), headers_(headers), first_row_(true) {
    // Open file and write headers
    file_.open(filepath_);
    write_row(headers_);
  }

  ~csv_writer() {
    // Close the file if it's open
    if (file_.is_open()) {
      file_.close();
    }
  }

  template<typename T>
  void write_row(const std::vector<T>& row) {
    if (!file_.is_open()) return;
    std::ostringstream oss;
    for (size_t i = 0; i < row.size(); ++i) {
      oss << row[i];
      if (i < row.size() - 1) {
        oss << ",";
      }
    }
    file_ << oss.str() << "\n";
  }

private:
  std::filesystem::path filepath_;
  std::ofstream file_;
  std::vector<std::string> headers_;
  bool first_row_;
};

} // namespace benchmark
