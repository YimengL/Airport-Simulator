//
// Created by Yimeng Li on 03/06/2018.
//

#include "parking_stand.h"

ParkingStand::ParkingStand() {
    state = ParkingStandState::Available;
}

ParkingStand::ParkingStand(int id) {
    parking_id = "p_" + std::to_string(id);
    state = ParkingStandState::Available;
}
