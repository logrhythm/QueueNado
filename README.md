QueueNado
=========

`QueueNado` is a collection of C++ implemntations of ZMQ fast queue patterns (Push/Pull, request/reply, pub/sub, dealer/router) for C++


## Requirements
1. [g3log](https://github.com/KjellKod/g3log)
2. [g3sinks](https://github.com/KjellKod/g3sinks)
3. [FileIO](https://github.com/LogRhythm/FileIO)
4. [ZMQ v3.2.5](http://zeromq.org/)
5. [ZLIB](http://www.zlib.net)
Most Linux distributions carry zlib. 
Example: In Ubuntu it can be installed with 
```sudo apt-get install zlib1g-dev``` 
6. [StopWatch](https://github.com/LogRhythm/StopWatch)
7. [DeathKnell](https://github.com/LogRhythm/DeathKnell)
8. [tcmalloc](https://github.com/gperftools/gperftools)
The creators of tcmalloc recommends installing ` libunwind`   to avoid encountering a  deadlock bug during stack unwinding. 
9. [boost](http://www.boost.org/doc/libs/1_63_0/index.html)
For installing boost please follow their [instructions](http://www.boost.org/doc/libs/1_63_0/more/getting_started/unix-variants.html). 


## Build and Install
Example of installation
```
cd Queuenado
cd 3rdparty
unzip gtest-1.7.0.zip
cd ..
mkdir build
cd build
cmake ..
make -j
```

### Executing the unit tests
```
sudo ./UnitTestRunneer

```

### Installing
```
sudo make install
```

Alternative on Debian systems
```
make package
sudo dpkg -i Queuenado-<package_version>Linux.deb
```


# Rifle - Vampire
`Rifle - Vampire` implements the [push / pull](http://zguide.zeromq.org/page:all#Divide-and-Conquer) messaging pattern in zmq. The communication setup supports the following client/server communications

**Client -to- Server**
* one-to-one
* one-to-many
* many-to-one

In a *many* scenario, it is the "one" side that does the ```bind``` ([[Vampire.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/Vampire.h) [[Rifle.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/Rifle.h)) on the queue.
#### Rifle - Vampire Limitation
It **cannot** do many-to-many

#### Use Cases for `Rifle - Vampire`
* Reliable messaging without responses
* High performance (500k msgs per second or higher) with zero_copy
* Process to process communication

#### API
[[Vampire.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/Vampire.h)
[[Rifle.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/Rifle.h)

#### Test usage
[[RifleVampireTests.cpp]](https://github.com/LogRhythm/QueueNado/blob/master/test/RifleVampireTests.cpp)

# Shotgun - Alien
`Shotgun - Alien` implements the [pub / sub](http://zguide.zeromq.org/page:all#Getting-the-Message-Out) messaging pattern in zmq.

#### Use Cases for `Shotgun - Alien`
* One to many: one sender communicating with many listeners.
* Not high performance around 10k msgs a sec. This can be improved by batching many messages together.
* Process to process communication
* All listeners receive every message sent.

#### Known limitations and issues
* [Slow joiner](http://zguide.zeromq.org/php:chapter5#Representing-State-as-Key-Value-Pairs) issues don't matter or can be worked around


#### API
[[Shotgun.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/Shotgun.h)
[[Alien.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/Alien.h)

#### Test usage
[[ShotgunAlienTests.cpp]](https://github.com/LogRhythm/QueueNado/blob/master/test/ShotgunAlienTests.cpp)


# Headcrab - Crowbar
`Headcrab - Crowbar` implements [request / reply](http://zguide.zeromq.org/page:all#Ask-and-Ye-Shall-Receive) messaging pattern in zmq.

#### Use Cases for `Headcrab - Crowbar` 
* Need to receive acknowledgement from the receiver for each message sent to it
* Know that their will always be a response (otherwise the socket will be broken)

#### Known limitations and issues
* Lower performance (60k msgs a sec)

[[Headcrab.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/Headcrab.h)
[[Crowbar.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/Crowbar.h)

#### Test usage
[[CrowbarHeadcrabTests.cpp]](https://github.com/LogRhythm/QueueNado/blob/master/test/CrowbarHeadcrabTests.cpp)


# Harpoon - Kraken
`Harpoon - Kraken` implements a streaming version of [pub / sub](http://zguide.zeromq.org/page:all#Getting-the-Message-Out). It enables  data streaming from a publisher to a subscriber. 


#### Kraken Purpose:
The publisher sending streaming data.
#### Usage example calls from the API: [[kraken.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/Kraken.h)

* `SendTidalWave()` : Send a data chunk to the subscriber ([[harpoon]](https://github.com/LogRhythm/QueueNado/blob/master/src/Harpoon.h)). The call blocks until there is space available in the queue. Returns `TIMEOUT`, `INTERRUPT`, `CONTINUE` status to indicate the status of the underlying queue.

* `FinalBreach()` : Call to subscriber ([[harpoon]](https://github.com/LogRhythm/QueueNado/blob/master/src/Harpoon.h)) to indicate the end of a stream.

#### Harpoon: Subscriber that receives the data
Usage example calls from the API:
* `Aim()` : Set location of the queue (tcp)
* `Heave()` : Request data and wait for the data to be returned. Returns `TIMEOUT`, `INTERRUPT`, `VICTORIOUS`, `CONTINUE` to indicate status of the stream. `VICTORIOUS` means that the stream has completed.

#### API
[[Kraken.h]] (https://github.com/LogRhythm/QueueNado/blob/master/src/Kraken.h)
[[KrakenBattle.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/KrakenBattle.h)
[[Harpoon.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/Harpoon.h)
[[HarpoonBattle.h]](https://github.com/LogRhythm/QueueNado/blob/master/src/HarpoonBattle.h)

#### Test usage
* [[KrakenBattleTest.cpp]](https://github.com/LogRhythm/QueueNado/blob/master/test/KrakenBattleTest.cpp)
* [[HarpoonKrakenTests.cpp]](https://github.com/LogRhythm/QueueNado/blob/master/test/HarpoonKrakenTests.cpp)
* [[KrakenIntegrationTest.cpp]](https://github.com/LogRhythm/QueueNado/blob/master/test/KrakenIntegrationTest.cpp)


# Notifier - Listener
`Notifier - Listener` extends the `Shotgun - Alien` implementation of the [pub / sub](http://zguide.zeromq.org/page:all#Getting-the-Message-Out) messaging pattern in zmq.

The `Notifier - Listener` classes are  wrappers around the `Shotgun - Alien`queueing framework. A `Notifier` is used to place a message onto a queue that is read from by multiple Listeners. It uses handshake communications, where each `Listener` must respond back to the `Notifier` that it received the message. A `Notification` is deemed successful only if every expected `Listener` responds to the `Notifier`. 

#### Use cases for Notifier/Listener
* One-to-many with handshake feedback
* Alerting multiple processes of an event, or a call to action

#### Limitations for `Notifier - Listener`
The same as for `Shotgun - Alien`

# Boomstick - Skeleton
The `Boomstick - Skeleton` is used for connecting to ElasticSearch over a [wrapper](https://github.com/LogRhythm/transport-zeromq). At the moment part of the pattern implementation is not open sourced and still proprietary. Until further notice it is not recommended to use the `BoomStick - Skeleton` classes. 




