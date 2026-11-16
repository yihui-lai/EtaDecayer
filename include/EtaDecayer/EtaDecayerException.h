// Exception thrown by EtaDecayer: a std::exception with a category ("Configuration" for invalid settings, "EtaDecayer"
// for failures during generation) to which the message is streamed, as in
//   throw etadecayer::Exception("EtaDecayer") << "text " << value;
#ifndef ETADECAYER_ETA_DECAYER_EXCEPTION_H
#define ETADECAYER_ETA_DECAYER_EXCEPTION_H

#include <exception>
#include <sstream>
#include <string>

namespace etadecayer {
  class Exception : public std::exception {
  public:
    explicit Exception(std::string category) : category_(std::move(category)), what_(category_ + ": ") {}

    template <typename T>
    Exception& operator<<(const T& value) {
      std::ostringstream os;
      os << value;
      message_ += os.str();
      what_ = category_ + ": " + message_;
      return *this;
    }

    const char* what() const noexcept override { return what_.c_str(); }
    const std::string& category() const { return category_; }
    const std::string& message() const { return message_; }

  private:
    std::string category_, message_, what_;
  };
}  // namespace etadecayer

#endif
