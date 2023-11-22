#pragma once

#include "svc_discovery/HandlerManager.h"

namespace psychopomp {
    class Bin {
        Bin(std::string binName);

        void updateState();

        void registerRequestHandler();
    };

}