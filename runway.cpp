//
// Created by Yimeng Li on 03/06/2018.
//

#include "runway.h"

Runway::Runway() {
    state = RunwayState::Available;
}

Runway::Runway(int id) {
    runway_id = "r_" + std::to_string(id);
    state = RunwayState::Available;
}