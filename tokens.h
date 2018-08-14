//
// Created by Yimeng Li on 03/06/2018.
//

#ifndef AIRPORTSIMULATOR_TOKENS_H
#define AIRPORTSIMULATOR_TOKENS_H

#include <string>
#include <chrono>

enum class AirportState { Hold, Proceed };

class LandingRequestToken {
public:
    AirportState state;
    std::string aircraft_id;
    std::string runway_id;
    std::string parking_stand_id;
    time_t expiration;

    LandingRequestToken() = default;

    LandingRequestToken(AirportState state) : state(state) {}

    LandingRequestToken(AirportState state, const std::string &aircraft_id,
                        const std::string &runway_id, const std::string &parking_stand_id, int secs) :
            state(state),
            aircraft_id(aircraft_id),
            runway_id(runway_id),
            parking_stand_id(parking_stand_id) {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        expiration = std::chrono::system_clock::to_time_t(now + std::chrono::seconds(secs));
    }
};

class TakeOffRequestToken {
public:
    AirportState state;
    std::string aircraft_id;
    std::string runway_id;
    time_t expiration;

    TakeOffRequestToken() = default;

    TakeOffRequestToken(AirportState state) : state(state) {}

    TakeOffRequestToken(AirportState state, const std::string &aircraft_id,
                        const std::string &runway_id, int secs) : state(state),
                                                             aircraft_id(aircraft_id),
                                                             runway_id(runway_id) {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        expiration = std::chrono::system_clock::to_time_t(now +  std::chrono::seconds(secs));
    }
};

#endif //AIRPORTSIMULATOR_TOKENS_H
