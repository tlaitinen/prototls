#ifndef _prototls_tsdeque_hpp_
#define _prototls_tsdeque_hpp_
#include <deque>
#include <boost/thread/mutex.hpp>
namespace prototls {
    template<class T>
        class TSDeque {
            boost::mutex monitor;
            std::deque<T> elems;
        public:
            void try_pop_front(T& e) {
                boost::mutex::scoped_lock lock(monitor);
                if (!elems.empty()) {
                    e = elems.front();
                    elems.pop_front();
                }
            }
            void push_back(T e) {
                boost::mutex::scoped_lock lock(monitor);
                elems.push_back(e);
            }
        };
}
#endif
