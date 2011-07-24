#include <boost/asio/deadline_timer.hpp>

#include <relasio/readline.hpp>

namespace asio = boost::asio;

struct CommandHandler {
  void operator()(std::string const& s, relasio::readline& readline) const {
    readline.write("You said: "+s);
  }

  asio::deadline_timer& timer_;
};

struct F {
  void operator()(const boost::system::error_code& ec) {
    if (ec) {
      std::ostringstream os;
      os << "Error: " << ec;
      readline_.write(os.str());
    } else {
      std::ostringstream os;
      os << "Time! " << count_;
      readline_.write(os.str());
      if (--count_) {
        timer_.expires_from_now(boost::posix_time::seconds(2));
        timer_.async_wait(*this);
      }
    }
  }

  std::uint32_t count_;
  asio::deadline_timer& timer_;
  relasio::readline& readline_;
};

int main()
{
  asio::io_service io;
  asio::deadline_timer timer(io, boost::posix_time::seconds(2));
  CommandHandler command_handler = { timer };
  relasio::readline readline(io, command_handler);
  F handler = { 3, timer, readline };
  timer.async_wait(handler);
  io.run();
  return 0;
}

