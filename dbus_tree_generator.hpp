#pragma once
#include "core/reactor.hpp"
#include "dbus_utility.hpp";
namespace dbus {
namespace utility {
struct DbusTreeGenerator {

  using ObjectIter = ManagedObjectType::iterator;
  using InterfaceIter = ObjectIter::value_type::second_type::iterator;
  using PropIter = InterfaceIter::value_type::second_type::iterator;
  using value_type = std::tuple<sdbusplus::message::object_path, std::string,
                                std::string, DbusVariantType>;
  struct View {

    ManagedObjectType &managedObjects;
    struct iterator {
      ObjectIter objectIter;
      InterfaceIter interfaceIter;
      PropIter propIter;
      ObjectIter eObjectIter;
      iterator(ObjectIter o, InterfaceIter i, PropIter p, ObjectIter e)
          : objectIter(o), interfaceIter(i), propIter(p), eObjectIter(e) {}
      iterator &operator++() {
        if (++propIter == std::end(interfaceIter->second)) {
          if (++interfaceIter == std::end(objectIter->second)) {
            if (++objectIter == eObjectIter) {
              interfaceIter = InterfaceIter{};
              propIter = PropIter{};
              return *this;
            }
            interfaceIter = std::begin(objectIter->second);
          }
          propIter = std::begin(interfaceIter->second);
        }
        return *this;
      }
      iterator operator++(int) {
        auto temp = *this;
        ++(*this);
        return temp;
      }
      friend bool operator==(const iterator &f, const iterator &s) {
        return f.objectIter == s.objectIter &&
               f.interfaceIter == s.interfaceIter && s.propIter == f.propIter;
      }
      friend bool operator!=(const iterator &f, const iterator &s) {
        return !(f == s);
      }
      DbusTreeGenerator::value_type operator*() {
        return std::make_tuple(objectIter->first, interfaceIter->first,
                               propIter->first, propIter->second);
      }
    };
    View(ManagedObjectType &mObjects) : managedObjects(mObjects) {}
    auto begin() {
      auto findBegin = [](auto &outerIter) {
        auto inner = std::begin(outerIter->second);
        while (inner == std::end(outerIter->second)) {
          inner = std::begin((++outerIter)->second);
        }
        return inner;
      };
      auto objIter = std::begin(managedObjects);
      auto interfaceIter = findBegin(objIter);
      auto propIter = findBegin(interfaceIter);
      return iterator(objIter, interfaceIter, propIter,
                      std::end(managedObjects));
    }
    auto end() {
      return iterator(std::end(managedObjects), InterfaceIter{}, PropIter{},
                      std::end(managedObjects));
    }
  };

  ManagedObjectType &managedObjects;
  View view;
  View::iterator iter;
  DbusTreeGenerator(ManagedObjectType &obj)
      : managedObjects(obj), view(managedObjects), iter(view.begin()) {}
  auto operator()(bool &moreAvailable) {
    auto ret = *iter;
    moreAvailable = ++iter != view.end();
    return ret;
  }
};
struct DbusPropertyListGenerator {

  struct View {
    DBusPropertiesMap &propMap;
    using iterator = DBusPropertiesMap::iterator;
    using value_type = iterator::value_type;

    View(DBusPropertiesMap &m) : propMap(m) {}
    iterator begin() { return std::begin(propMap); }
    iterator end() { return std::end(propMap); }
  };
  using value_type = View::value_type;
  DBusPropertiesMap &propMap;
  View view;
  using iterator = View::iterator;
  iterator iter;
  DbusPropertyListGenerator(DBusPropertiesMap &obj)
      : propMap(obj), view(propMap) {
    iter = std::begin(view);
  }
  auto operator()(bool &moreAvailable) {
    auto ret = *iter;
    moreAvailable = ++iter != view.end();
    return ret;
  }
};
struct DbusInterfaceListGenerator {

  struct View {
    DBusInteracesMap &iMap;
    using ImapIterator = DBusInteracesMap::iterator;
    using PropIter = DbusPropertyListGenerator::iterator;
    using value_type = std::tuple<std::string, std::string, DbusVariantType>;
    View(DBusInteracesMap &m) : iMap(m) {}
    struct iterator {

      ImapIterator iter;
      ImapIterator eIter;
      PropIter pIter;
      iterator(ImapIterator b, PropIter p, ImapIterator e)
          : iter(b), pIter(p), eIter(e) {}
      iterator &operator++() {
        if (++pIter == std::end(iter->second)) {
          if (++iter == eIter) {
            pIter = PropIter{};
            return *this;
          }
          pIter = std::begin(iter->second);
        }
        return *this;
      }
      friend bool operator==(const iterator &f, const iterator &s) {
        return f.iter == s.iter && f.pIter == s.pIter;
      }
      friend bool operator!=(const iterator &f, const iterator &s) {
        return !(f == s);
      }
      value_type operator*() {
        return std::make_tuple(iter->first, pIter->first, pIter->second);
      }
    };
    iterator begin() {
      auto findBegin = [](auto &outerIter) {
        auto inner = std::begin(outerIter->second);
        while (inner == std::end(outerIter->second)) {
          inner = std::begin((++outerIter)->second);
        }
        return inner;
      };

      auto interfaceIter = std::begin(iMap);
      auto propIter = findBegin(interfaceIter);
      return iterator(interfaceIter, propIter, std::end(iMap));
    }
    iterator end() {
      return iterator(std::end(iMap), PropIter(), std::end(iMap));
    }
  };
  using value_type = View::value_type;
  DBusInteracesMap &propMap;
  View view;
  View::iterator iter;
  DbusInterfaceListGenerator(DBusInteracesMap &obj)
      : propMap(obj), view(propMap), iter(view.begin()) {}
  auto operator()(bool &moreAvailable) {
    auto ret = *iter;
    moreAvailable = ++iter != view.end();
    return ret;
  }
};
} // namespace utility
} // namespace dbus