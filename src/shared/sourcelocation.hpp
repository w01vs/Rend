#ifndef SOURCE_LOCATION_HPP
#define SOURCE_LOCATION_HPP

#include <cstddef>

struct SourceLocation {
    size_t line;
    size_t column;
    bool valid = true;
};

#endif // SOURCE_LOCATION_HPP