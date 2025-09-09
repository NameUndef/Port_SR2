#ifndef INCLUDE_THREAD_POOL_SUBPROGRAM_HPP_
#define INCLUDE_THREAD_POOL_SUBPROGRAM_HPP_

#include "core/subprogram_locator.hpp"
#include "thread_pool.hpp"

namespace core::subprograms {
    inline const char* THREAD_POOL_SUBPROGRAM_NAME = "thread_pool";

    template <>
    ThreadPool* get_parent<ThreadPool>(SubprogramLocator* locator);
    
    void install_thread_pool_subprogram(SubprogramLocator& subprogram_locator);
}

#endif  // INCLUDE_THREAD_POOL_SUBPROGRAM_HPP__HPP_