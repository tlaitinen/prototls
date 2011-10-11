/** prototls - Portable asynchronous client/server communications C++ library 
   
     See LICENSE for copyright information.
*/
#ifndef _prototls_tsdeque_hpp_
#define _prototls_tsdeque_hpp_
#include <deque>
#include <boost/thread/mutex.hpp>
namespace prototls {
    /** thread safe queue */
    template<class T>
        class TSDeque {
            boost::mutex monitor;
            std::deque<T> elems;
        public:
            /** tries to pop the first element. does nothing
             if the queue is empty */
            void try_pop_front(T& e) {
                boost::mutex::scoped_lock lock(monitor);
                if (!elems.empty()) {
                    e = elems.front();
                    elems.pop_front();
                }
            }
            /** pushes an element to the end of the queue */
            void push_back(T e) {
                boost::mutex::scoped_lock lock(monitor);
                elems.push_back(e);
            }
        };
}
#endif
