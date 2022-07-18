#include <fstream>
#include <sstream>

#include <fc/filesystem.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/fstream.hpp>
#include <fc/log/logger.hpp>


namespace fc {

   void read_file_contents( const fc::path& filename, std::string& result )
   {
      std::ifstream f( filename, std::ios::in | std::ios::binary );
      FC_ASSERT(f, "Failed to open ${filename}", ("filename", filename));
      // don't use fc::stringstream here as we need something with override for << rdbuf()
      std::stringstream ss;
      ss << f.rdbuf();
      FC_ASSERT(f, "Failed reading ${filename}", ("filename", filename));
      result = ss.str();
   }
  
} // namespace fc 
