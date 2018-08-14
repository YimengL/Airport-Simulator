//
// Created by Yimeng Li on 03/06/2018.
//

#include "airport.h"

Airport::~Airport() {
    for (auto& fut : future_arr) {
        fut.get();
    }
}

/*
 * @param rw: a Runway ptr
 * @param ps: a ParkingStand ptr
 * @return : if success or not
 */
bool Airport::reserve_landing_resource(std::shared_ptr<Runway> rw, std::shared_ptr<ParkingStand> ps) {
    if (std::try_lock(rw->r_lock, ps->p_lock) == -1) {
        if (ps->getState() == ParkingStandState::Available &&
            rw->getState() == RunwayState::Available) {
            rw->setState(RunwayState::Reserved);
            ps->setState(ParkingStandState::Reserved);
            return true;
        }
        rw->r_lock.unlock();
        ps->p_lock.unlock();
    }
    return false;
}

/*
 * @param rw: a Runway ptr
 * @param ps: a ParkingStand ptr
 * @return : if success or not
 */
bool Airport::reserve_takeoff_resource(std::shared_ptr<Runway> rw, std::shared_ptr<ParkingStand> ps) {
    if (try_lock(rw->r_lock, ps->p_lock) == -1) {
        if (ps->getState() == ParkingStandState::Occupied &&
            rw->getState() == RunwayState::Available) {
            rw->setState(RunwayState::Reserved);
            ps->setState(ParkingStandState::Available);
            return true;
        }
        rw->r_lock.unlock();
        ps->p_lock.unlock();
    }
    return false;
}

void Airport::add_runway(std::shared_ptr<Runway> runway) {
    runways.push_back(runway);
    runway_map.insert(make_pair(runway->getRunway_id(), runway));
}

void Airport::add_parking_stands(std::shared_ptr<ParkingStand> parking_stand) {
    parking_stands.push_back(parking_stand);
    parking_stand_map.insert(make_pair(parking_stand->getParking_id(), parking_stand));
}

LandingRequestToken Airport::request_landing(std::string aircraft_id) {
    for (auto& ps : parking_stands) {
        for (auto& rw : runways) {
            if (reserve_landing_resource(rw, ps)) {
                return LandingRequestToken(AirportState::Proceed, aircraft_id,
                                           rw->getRunway_id(), ps->getParking_id(), 4);
            }
        }
    }
    return LandingRequestToken(AirportState::Hold);
}

/*
 * @param aircraft_id: unique id of aircraft
 * @return : a token either Proceed or Hold
 */
TakeOffRequestToken Airport::request_takeoff(std::string aircraft_id) {
    auto got = parking_info.find(aircraft_id);
    if (got == parking_info.end()) {
        throw std::runtime_error("This airport does not have this plane");
    }
    std::shared_ptr<ParkingStand> ps = parking_info[aircraft_id];
    for (auto& rw : runways) {
        if (reserve_takeoff_resource(rw, ps)) {
            return TakeOffRequestToken(AirportState::Proceed, aircraft_id,
                                       rw->getRunway_id(), 4);
        }
    }
    return TakeOffRequestToken(AirportState::Hold);
}

bool Airport::perform_landing(LandingRequestToken token) {
    if (token.state != AirportState::Proceed) {
        // maybe impossible path, but keep it.
        throw std::runtime_error("Invalid Input");
    }
    std::shared_ptr<Runway> rw = runway_map[token.runway_id];
    std::shared_ptr<ParkingStand> ps = parking_stand_map[token.parking_stand_id];
    if (token.expiration < std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) {
        rw->r_lock.unlock();
        ps->p_lock.unlock();
        throw std::runtime_error("Token was expired");
    }

    rw->setState(RunwayState::InOperation);

    std::shared_future<void> fut = async(std::launch::async, [&token, rw, ps, this] () {
        std::this_thread::sleep_for(std::chrono::seconds(kOperationDurationSec));
        parking_info_lock.lock();
        parking_info.insert(make_pair(token.aircraft_id, ps));
        parking_info_lock.unlock();
        rw->setState(RunwayState::Available);
        ps->setState(ParkingStandState::Occupied);
        // Log("Aircraft ID: ", token.aircraft_id, ", Parking ID: ", token.parking_stand_id, '\n');
        rw->r_lock.unlock();
        ps->p_lock.unlock();
    });
    future_lock.lock();
    future_arr.push_back(fut);
    future_lock.unlock();
    return true;
}

bool Airport::perform_takeoff(TakeOffRequestToken token) {
    std::shared_ptr<Runway> rw = runway_map[token.runway_id];
    std::shared_ptr<ParkingStand> ps = parking_info[token.aircraft_id];
    if (token.expiration < std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) {
        rw->r_lock.unlock();
        ps->p_lock.unlock();
        throw std::runtime_error("Token was expired");
    }
    rw->setState(RunwayState::InOperation);

    std::shared_future<void> fut = async(std::launch::async, [&token, rw, ps, this] () {
        std::this_thread::sleep_for(std::chrono::seconds(kOperationDurationSec));
        parking_info_lock.lock();
        parking_info.erase(token.aircraft_id);
        parking_info_lock.unlock();
        rw->setState(RunwayState::Available);
        rw->r_lock.unlock();
        ps->setState(ParkingStandState::Available);
        ps->p_lock.unlock();
    });
    future_lock.lock();
    future_arr.push_back(fut);
    future_lock.unlock();
    return true;
}
