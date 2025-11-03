#ifndef ERRORS_HPP
#define ERRORS_HPP

#include "ast_def.hpp"
#include <string>
#include <iostream>

enum class ErrorType {
    SYNTAX,
    SEMANTIC,
    UNKNOWN,

};

struct Diagnostics {
    std::string message;
    SourceLocation loc;
    ErrorType type;
};

class ErrorReporter {
  public:
    void report_error(SourceLocation& loc, const std::string& msg, ErrorType type) {
        diagnostics_.emplace_back(msg, loc, type);
        error_count_++;
    }

    bool has_errors() const {
        return error_count_ > 0;
    }

    void print_diagnostics() const {
        for(auto& d : diagnostics_){
            std::cerr << d.message << std::endl;
        }
    }

  private:
  std::vector<Diagnostics> diagnostics_;
  size_t error_count_ = 0;
};

#endif // ERRORS_HPP
