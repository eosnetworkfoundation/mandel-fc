#pragma once
#include <fc/exception/exception.hpp>
#include <fc/io/cfile.hpp>
#include <fc/io/datastream.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/raw.hpp>
#include <fstream>

namespace fc {

   /**
    * @class tracked_storage
    * @brief tracks the size of storage allocated to its underlying multi_index
    *
    * This class wraps a multi_index container and tracks the memory allocated as
    * the container creates, modifies, and deletes. It also provides read and write
    * methods for persistence.
    * 
    * Requires ContainerType::value_type to have a size() method that represents the
    * memory used for that object and is required to be a pack/unpack-able type.
    */

   template <typename ContainerType>
   class tracked_storage {
   private:
      size_t _size = 0;
      ContainerType _index;
   public:
      typedef typename ContainerType::template nth_index<0>::type primary_index_type;

      tracked_storage() = default;

      // read in the contents of a persisted tracked_storage and limit the storage to
      // max_memory.
      // returns true if entire persisted tracked_storage was read
      bool read(fc::cfile_datastream& ds, size_t max_memory) {
         auto container_size = _index.size();
         fc::raw::unpack(ds, container_size);
         for (size_t i = 0; i < container_size; ++i) {
            if (size() >= max_memory) {
               return false;
            }
            ContainerType::value_type v;
            fc::raw::unpack(ds, v);
            insert(std::move(v));
         }

         return true;
      }

      void write(fc::cfile& dat_content) const {
         const auto container_size = _index.size();
         dat_content.write( reinterpret_cast<const char*>(&container_size), sizeof(container_size) );
         const primary_index_type& primary_idx = _index.template get<0>();
         
         for (auto itr = primary_idx.cbegin(); itr != primary_idx.cend(); ++itr) {
            auto data = fc::raw::pack(*itr);
            dat_content.write(data.data(), data.size());
         }
      }

      void insert(ContainerType::value_type&& obj) {
         _size += obj.size();
         _index.insert(std::move(obj));
      }

      void insert(ContainerType::value_type obj) {
         _size += obj.size();
         _index.insert(std::move(obj));
      }

      template<typename Key>
      typename primary_index_type::iterator find(const Key& key) {
         primary_index_type& primary_idx = _index.template get<0>();
         return primary_idx.find(key);
      }

      template<typename Key>
      typename primary_index_type::const_iterator find(const Key& key) const {
         const primary_index_type& primary_idx = _index.template get<0>();
         return primary_idx.find(key);
      }

      template<typename Lam>
      void modify(typename primary_index_type::iterator itr, Lam lam) {
         const auto orig_size = itr->size();
         _index.modify( itr, std::move(lam));
         _size += itr->size() - orig_size;
      }

      template<typename Key>
      void erase(const Key& key) {
         auto itr = _index.find(key);
         if (itr == _index.end())
            return;

         _size -= itr->size();
         _index.erase(itr);
      }

      size_t size() const {
         return _size;
      }

      const ContainerType& index() const {
         return _index;
      }
   };
}
