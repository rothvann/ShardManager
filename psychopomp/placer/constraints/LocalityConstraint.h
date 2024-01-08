#pragma once

#include "psychopomp/placer/constraints/Constraint.h"

namespace psychopomp {

class LocalityConstraint : public Constraint {
    
};
}

// for each bin check shard ranges near
// Default cost 100, each shard next is - 1