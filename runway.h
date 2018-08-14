//
// Created by Yimeng Li on 03/06/2018.
//

#ifndef AIRPORTSIMULATOR_RUNWAY_H
#define AIRPORTSIMULATOR_RUNWAY_H

#include <string>
#include <mutex>

enum class RunwayState { InOperation, Reserved, Available };

/** A runway, essential for air travel. */
class Runway {
private:
    std::string runway_id;
    int length;
    RunwayState state;

public:
    std::mutex r_lock;
    Runway();
    Runway(int id);

    RunwayState getState() const { return state; }
    void setState(RunwayState state) { Runway::state = state; }
    const std::string &getRunway_id() const { return runway_id; }
};

#endif //AIRPORTSIMULATOR_RUNWAY_H
