#pragma once
#include <fc/io/cfile.hpp>
#include <fc/io/raw.hpp>

namespace fc {

namespace persistence_util {
   cfile read_persistence_file(const fc::path& dir, const std::string& filename, const uint32_t magic_number,
      const uint32_t min_supported_version, const uint32_t max_supported_version) {
      if (!fc::is_directory(dir))
         fc::create_directories(dir);
      
      auto dat_file = dir / filename;

      cfile dat_content;
      dat_content.set_file_path(dat_file);
      dat_content.open(cfile::update_rw_mode);

      auto ds = dat_content.create_datastream();

      // validate totem
      uint32_t totem = 0;
      fc::raw::unpack( ds, totem );
      if( totem != magic_number) {
         FC_THROW_EXCEPTION(fc::parse_error_exception,
                            "File '${filename}' has unexpected magic number: ${actual_totem}. Expected ${expected_totem}",
                            ("filename", dat_file.generic_string())
                            ("actual_totem", totem)
                            ("expected_totem", magic_number));
      }

      // validate version
      uint32_t version = 0;
      fc::raw::unpack( ds, version );
      if( version < min_supported_version || version > max_supported_version) {
         FC_THROW_EXCEPTION(fc::parse_error_exception,
                            "Unsupported version of file '${filename}'. "
                            "Version is ${version} while code supports version(s) [${min},${max}]",
                            ("filename", dat_file.generic_string())
                            ("version", version)
                            ("min", min_supported_version)
                            ("max", max_supported_version));
      }
      return dat_content;
   }

   cfile write_persistence_file(const fc::path& dir, const std::string& filename, const uint32_t magic_number, const uint32_t current_version) {
      if (!fc::is_directory(dir))
         fc::create_directories(dir);

      auto dat_file = dir / filename;
      cfile dat_content;
      dat_content.set_file_path(dat_file.generic_string().c_str());
      dat_content.open( cfile::truncate_rw_mode );
      dat_content.write( reinterpret_cast<const char*>(&magic_number), sizeof(magic_number) );
      dat_content.write( reinterpret_cast<const char*>(&current_version), sizeof(current_version) );
      return dat_content;
   }
}


inline cfile cfile::read_dat_file(const fc::path& dir, const std::string& filename, const uint32_t magic_number,
   const uint32_t min_supported_version, const uint32_t max_supported_version) {
   if (!fc::is_directory(dir))
      fc::create_directories(dir);
   
   auto dat_file = dir / filename;
   using cfile_stream = fc::datastream<fc::cfile>;
   cfile_stream dat_content;
   dat_content.set_file_path(dat_file);
   dat_content.open(cfile::update_rw_mode);

   // validate totem
   uint32_t totem = 0;
   fc::raw::unpack( dat_content, totem );
   if( totem != magic_number) {
      FC_THROW_EXCEPTION(fc::parse_error_exception,
                         "File '${filename}' has unexpected magic number: ${actual_totem}. Expected ${expected_totem}",
                         ("filename", dat_file.generic_string())
                         ("actual_totem", totem)
                         ("expected_totem", magic_number));
   }

   // validate version
   uint32_t version = 0;
   fc::raw::unpack( dat_content, version );
   if( version < min_supported_version || version > max_supported_version) {
      FC_THROW_EXCEPTION(fc::parse_error_exception,
                         "Unsupported version of file '${filename}'. "
                         "Version is ${version} while code supports version(s) [${min},${max}]",
                         ("filename", dat_file.generic_string())
                         ("version", version)
                         ("min", min_supported_version)
                         ("max", max_supported_version));
   }
   return dat_content;
}

} // namespace fc

#ifndef _WIN32
#undef FC_FOPEN
#else
#undef FC_CAT
#undef FC_PREL
#undef FC_FOPEN
#endif
