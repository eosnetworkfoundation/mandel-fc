#pragma once
#include <utility>
#include <memory>

#include <fc/string.hpp>
#include <fc/reflect/typename.hpp>
#include <fc/fwd.hpp>

#include <filesystem>

namespace fc {
  using std::filesystem::path;

  namespace detail
  {
    class path_wrapper
    {
    public:
      path_wrapper(path p) :
        _path(p)
      {
      }
      const path* operator->() const
      {
        return &_path;
      }
    private:
      path _path;
    };
  }

  using std::filesystem::directory_iterator;
  using std::filesystem::recursive_directory_iterator;

  bool     exists( const path& p );
  bool     is_directory( const path& p );
  bool     is_regular_file( const path& p );
  void     create_directories( const path& p );
  void     remove_all( const path& p );
  path     absolute( const path& p );
  path     make_relative(const path& from, const path& to);
  path     canonical( const path& p );
  uint64_t file_size( const path& p );
  uint64_t directory_size( const path& p );
  bool     remove( const path& p );
  void     copy( const path& from, const path& to );
  void     rename( const path& from, const path& to );
  void     resize_file( const path& file, size_t s );
  std::string to_native_ansi_path( const path& p );
  
  // setuid, setgid not implemented.
  // translates octal permission like 0755 to S_ stuff defined in sys/stat.h
  // no-op on Windows.
  void     chmod( const path& p, int perm );

  void     create_hard_link( const path& from, const path& to );

  path        unique_path();
  path        temp_directory_path();

  /** @return the home directory on Linux and OS X and the Profile directory on Windows */
  const path& home_path();

  /** @return the home_path() on Linux, home_path()/Library/Application Support/ on OS X, 
   *  and APPDATA on windows
   */
  const path& app_path();

  /** @return application executable path */
  const fc::path& current_path();

  class variant;
  void to_variant( const fc::path&,  fc::variant&  );
  void from_variant( const fc::variant& , fc::path& );

  template<> struct get_typename<path> { static const char* name()   { return "path";   } };

  /**
   * Class which creates a temporary directory inside an existing temporary directory.
   */
  class temp_file_base
  {
  public:
     inline ~temp_file_base() { remove(); }
     inline operator bool() const { return _path.has_value(); }
     inline bool operator!() const { return !_path.has_value(); }
     const fc::path& path() const;
     void remove();
     void release();
  protected:
     typedef std::optional<fc::path> path_t;
     inline temp_file_base(const path_t& path) : _path(path) {}
     inline temp_file_base(path_t&& path) : _path(std::move(path)) {}
     path_t _path;
  };

  /**
   * Class which creates a temporary directory inside an existing temporary directory.
   */
  class temp_file : public temp_file_base
  {
  public:
     temp_file(temp_file&& other);
     temp_file& operator=(temp_file&& other);
     temp_file(const fc::path& tempFolder = fc::temp_directory_path(), bool create = false);
  };

  /**
   * Class which creates a temporary directory inside an existing temporary directory.
   */
  class temp_directory : public temp_file_base
  {
  public:
     temp_directory(temp_directory&& other);
     temp_directory& operator=(temp_directory&& other);
     temp_directory(const fc::path& tempFolder = fc::temp_directory_path());
  };


#if !defined(__APPLE__)
  // this code is known to work on linux and windows.  It may work correctly on mac, 
  // or it may need slight tweaks or extra includes.  It's disabled now to avoid giving
  // a false sense of security.
# define FC_HAS_SIMPLE_FILE_LOCK
#endif
#ifdef FC_HAS_SIMPLE_FILE_LOCK  
  /** simple class which only allows one process to open any given file. 
   * approximate usage:
   * int main() {
   *   fc::simple_file_lock instance_lock("~/.my_app/.lock");
   *   if (!instance_lock.try_lock()) {
   *     elog("my_app is already running");
   *     return 1;
   *   }
   *   // do stuff here, file will be unlocked when instance_lock goes out of scope
   * }
  */
  class simple_lock_file
  {
  public:
    simple_lock_file(const path& lock_file_path);
    ~simple_lock_file();
    bool try_lock();
    void unlock();
  private:
    class impl;
    std::unique_ptr<impl> my;
  };
#endif // FC_HAS_SIMPLE_FILE_LOCK
}