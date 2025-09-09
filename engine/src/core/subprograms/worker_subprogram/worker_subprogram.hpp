#ifndef INCLUDE_WORKER_SUBPROGRAM_HPP_
#define INCLUDE_WORKER_SUBPROGRAM_HPP_

#include "core/subprogram_locator.hpp"

namespace core::subprograms {
    
    using Process = std::function<void(std::any*, SubprogramLocator::ParentsData*)>;

    void install_worker(SubprogramInfo& info, const Process& process);
}

#endif  // INCLUDE_WORKER_SUBPROGRAM_HPP_