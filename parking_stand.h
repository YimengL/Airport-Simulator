//
// Created by Yimeng Li on 03/06/2018.
//

#ifndef AIRPORTSIMULATOR_PARKING_STAND_H
#define AIRPORTSIMULATOR_PARKING_STAND_H

#include <string>
#include <mutex>

enum class ParkingStandState { Occupied, Reserved, Available };

/** Parking stand. Useful whether you're in a 747-800 or a station wagon. */
class ParkingStand {
private:
    std::string parking_id;
    ParkingStandState state;

public:
    std::mutex p_lock;
    ParkingStand();
    ParkingStand(int id);

    ParkingStandState getState() const { return state; }
    void setState(ParkingStandState state) { ParkingStand::state = state; }
    const std::string &getParking_id() const { return parking_id; }
};

#endif //AIRPORTSIMULATOR_PARKING_STAND_H
