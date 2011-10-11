/** prototls - Portable asynchronous client/server communications C++ library 
   
     See LICENSE for copyright information.
*/
#ifndef _prototls_common_hpp_
#define _prototls_common_hpp_
#include <string>
#include <sstream>
namespace prototls {
    /** a convenience method to convert objects to strings */
    template <class T> std::string toString(T t) {
        std::stringstream s;
        s << t;
        return s.str();
    }
}
#endif
