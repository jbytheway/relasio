#ifndef RELASIO_READLINE_HPP
#define RELASIO_READLINE_HPP

#include <boost/function.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/parameter/name.hpp>
#include <boost/parameter/preprocessor.hpp>
#include <boost/asio/io_service.hpp>

#include <relasio/api.hpp>

#if BOOST_PARAMETER_MAX_ARITY < 6
#error Please increase BOOST_PARAMETER_MAX_ARITY
#endif

namespace relasio {

class readline;

BOOST_PARAMETER_NAME(io_service)
BOOST_PARAMETER_NAME(command_handler)
BOOST_PARAMETER_NAME(eof_handler)
BOOST_PARAMETER_NAME(prompt)
BOOST_PARAMETER_NAME(history_file)
BOOST_PARAMETER_NAME(history_filter)

namespace detail {

inline bool default_history_filter(std::string const& s) {
  return !s.empty();
}

RELASIO_API void default_eof_handler(readline&);

struct readline_constructor_impl {
  class impl;

  template<typename ArgumentPack>
  readline_constructor_impl(ArgumentPack const& args)
  {
    initialize(
      args[_io_service],
      args[_command_handler],
      args[_eof_handler | default_eof_handler],
      args[_prompt | "> "],
      args[_history_file | boost::filesystem::path()],
      args[_history_filter | default_history_filter]
    );
  }

  RELASIO_API void initialize(
    boost::asio::io_service& io_service,
    boost::function<void(std::string const&, readline&)> const& command_handler,
    boost::function<void(readline&)> const& eof_handler,
    std::string const& prompt,
    boost::filesystem::path const& history_file,
    boost::function<bool(std::string const&)> const& history_filter
  );

  // Pimpl
  std::shared_ptr<impl> impl_;
};

}

class RELASIO_API readline :
  private detail::readline_constructor_impl,
  private boost::noncopyable
{
  public:
    BOOST_PARAMETER_CONSTRUCTOR(
      readline,
      (detail::readline_constructor_impl),
      tag,
      (required
        (in_out(io_service), (boost::asio::io_service))
        (command_handler, *)
      )
      (optional
        (eof_handler, *)
        (prompt, (std::string))
        (history_file, (boost::filesystem::path))
        (history_filter, *)
      )
    )
    void write(std::string const&);

    void set_prompt(std::string const&);

    void stop();

    friend struct readline_constructor_impl;
};

}

#endif // RELASIO_READLINE_HPP

