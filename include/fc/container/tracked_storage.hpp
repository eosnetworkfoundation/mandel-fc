#pragma once
#include <fc/exception/exception.hpp>
#include <fc/io/cfile.hpp>
#include <fc/io/datastream.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/raw.hpp>
#include <fstream>

namespace eosio::chain_apis {

   /**
    * @class tracked_storage
    * @brief tracks the size of storage allocated to its underlying multi_index
    *
    * This class wraps a multi_index container and tracks the memory allocated as
    * the container creates, modifies, and deletes.
    */

   template <typename ContainerType, typename Value, typename PrimaryTag>
   class tracked_storage {
   private:
      uint64_t _size = 0;
      ContainerType _index;
   public:
      typedef PrimaryTag primary_tag;
      typedef typename ContainerType::template index<primary_tag>::type primary_index_type;

      tracked_storage() {

      }

      void read(fc::cfile_datastream& ds, uint64_t max_memory) {
         auto container_size = _index.size();
         fc::raw::unpack(ds, container_size);
         for (uint64_t i = 0; i < container_size && size() < max_memory; ++i) {
            Value v;
            fc::raw::unpack(ds, v);
            insert(std::move(v));
         }
      }

      void write(fc::cfile& dat_content) const {
         const auto container_size = _index.size();
         dat_content.write( reinterpret_cast<const char*>(&container_size), sizeof(container_size) );
         const primary_index_type& primary_idx = _index.template get<primary_tag>();
         
         for (auto itr = primary_idx.cbegin(); itr != primary_idx.cend(); ++itr) {
            auto data = fc::raw::pack(*itr);
            dat_content.write(data.data(), data.size());
         }
      }

      void insert(Value&& obj) {
         _index.insert(obj);
         _size += obj.size();
      }

      template<typename Key>
      typename primary_index_type::iterator find(const Key& key) {
         primary_index_type& primary_idx = _index.template get<primary_tag>();
         return primary_idx.find(key);
      }

      template<typename Key>
      typename primary_index_type::const_iterator find(const Key& key) const {
         const primary_index_type& primary_idx = _index.template get<primary_tag>();
         return primary_idx.find(key);
      }

      template<typename Lam>
      void modify(typename primary_index_type::iterator itr, Lam&& lam) {
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

      uint64_t size() const {
         return _size;
      }

      const ContainerType& index() const {
         return _index;
      }
   };
}
