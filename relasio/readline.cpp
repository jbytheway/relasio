#include <relasio/readline.hpp>

#include <readline/readline.h>
#include <readline/history.h>

#include <iostream>

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/placeholders.hpp>

#include <relasio/relasio_error.hpp>

namespace relasio {

namespace detail {

// static data for callback
static readline_constructor_impl::impl* current_readline;

void default_eof_handler(readline& r)
{
  r.stop();
}

struct readline_constructor_impl::impl {
  impl(
    readline* parent,
    boost::asio::io_service& io_service,
    boost::function<void(std::string const&, readline&)> const& command_handler,
    boost::function<void(readline&)> const& eof_handler,
    std::string const& prompt,
    boost::filesystem::path const& history_file,
    boost::function<bool(std::string const&)> const& history_filter
  );
  ~impl();

  void wait_for_chars();
  void chars_available(
    boost::system::error_code const&,
    size_t bytes_transferred
  );
  void line(char*);
  void write(std::string const& message);
  void queue_redisplay();
  void redisplay(boost::system::error_code const&);
  void set_prompt(std::string const& prompt);
  void stop();

  // Things set in constructor
  readline* parent_;
  boost::asio::io_service& io_service_;
  boost::function<void(std::string const&, readline&)>
    command_handler_;
  boost::function<void(readline&)> eof_handler_;
  std::string prompt_;
  boost::filesystem::path const history_file_;
  boost::function<bool(std::string const&)> history_filter_;

  // Things used along the way
  std::string last_line_;
  boost::asio::posix::stream_descriptor stdin_reader_;
  boost::asio::deadline_timer redisplay_timer_;
  boost::asio::null_buffers buffers_;
};

// Callback function to pass to readline
extern "C" {
  static void relasio_line_callback_handler(char* line)
  {
    assert(current_readline);
    current_readline->line(line);
  }
}

void readline_constructor_impl::initialize(
  boost::asio::io_service& io_service,
  boost::function<void(std::string const&, readline&)> const& command_handler,
  boost::function<void(readline&)> const& eof_handler,
  std::string const& prompt,
  boost::filesystem::path const& history_file,
  boost::function<bool(std::string const&)> const& history_filter
)
{
  impl_.reset(new impl(
      static_cast<readline*>(this),
      io_service, command_handler, eof_handler, prompt, history_file,
      history_filter
  ));
}

readline_constructor_impl::impl::impl(
  readline* parent,
  boost::asio::io_service& io_service,
  boost::function<void(std::string const&, readline&)> const& command_handler,
  boost::function<void(readline&)> const& eof_handler,
  std::string const& prompt,
  boost::filesystem::path const& history_file,
  boost::function<bool(std::string const&)> const& history_filter
) :
  parent_(parent),
  io_service_(io_service),
  command_handler_(command_handler),
  eof_handler_(eof_handler),
  prompt_(prompt),
  history_file_(history_file),
  history_filter_(history_filter),
  stdin_reader_(io_service, 0), // Bind to stdin
  redisplay_timer_(io_service)
{
  if (current_readline) {
    throw std::logic_error("Can't create multiple relasio::readline objects");
  }
  current_readline = this;
  if (!history_file_.empty() && boost::filesystem::exists(history_file_)) {
    if (!boost::filesystem::is_directory(history_file_)) {
      if (0 == read_history(history_file_.c_str())) {
        if (0 == history_set_pos(history_length)) {
          throw relasio_error("error setting history position\n");
        }
        if (history_length > 0) {
          // Set up last_line_
          HIST_ENTRY* last = history_get(history_length);
          if (last) last_line_ = last->line;
        }
      } else {
        throw relasio_error("error reading history\n");
      }
    }
  }
  rl_callback_handler_install(
    prompt_.c_str(), relasio_line_callback_handler
  );
  wait_for_chars();
}


readline_constructor_impl::impl::~impl()
{
  try {
    stop();
  } catch (...) {
    // Ignore errors
  }
}

void readline_constructor_impl::impl::wait_for_chars()
{
  stdin_reader_.async_read_some(
    buffers_,
    boost::bind(
      &impl::chars_available, this,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred
    )
  );
}

void readline_constructor_impl::impl::chars_available(
  boost::system::error_code const& ec,
  size_t bytes_transferred
)
{
  static_cast<void>(bytes_transferred);
  assert(bytes_transferred == 0);
  if (ec) {
    stop();
  } else {
    rl_callback_read_char();
    wait_for_chars();
  }
}

void readline_constructor_impl::impl::line(char* l)
{
  if (!l) {
    std::cout << std::endl;
    io_service_.post(std::bind(eof_handler_, boost::ref(*parent_)));
  } else {
    if (history_filter_(l) && l != last_line_) {
      add_history(l);
      last_line_ = l;
    }
    std::string s(l);
    io_service_.post(std::bind(command_handler_, s, boost::ref(*parent_)));
    free(l);
  }
}

void readline_constructor_impl::impl::write(std::string const& message)
{
  if (current_readline == this) {
    std::cout << '\n' << message << std::flush;
    queue_redisplay();
  } else {
    // We are no longer the "proper" readline instance.  Most likely this
    // happens during program shutdown.  For the benefit of such circumstances
    // we still print the message, but no prompt
    std::cout << message << std::endl;
  }
}

void readline_constructor_impl::impl::queue_redisplay()
{
  auto time = boost::posix_time::milliseconds(50);
  redisplay_timer_.expires_from_now(time);
  redisplay_timer_.async_wait(
    boost::bind(&impl::redisplay, this, boost::asio::placeholders::error)
  );
}

void readline_constructor_impl::impl::redisplay(
  boost::system::error_code const& ec
)
{
  if (ec) {
    return;
  }

  std::cout << std::endl;
  rl_on_new_line();
  rl_redisplay();
}

void readline_constructor_impl::impl::set_prompt(std::string const& prompt)
{
  assert(current_readline == this);
  prompt_ = prompt;
  rl_set_prompt(prompt_.c_str());
  queue_redisplay();
}

void readline_constructor_impl::impl::stop()
{
  if (current_readline == NULL) {
    // We're already stopped
    return;
  }

  assert(detail::current_readline == this);
  stdin_reader_.cancel();
  redisplay_timer_.cancel();
  rl_callback_handler_remove();
  detail::current_readline = NULL;
  std::cout << std::endl;

  if (!history_file_.empty()) {
    boost::filesystem::create_directories(history_file_.parent_path());
    if (0 == write_history(history_file_.c_str())) {
      if (0 != history_truncate_file(
            history_file_.c_str(), history_length
          )) {
        throw relasio_error("error truncating history\n");
      }
    } else {
      throw relasio_error("error writing history\n");
    }
  }
}

} // namespace detail

void readline::write(std::string const& message)
{
  impl_->write(message);
}

void readline::set_prompt(std::string const& prompt)
{
  impl_->set_prompt(prompt);
}

void readline::stop()
{
  impl_->stop();
}

}

