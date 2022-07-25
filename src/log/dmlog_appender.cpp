#include <fc/log/dmlog_appender.hpp>
#include <fc/log/log_message.hpp>
#include <fc/string.hpp>
#include <fc/variant.hpp>
#include <fc/reflect/variant.hpp>
#ifndef WIN32
#include <unistd.h>
#include <signal.h>
#endif
#include <boost/asio/io_context.hpp>
#include <boost/thread/mutex.hpp>
#include <fc/exception/exception.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <iomanip>
#include <mutex>
#include <sstream>

namespace fc {
   class dmlog_appender::impl {
      public:
         bool is_stopped = false;
         FILE* out = nullptr;
         bool owns_out = false;
         bool is_fifo = false;
   };

   dmlog_appender::dmlog_appender( const std::optional<dmlog_appender::config>& args )
   :dmlog_appender(){
      if (!args || (args->file == "-" && args->fifo == "-"))
      {
         my->out = stdout;
      }
      else if (args->fifo != "-") {
         auto fd = open(args->fifo.c_str(), O_RDWR | O_NONBLOCK);
         if (fd < 0) {
            FC_THROW("open failed for path ${name}, errno ${errno} -- ${errstr}", ("name", args->fifo) ("errno", errno) ("errstr", strerror(errno)));
         }

         my->out = fdopen(fd, "a");
         if (my->out)
         {
            my->owns_out = true;
            my->is_fifo = true;
         }
         else
         {
            FC_THROW("Failed to open deep mind log file ${name}", ("name", args->fifo));
         }
      }
      else {
         my->out = std::fopen(args->file.c_str(), "a");
         if (my->out)
         {
            std::setbuf(my->out, nullptr);
            my->owns_out = true;
         }
         else
         {
            FC_THROW("Failed to open deep mind log file ${name}", ("name", args->file));
         }
      }
   }

   dmlog_appender::dmlog_appender( const variant& args )
   :dmlog_appender(args.as<std::optional<config>>()){}

   dmlog_appender::dmlog_appender()
   :my(new impl){}

   dmlog_appender::~dmlog_appender() {
      if (my->owns_out)
      {
         std::fclose(my->out);
      }
   }

   void dmlog_appender::initialize( boost::asio::io_service& io_service ) {}

   void dmlog_appender::log( const log_message& m ) {
      FILE* out = my->out;

      string message = format_string( "DMLOG " + m.get_format() + "\n", m.get_data() );

      auto remaining_size = message.size();
      auto message_ptr = message.c_str();
      while (!my->is_stopped && remaining_size) {
         auto written = fwrite(message_ptr, sizeof(char), remaining_size, out);

         // In FIFO mode, fwrite fails with EAGAIN when no process opens FIFO for reading. Do not stop current process if this happens.
         // EINTR shouldn't happen anymore, but keep this detection, just in case.
         if(written == 0 && errno != EINTR && (!(my->is_fifo && errno == EAGAIN)))
         {
            my->is_stopped = true;
         }

         if(written != remaining_size)
         {
            if(!(my->is_fifo && errno == EAGAIN)) {
               fprintf(stderr, "DMLOG FPRINTF_FAILED failed written=%lu remaining=%lu %d %s\n", written, remaining_size, ferror(out), strerror(errno));
            }
            clearerr(out);
         }

         if(my->is_stopped)
         {
            fprintf(stderr, "DMLOG FPRINTF_FAILURE_TERMINATED\n");
            // Depending on the error, we might have already gotten a SIGPIPE
            // An extra signal is harmless, though.  Use a process targeted
            // signal (not raise) because the SIGTERM may be blocked in this
            // thread.
            kill(getpid(), SIGTERM);
         }

         message_ptr = &message_ptr[written];
         remaining_size -= written;
      }
   }
}
