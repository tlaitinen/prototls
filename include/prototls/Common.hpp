#ifndef _prototls_common_hpp_
#define _prototls_common_hpp_
#include <string>
#include <sstream>
namespace prototls {
    template <class T> std::string toString(T t) {
        std::stringstream s;
        s << t;
        return s.str();
    }
}
#endif
