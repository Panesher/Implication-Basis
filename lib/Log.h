#pragma once

#include <iostream>
#include <sstream>

constexpr int DEBUG = 0;
constexpr int INFO = 1;
constexpr int WARNING = 2;
constexpr int ERROR = 3;

namespace loger_settings {
void set_log_level_internal(int level, const std::string &file, int line,
                            const std::string &function);
}  // namespace loger_settings

class Logger {
 public:
  explicit Logger(int level, const std::string &file, int line,
                  const std::string &function);

  template <class T>
  Logger &operator<<(const T &t) {
    if (is_enabled()) {
      get_underlying() << t;
    }
    return *this;
  }

  ~Logger() noexcept;

 private:
  const int level_;
  const int line_;
  const std::string file_;
  const std::string function_;
  std::stringstream stream_;

  bool used_;

  bool is_enabled() const;

  std::stringstream &get_underlying();

  std::string format_level() const;
  std::string get_color() const;
};

#define LOG(x) Logger(x, __FILE__, __LINE__, __FUNCTION__)
#define SET_LOG_LEVEL_VERBOSITY(x) \
  loger_settings::set_log_level_internal(x, __FILE__, __LINE__, __FUNCTION__)
