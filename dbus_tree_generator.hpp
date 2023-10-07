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
    struct Iterator {
      ObjectIter objectIter;
      InterfaceIter interfaceIter;
      PropIter propIter;
      ObjectIter eObjectIter;
      Iterator(ObjectIter o, InterfaceIter i, PropIter p, ObjectIter e)
          : objectIter(o), interfaceIter(i), propIter(p), eObjectIter(e) {}
      Iterator &operator++() {
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
      Iterator operator++(int) {
        auto temp = *this;
        ++(*this);
        return temp;
      }
      friend bool operator==(const Iterator &f, const Iterator &s) {
        return f.objectIter == s.objectIter &&
               f.interfaceIter == s.interfaceIter && s.propIter == f.propIter;
      }
      friend bool operator!=(const Iterator &f, const Iterator &s) {
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
      return Iterator(objIter, interfaceIter, propIter,
                      std::end(managedObjects));
    }
    auto end() {
      return Iterator(std::end(managedObjects), InterfaceIter{}, PropIter{},
                      std::end(managedObjects));
    }
  };

  ManagedObjectType &managedObjects;
  View view;
  View::Iterator iter;
  DbusTreeGenerator(ManagedObjectType &obj)
      : managedObjects(obj), view(managedObjects), iter(view.begin()) {}
  auto operator()(bool &moreAvailable) {
    auto ret = *iter;
    moreAvailable = ++iter != view.end();
    return ret;
  }
};
} // namespace utility
} // namespace dbus