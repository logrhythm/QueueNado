#include "Alien.h"
#include "czmq.h"
#include "boost/thread.hpp"
#include "g2log.hpp"
#include <memory>

/**
 * Alien is a ZeroMQ Sub socket.
 */
Alien::Alien() {
    mCtx = zctx_new();
    assert(mCtx);
    mBody = zsocket_new(mCtx, ZMQ_SUB);
    assert(mBody);
}

/**
 * Setup the location to receive messages.
 * @param location
 */
void Alien::PrepareToBeShot(const std::string& location) {
    //Subscribe to everything
    char dummy = '\0';
    zsocket_set_subscribe(mBody, &dummy);
    zsocket_set_rcvhwm(mBody, 32 * 1024);
    int rc = zsocket_connect(mBody, location.c_str());
    if (rc == -1) {
        LOG(WARNING) << "location: " << location;
        LOG(WARNING) << "connect socket rc == " << rc;
        throw std::string("Failed to connect to socket");
    }

}

/**
 * Blocking call that returns when the alien has been shot.
 * @return 
 */
std::vector<std::string> Alien::GetShot() {
    std::vector<std::string> bullets;
    while (!zctx_interrupted && bullets.empty()) {
       GetShot(1000,bullets);
       boost::this_thread::interruption_point();
    }
    return std::move(bullets);
}

/**
 * Blocking call that returns when the alien has been shot.
 * @return 
 */
void Alien::GetShot(const unsigned int timeout, std::vector<std::string>& bullets) {

    if (zsocket_poll(mBody, timeout)) {
        zmsg_t* msg = zmsg_recv(mBody);
        if (msg) {
            zframe_t* data = zmsg_pop(msg);
            zframe_destroy(&data);
            int msgSize = zmsg_size(msg);
            for (int i = 0; i < msgSize; i++) {
                data = zmsg_pop(msg);
                std::string bullet;
                bullet.assign(reinterpret_cast<char*> (zframe_data(data)), zframe_size(data));
                bullets.push_back(bullet);
                zframe_destroy(&data);
            }
        } else {
            throw std::string("GetShot returned null");
        }
        zmsg_destroy(&msg);
    }

}

/**
 * Destroy the body and context of the alien.
 */
Alien::~Alien() {
    zsocket_destroy(mCtx, mBody);
    zctx_destroy(&mCtx);
}
