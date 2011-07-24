#include <boost/asio/deadline_timer.hpp>

namespace asio = boost::asio;

struct F {
  void operator()(const boost::system::error_code& ec) {
    if (ec) {
      std::cout << "Error: " << ec << std::endl;
    } else {
      std::cout << "Time! " << count_ << std::endl;
      if (--count_) {
        timer_.expires_from_now(boost::posix_time::seconds(2));
        timer_.async_wait(*this);
      }
    }
  }

  std::uint32_t count_;
  asio::deadline_timer& timer_;
};

int main()
{
  asio::io_service io;
  asio::deadline_timer timer(io, boost::posix_time::seconds(2));
  F handler = { 3, timer };
  timer.async_wait(handler);
  io.run();
  return 0;
}

