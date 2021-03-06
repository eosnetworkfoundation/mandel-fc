#include <fstream>
#include <sstream>

#include <fc/filesystem.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/fstream.hpp>
#include <fc/log/logger.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

namespace fc {

   void read_file_contents( const fc::path& filename, std::string& result )
   {
      const boost::filesystem::path& bfp = filename;
      boost::filesystem::ifstream f( bfp, std::ios::in | std::ios::binary );
      FC_ASSERT(f, "Failed to open ${filename}", ("filename", filename));
      // don't use fc::stringstream here as we need something with override for << rdbuf()
      std::stringstream ss;
      ss << f.rdbuf();
      FC_ASSERT(f, "Failed reading ${filename}", ("filename", filename));
      result = ss.str();
   }
  
} // namespace fc 
