#include <iostream>
#include <ctime>
#include <cassert>

#include "parking_stand.h"
#include "runway.h"
#include "airport.h"

using namespace std;
using namespace std::chrono;

/**
 * @defgroup Boilerplate
 * Basic support for logging and random sleeps. Nothing exciting to see here.
 * @{ */

/** Some simple thread-safe logging functionality. */
void Log() {}

static recursive_mutex s_mutex;

template<typename First, typename ...Rest>
void Log(First&& first, Rest&& ...rest) {
    lock_guard<recursive_mutex> lock(s_mutex);
    std::cout << std::forward<First>(first);
    Log(std::forward<Rest>(rest)...);
}

/** Produces a random integer within the [1..max] range. */
static int RandomInt(int maxSleep = 6) {
    const int minSleep = 1;
    return minSleep + (rand() % static_cast<int>(maxSleep - minSleep + 1));
}

/* Declaration of testing function */
void test_request_landing_success(vector<string>);
void test_request_landing_fail1(vector<string>);
void test_request_landing_fail2(vector<string>);
void test_perform_landing_success(vector<string>);
void test_perform_landing_exception(vector<string>);
void test_perform_landing_async(vector<string>);
void test_request_takeoff_success(vector<string>);
void test_request_takeoff_exception(vector<string>);
void test_perform_takeoff_success(vector<string>);
void test_perform_takeoff_exception(vector<string>);
void test_request_landing_race1(int t);
void test_request_landing_race2(int t);

/* test by EYE(log), not by assertion */
void test_perform_landing_race(int thr, int num_rw, int num_ps);
void test_perform_takeoff_race(int thr, int num_rw, int num_ps);

int main() {
    vector<string> planes;
    for (int i = 0; i < 2; ++i) {
        string id {"Aircraft "};
        id.append(to_string(i));
        planes.push_back(id);
    }

    test_request_landing_success(planes);
    test_request_landing_fail1(planes);
    test_request_landing_fail2(planes);
    test_perform_landing_success(planes);
    test_perform_landing_exception(planes);
    test_perform_landing_async(planes);
    test_request_takeoff_success(planes);
    test_request_takeoff_exception(planes);
    test_perform_takeoff_success(planes);
    test_perform_takeoff_exception(planes);
    for (int t = 1; t <= 20; ++t) {
        test_request_landing_race1(t);
    }
    for (int t = 1; t <= 20; ++t) {
        test_request_landing_race2(t);
    }

    auto start1 = system_clock::now();
    test_perform_landing_race(8, 3, 8);
    auto end1 = system_clock::now();
    duration<double> running_time1 = end1 - start1;
    Log("Elapsed time: ", running_time1.count(), "s\n");

    auto start2 = system_clock::now();
    test_perform_takeoff_race(8, 3, 8);
    auto end2 = system_clock::now();
    chrono::duration<double> running_time2 = end2 - start2;
    Log("Elapsed time: ", running_time2.count(), "s\n");

    return 0;
}

/* Implementation of testing functions */

void test_request_landing_success(vector<string> planes) {
    Airport airport {};
    shared_ptr<Runway> rw = make_shared<Runway>(0);
    assert(rw->getState() == RunwayState::Available);

    airport.add_runway(rw);
    LandingRequestToken token1 = airport.request_landing(planes.at(0));
    assert(token1.state == AirportState::Hold);

    shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(0);
    assert(ps->getState() == ParkingStandState::Available);

    airport.add_parking_stands(ps);
    LandingRequestToken token2 = airport.request_landing(planes.at(0));
    assert(token2.state == AirportState::Proceed);
    Log("[PASS]test_request_landing_success\n");
}

void test_perform_landing_exception(vector<string> planes) {
    Airport airport {};
    shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(0);
    shared_ptr<Runway> rw = make_shared<Runway>(0);
    airport.add_runway(rw);
    airport.add_parking_stands(ps);

    LandingRequestToken token1 = airport.request_landing(planes.at(0));
    this_thread::sleep_for(seconds(6));             // wait for taken expire
    string msg = "";
    try {
        airport.perform_landing(token1);
    }
    catch (runtime_error& e) {
        msg.append(static_cast<string>(e.what()));
    }
    assert(msg == "Token was expired");
    this_thread::sleep_for(seconds(kOperationDurationSec + 1));
    assert(ps->getState() == ParkingStandState::Reserved);
    assert(rw->getState() == RunwayState::Reserved);
    Log("[PASS]test_perform_landing_exception\n");
}

void test_request_landing_fail1(vector<string> planes) {
    Airport airport {};
    shared_ptr<Runway> rw = make_shared<Runway>(0);
    rw->setState(RunwayState::Reserved);
    assert(rw->getState() != RunwayState::Available);

    shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(0);
    assert(ps->getState() == ParkingStandState::Available);

    airport.add_runway(rw);
    airport.add_parking_stands(ps);
    LandingRequestToken token1 = airport.request_landing(planes.at(0));
    assert(token1.state == AirportState::Hold);
    Log("[PASS]test_request_landing_fail1\n");
}

void test_request_landing_fail2(vector<string> planes) {
    Airport airport {};
    shared_ptr<Runway> rw = make_shared<Runway>(0);
    assert(rw->getState() == RunwayState::Available);

    shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(0);
    ps->setState(ParkingStandState::Occupied);
    assert(ps->getState() != ParkingStandState::Available);

    airport.add_runway(rw);
    airport.add_parking_stands(ps);
    LandingRequestToken token1 = airport.request_landing(planes.at(0));
    assert(token1.state == AirportState::Hold);
    Log("[PASS]test_request_landing_fail2\n");
}

void test_perform_landing_success(vector<string> planes) {
    Airport airport {};
    shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(0);
    shared_ptr<Runway> rw = make_shared<Runway>(0);
    airport.add_runway(rw);
    airport.add_parking_stands(ps);

    LandingRequestToken token1 = airport.request_landing(planes.at(0));
    assert(token1.state == AirportState::Proceed);
    assert(ps->getState() == ParkingStandState::Reserved);
    assert(rw->getState() == RunwayState::Reserved);

    airport.perform_landing(token1);
    this_thread::sleep_for(seconds(6)); // max sleep time
    assert(ps->getState() == ParkingStandState::Occupied);
    assert(rw->getState() == RunwayState::Available);
    Log("[PASS]test_perform_landing_success\n");
}

void test_perform_landing_async(vector<string> planes) {
    Airport airport {};
    shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(0);
    shared_ptr<Runway> rw = make_shared<Runway>(0);
    airport.add_runway(rw);
    airport.add_parking_stands(ps);

    LandingRequestToken token1 = airport.request_landing(planes.at(0));
    assert(token1.state == AirportState::Proceed);
    assert(ps->getState() == ParkingStandState::Reserved);
    assert(rw->getState() == RunwayState::Reserved);

    airport.perform_landing(token1);
    assert(rw->getState() == RunwayState::InOperation);
    assert(ps->getState() == ParkingStandState::Reserved);
    Log("[PASS]test_perform_landing_async\n");
}

void test_request_takeoff_success(vector<string> planes) {
    Airport airport {};
    shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(0);
    shared_ptr<Runway> rw = make_shared<Runway>(0);
    airport.add_runway(rw);
    airport.add_parking_stands(ps);

    LandingRequestToken token1 = airport.request_landing(planes.at(0));
    airport.perform_landing(token1);
    this_thread::sleep_for(seconds(6));
    assert(ps->getState() == ParkingStandState::Occupied);
    assert(rw->getState() == RunwayState::Available);

    TakeOffRequestToken token2 = airport.request_takeoff(planes.at(0));
    assert(ps->getState() == ParkingStandState::Available);
    assert(rw->getState() == RunwayState::Reserved);
    assert(token2.state == AirportState::Proceed);
    Log("[PASS]test_request_takeoff_success\n");
}

/*
 * Test Case: If planes is not landing, request_takeoff call will fail(throw exception)
 */
void test_request_takeoff_exception(vector<string> planes) {
    Airport airport {};
    shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(0);
    shared_ptr<Runway> rw = make_shared<Runway>(0);
    airport.add_runway(rw);
    airport.add_parking_stands(ps);

    LandingRequestToken token1 = airport.request_landing(planes.at(0));
    string msg = "";
    try {
        TakeOffRequestToken token2 = airport.request_takeoff(planes.at(0));
    }
    catch (runtime_error& e) {
        msg.append(static_cast<string>(e.what()));
    }
    assert(msg == "This airport does not have this plane");
    assert(ps->getState() == ParkingStandState::Reserved);
    assert(rw->getState() == RunwayState::Reserved);
    Log("[PASS]test_request_takeoff_exception\n");
}

void test_perform_takeoff_success(vector<string> planes) {
    Airport airport {};
    shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(0);
    shared_ptr<Runway> rw = make_shared<Runway>(0);
    airport.add_runway(rw);
    airport.add_parking_stands(ps);

    LandingRequestToken token1 = airport.request_landing(planes.at(0));
    airport.perform_landing(token1);
    this_thread::sleep_for(seconds(kOperationDurationSec + 1));
    TakeOffRequestToken token2 = airport.request_takeoff(planes.at(0));
    airport.perform_takeoff(token2);
    assert(ps->getState() == ParkingStandState::Available);
    assert(rw->getState() == RunwayState::InOperation);
    this_thread::sleep_for(seconds(kOperationDurationSec + 1));
    assert(rw->getState() == RunwayState::Available);
    Log("[PASS]test_perform_takeoff_success\n");
}

void test_perform_takeoff_exception(vector<string> planes) {
    Airport airport {};
    shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(0);
    shared_ptr<Runway> rw = make_shared<Runway>(0);
    airport.add_runway(rw);
    airport.add_parking_stands(ps);

    LandingRequestToken token1 = airport.request_landing(planes.at(0));
    airport.perform_landing(token1);
    this_thread::sleep_for(seconds(kOperationDurationSec + 1));
    TakeOffRequestToken token2 = airport.request_takeoff(planes.at(0));
    assert(token1.state == AirportState::Proceed);
    this_thread::sleep_for(seconds(kOperationDurationSec + 1));  // wait for token expire
    string msg = "";
    try {
        airport.perform_takeoff(token2);
    }
    catch (runtime_error& e) {
        msg.append(static_cast<string>(e.what()));
    }
    assert(msg == "Token was expired");
    Log("[PASS]test_perform_takeoff_exception\n");
}

void test_request_landing_race1(int t) {
    Airport airport{};
    vector<shared_ptr<thread>> aircrafts;
    LandingRequestToken tokens[2];
    shared_ptr<Runway> rw = make_shared<Runway>(0);
    airport.add_runway(rw);

    shared_ptr<ParkingStand> ps1 = make_shared<ParkingStand>(0);
    airport.add_parking_stands(ps1);
    shared_ptr<ParkingStand> ps2 = make_shared<ParkingStand>(1);
    airport.add_parking_stands(ps2);

    for (int i = 0; i < 2; i++) {
        aircrafts.push_back(make_shared<thread>([&airport, i, &tokens]() {
            string id{"Aircraft "};
            id.append(to_string(i));
            for (int j = 0; j < 10; ++j) {
                // do not iterate to many times
                LandingRequestToken token1 = airport.request_landing(id);
                tokens[i] = token1;
                if (token1.state == AirportState::Proceed) {
                    break;
                }
            }
        }));
    }
    for (auto aircraft : aircrafts) {
        aircraft->join();
    }
    assert(tokens[0].state != tokens[1].state);
    Log("[PASS]test_request_landing_race1: ", t, '\n');
}

void test_request_landing_race2(int t) {
    // check deadlock & livelock
    Airport airport{};
    vector<shared_ptr<thread>> aircrafts;
    LandingRequestToken tokens[2];
    shared_ptr<Runway> rw1 = make_shared<Runway>(0);
    airport.add_runway(rw1);
    shared_ptr<Runway> rw2 = make_shared<Runway>(1);
    airport.add_runway(rw2);

    shared_ptr<ParkingStand> ps1 = make_shared<ParkingStand>(0);
    airport.add_parking_stands(ps1);
    shared_ptr<ParkingStand> ps2 = make_shared<ParkingStand>(1);
    airport.add_parking_stands(ps2);

    for (int i = 0; i < 2; i++) {
        aircrafts.push_back(make_shared<thread>([&airport, i, &tokens]() {
            string id{"Aircraft "};
            id.append(to_string(i));
            for (int j = 0; j < 10; ++j) {
                // do not iterate to many times
                LandingRequestToken token1 = airport.request_landing(id);
                tokens[i] = token1;
                if (token1.state == AirportState::Proceed) {
                    break;
                }
                this_thread::sleep_for(seconds(1));
            }
        }));
    }
    for (auto aircraft : aircrafts) {
        aircraft->join();
    }
    assert(tokens[0].state == AirportState::Proceed);
    assert(tokens[1].state == AirportState::Proceed);
    Log("[PASS]test_request_landing_race2: ", t, '\n');
}

void test_perform_landing_race(int thr, int num_rw, int num_ps) {
    Log("***************** [START] test_perform_landing_race ***********************\n\n");
    Airport airport{};
    vector<shared_ptr<thread>> aircrafts;
    for (int i = 0; i < num_rw; ++i) {
        shared_ptr<Runway> rw = make_shared<Runway>(i);
        airport.add_runway(rw);
    }
    for (int i = 0; i < num_ps; ++i) {
        shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(i);
        airport.add_parking_stands(ps);
    }

    for (int i = 0; i < thr; i++) {
        aircrafts.push_back(make_shared<thread>([&airport, i]() {
            string id{"Aircraft "};
            id.append(to_string(i));
            for (int j = 0; j < 100; ++j) {
                // do not iterate to many times
                LandingRequestToken token1 = airport.request_landing(id);
                // Log(id, " request\n");
                if (token1.state == AirportState::Proceed) {
                    // Log(id, " received a landing token.\n");
                }
                else {
                    this_thread::sleep_for(seconds(1));
                    continue;
                }
                // Wait some random time. In some cases the token will expire, that's OK.
                this_thread::sleep_for(seconds{RandomInt(1)});

                if (airport.perform_landing(token1)) {
                    Log(id, " landing success.\n");
                    this_thread::sleep_for(seconds(6)); // max sleep time
                    break;
                }
            }
        }));
    }
    for (auto aircraft : aircrafts) {
        aircraft->join();
    }
    Log("***************** [END] test_perform_landing_race ***********************\n\n");
}

void test_perform_takeoff_race(int thr, int num_rw, int num_ps) {
    Log("***************** [START] test_perform_takeoff_race ***********************\n\n");
    Airport airport {};
    vector<shared_ptr<thread>> aircrafts;
    for (int i = 0; i < num_rw; ++i) {
        shared_ptr<Runway> rw = make_shared<Runway>(i);
        airport.add_runway(rw);
    }
    for (int i = 0; i < num_ps; ++i) {
        shared_ptr<ParkingStand> ps = make_shared<ParkingStand>(i);
        airport.add_parking_stands(ps);
    }

    for (int i = 0; i < thr; i++) {
        aircrafts.push_back(make_shared<thread>([&airport, i]() {
            string id{"Aircraft "};
            id.append(to_string(i));
            for (int j = 0; j < 100; ++j) {
                // do not iterate to many times

                LandingRequestToken token1 = airport.request_landing(id);
                if (token1.state == AirportState::Proceed) {
                    // Log(id, " received a landing token.\n");
                }
                else {
                    this_thread::sleep_for(seconds(1));
                    continue;
                }

                // Wait some random time. In some cases the token will expire, that's OK.
                this_thread::sleep_for(seconds{RandomInt(1)});
                if (airport.perform_landing(token1)) {
                    Log(id, " landing success.\n");
                    this_thread::sleep_for(seconds(6)); // max sleep time
                    break;
                }
            }
            this_thread::sleep_for(seconds(6));

            for (int j = 0; j < 100; ++j) {
                // do not iterate to many times
                TakeOffRequestToken token1 = airport.request_takeoff(id);

                // Log(id, " request takeoff\n");
                if (token1.state == AirportState::Proceed) {
                    // Log(id, " received a takeoff token.\n");
                }
                else {
                    this_thread::sleep_for(seconds(1));
                    continue;
                }

                // Wait some random time. In some cases the token will expire, that's OK.
                this_thread::sleep_for(seconds{RandomInt(1)});
                if (airport.perform_takeoff(token1)) {
                    Log(id, " takeoff success.\n");
                    this_thread::sleep_for(seconds(6)); // max sleep time
                    break;
                }
            }
        }));
    }
    for (auto aircraft : aircrafts) {
        aircraft->join();
    }
    Log("***************** [END] test_perform_takeoff_race ***********************\n\n");
}