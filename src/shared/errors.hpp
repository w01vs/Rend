#ifndef ERRORS_HPP
#define ERRORS_HPP

#include "ast_def.hpp"
#include <iostream>
#include <string>

enum class ErrorType {
    SYNTAX,
    SEMANTIC,
    UNKNOWN,

};

struct Diagnostics {
    std::string message;
    SourceLocation loc;
    ErrorType type;
    std::string file;
    size_t line;
};

class ErrorReporter {
  public:
    void report_error(SourceLocation& loc, const std::string& msg, ErrorType type, std::string file,
                      size_t line)
    {
        diagnostics_.emplace_back(msg, loc, type, file, line);
        error_count_++;
    }

    bool has_errors() const
    {
        return error_count_ > 0;
    }

    void print_diagnostics() const
    {
#if DEBUG
        std::cerr << "Total errors: " << error_count_ << std::endl;
        for(auto& d : diagnostics_)
        {
            std::cerr << "Error in " << d.file << ":" << d.line << " - " << d.message << std::endl;
        }
#else
        for(auto& d : diagnostics_) { std::cerr << d.message << std::endl; }
#endif
    }

  private:
    std::vector<Diagnostics> diagnostics_;
    size_t error_count_ = 0;
};

#endif // ERRORS_HPP
