#include "Log.h"

#include <mutex>

namespace {

#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_BOLD_RED "\033[1m\033[31m"
#define COLOR_BOLD_MAGENTA "\033[1m\033[35m"

static std::mutex logger_mutex;
static int LOG_VERBOSITY = INFO;
static int setted_from_line;
static std::string setted_from_file;
static std::string setted_from_function;
static bool is_log_verbosity_setted = false;

}  // namespace

namespace loger_settings {

void set_log_level_internal(int level, const std::string &file, int line,
                            const std::string &function) {
  if (level < DEBUG || level > ERROR) {
    Logger(ERROR, file, line, function)
        << "Log level '" << level << "' verbosity outside proper range ["
        << DEBUG << ", " << ERROR << "]";
    return;
  }
  if (is_log_verbosity_setted) {
    Logger(ERROR, file, line, function)
        << "Log level verbosity already setted from: [" << setted_from_file
        << "(" << setted_from_line << "): " << function << "]";
    return;
  }
  is_log_verbosity_setted = true;
  setted_from_file = file;
  setted_from_line = line;
  setted_from_function = function;
  LOG_VERBOSITY = level;
}

}  // namespace loger_settings

Logger::Logger(int level, const std::string &file, int line,
               const std::string &function)
    : level_(level),
      file_(file),
      line_(line),
      function_(function),
      used_(false) {}

Logger::~Logger() noexcept {
  try {
    if (is_enabled()) {
      std::lock_guard<std::mutex> locker(logger_mutex);
      std::cerr << get_color() << stream_.str() << COLOR_RESET << std::endl;
    }
  } catch (...) {
    // ¯\_(ツ)_/¯
  }
}

bool Logger::is_enabled() const { return LOG_VERBOSITY <= level_; }

std::stringstream &Logger::get_underlying() {
  if (!used_) {
    used_ = true;
    stream_ << "[" << format_level() << ":" << file_ << "(" << line_
            << "):" << function_ << "] ";
  }
  return stream_;
}

std::string Logger::format_level() const {
  switch (level_) {
    case DEBUG:
      return "DEBUG";
    case INFO:
      return "INFO";
    case WARNING:
      return "WARNING";
    case ERROR:
      return "ERROR";
    default:
      return "UNKNOWN";
  }
}

std::string Logger::get_color() const {
  switch (level_) {
    case DEBUG:
      return COLOR_GREEN;
    case INFO:
      return COLOR_BLUE;
    case WARNING:
      return COLOR_BOLD_MAGENTA;
    case ERROR:
      return COLOR_BOLD_RED;
    default:
      return COLOR_RESET;
  }
}
