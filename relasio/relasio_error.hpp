#ifndef RELASIO_RELASIO_ERROR_HPP
#define RELASIO_RELASIO_ERROR_HPP

#include <relasio/api.hpp>

namespace relasio {

class RELASIO_API relasio_error : public std::runtime_error {
  public:
    relasio_error(std::string const& s) :
      std::runtime_error(s)
    {}
};

}

#endif // RELASIO_RELASIO_ERROR_HPP

