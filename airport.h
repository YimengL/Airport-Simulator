//
// Created by Yimeng Li on 03/06/2018.
//

#ifndef AIRPORTSIMULATOR_AIRPORT_H
#define AIRPORTSIMULATOR_AIRPORT_H

#include <vector>
#include <unordered_map>
#include <string>
#include <future>
#include <mutex>

#include "runway.h"
#include "parking_stand.h"
#include "tokens.h"

/** This can be used to initialize the airport simulation object operation duration parameter
 * with a value which is used by the testing code. */
static constexpr const int kOperationDurationSec = 5;

/** Simulation of an airport. Tiny preview of the headaches that come with the real thing. */
class Airport {
private:
    std::vector<std::shared_ptr<Runway>> runways;
    std::vector<std::shared_ptr<ParkingStand>> parking_stands;
    std::unordered_map<std::string, std::shared_ptr<Runway>> runway_map;
    std::unordered_map<std::string, std::shared_ptr<ParkingStand>> parking_stand_map;
    std::unordered_map<std::string, std::shared_ptr<ParkingStand>> parking_info;   // key: aircraft_id, value: ParkingStand ptr
    std::vector<std::shared_future<void>> future_arr;
    std::mutex future_lock;
    std::mutex parking_info_lock;

    bool reserve_landing_resource(std::shared_ptr<Runway> rw, std::shared_ptr<ParkingStand> ps);
    bool reserve_takeoff_resource(std::shared_ptr<Runway> rw, std::shared_ptr<ParkingStand> ps);

public:
    ~Airport();
    void add_runway(std::shared_ptr<Runway> runway);
    void add_parking_stands(std::shared_ptr<ParkingStand> parking_stand);
    LandingRequestToken request_landing(std::string aircraft_id);
    TakeOffRequestToken request_takeoff(std::string aircraft_id);
    bool perform_landing(LandingRequestToken token);
    bool perform_takeoff(TakeOffRequestToken token);
};

#endif //AIRPORTSIMULATOR_AIRPORT_H
