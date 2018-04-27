/* Not any company's property but Public-Domain
* Do with source-code as you will. No requirement to keep this
* header if need to use it/change it/ or do whatever with it
*
* Note that there is No guarantee that this code will work
* and I take no responsibility for this code and any problems you
* might get if using it.
*
* Modified from original test at:  https://github.com/KjellKod/Q
*/

#include "QAPI.h"
#include "QApiTests.h"
#include <q/spsc.hpp>
#include <q/mpmc.hpp>
#include <thread>
#include <algorithm>

using namespace QApiTests;

namespace {
   const size_t kAmount = 1000000;
   const size_t kSmallQueueSize = 100;
}


TEST(Queue, BaseAPI_Flexible) {
   auto queue = QAPI::CreateQueue<spsc::flexible::circular_fifo<std::string>>(kAmount);
   auto producer = std::get<QAPI::index::sender>(queue);
   auto consumer = std::get<QAPI::index::receiver>(queue);
   EXPECT_TRUE(producer.empty());
   EXPECT_FALSE(producer.full());
   EXPECT_EQ(kAmount, producer.capacity());
   EXPECT_EQ(kAmount, producer.capacity_free());
   EXPECT_EQ(0, producer.size());
   //EXPECT_TRUE(producer.lock_free()); See comment in QAPI.h
   EXPECT_EQ(0, producer.usage());
}

TEST(Queue, BaseAPI_Fixed) {
   auto queue = QAPI::CreateQueue<spsc::fixed::circular_fifo<std::string, kAmount>>();
   auto producer = std::get<QAPI::index::sender>(queue);
   auto consumer = std::get<QAPI::index::receiver>(queue);
   EXPECT_TRUE(producer.empty());
   EXPECT_FALSE(producer.full());
   EXPECT_EQ(kAmount, producer.capacity());
   EXPECT_EQ(kAmount, producer.capacity_free());
   EXPECT_EQ(0, producer.size());
   //EXPECT_TRUE(producer.lock_free()); See comment in QAPI.h
   EXPECT_EQ(0, producer.usage());
}


TEST(Queue, BaseAPI_Flexible_Locked) {
   auto queue = QAPI::CreateQueue<mpmc::flexible_lock_queue<std::string>>(kAmount);
   auto producer = std::get<QAPI::index::sender>(queue);
   auto consumer = std::get<QAPI::index::receiver>(queue);
   EXPECT_TRUE(producer.empty());
   EXPECT_FALSE(producer.full());
   EXPECT_EQ(kAmount, producer.capacity());
   EXPECT_EQ(kAmount, producer.capacity_free());
   EXPECT_EQ(0, producer.size());
   //EXPECT_FALSE(producer.lock_free());  // NOT lock -free
   EXPECT_EQ(0, producer.usage());
}



TEST(Performance, SPSC_Flexible_CircularFifo) {
   auto queue = QAPI::CreateQueue<spsc::flexible::circular_fifo<std::string>>(kAmount);
   RunSPSC(queue, kAmount);
}

TEST(Performance, SPSC_Flexible_CircularFifo_Smaller) {
   auto queue = QAPI::CreateQueue<spsc::flexible::circular_fifo<std::string>>(kSmallQueueSize);
   RunSPSC(queue, kAmount);
}


TEST(Performance, SPSC_Fixed_CircularFifo) {
   using namespace std;
   auto queue = QAPI::CreateQueue<spsc::fixed::circular_fifo<string, kAmount>>();
   RunSPSC(queue, kAmount);
}


TEST(Performance, SPSC_Fixed_CircularFifo_Smaller) {
   using namespace std;
   auto queue = QAPI::CreateQueue<spsc::fixed::circular_fifo<string, kSmallQueueSize>>();
   RunSPSC(queue, kAmount);
}


TEST(Performance, MPMC_1_to_1) {
   using namespace std;
   auto queue = QAPI::CreateQueue<mpmc::flexible_lock_queue<string>>(kAmount);
   RunSPSC(queue, kAmount);
}


TEST(Performance, MPMC_1_to_1_Smaller) {
   using namespace std;

   auto queue = QAPI::CreateQueue<mpmc::flexible_lock_queue<string>>(kSmallQueueSize);
   RunSPSC(queue, kAmount);
}


TEST(Performance, SPSC_Flexible_20secRun_LargeData) {
   using namespace std;
   auto queue = QAPI::CreateQueue<spsc::flexible::circular_fifo<std::string>>(kSmallQueueSize);
   const size_t large = 65554;
   std::string data(large, 'a');
   EXPECT_EQ(large, data.size());
   const size_t numberOfProducers = 1;
   const size_t numberOfConsumers = 1;
   const size_t kTimeToRunSec = 20;
   RunMPMC(queue, data, numberOfProducers, numberOfConsumers, kTimeToRunSec);
}

TEST(Performance, SPSC_Fixed_20secRun_LargeData) {
   using namespace std;
   auto queue = QAPI::CreateQueue<spsc::fixed::circular_fifo<std::string, kSmallQueueSize>>();
   const size_t large = 65554;
   std::string data(large, 'a');
   EXPECT_EQ(large, data.size());
   const size_t numberOfProducers = 1;
   const size_t numberOfConsumers = 1;
   const size_t kTimeToRunSec = 20;
   RunMPMC(queue, data, numberOfProducers, numberOfConsumers, kTimeToRunSec);
}


TEST(Performance, MPMC_1_to_4_20secRun_LargeData) {
   using namespace std;
   auto queue = QAPI::CreateQueue<mpmc::flexible_lock_queue<string>>(kSmallQueueSize);
   const size_t large = 65554;
   std::string data(large, 'a');
   EXPECT_EQ(large, data.size());
   const size_t numberOfProducers = 1;
   const size_t numberOfConsumers = 4;
   const size_t kTimeToRunSec = 20;
   RunMPMC(queue, data, numberOfProducers, numberOfConsumers, kTimeToRunSec);
}

TEST(Performance, MPMC_4_to_1_20secRun_LargeData) {
   using namespace std;
   auto queue = QAPI::CreateQueue<mpmc::flexible_lock_queue<string>>(kSmallQueueSize);
   const size_t large = 65554;
   std::string data(large, 'a');
   EXPECT_EQ(large, data.size());
   const size_t numberOfProducers = 4;
   const size_t numberOfConsumers = 1;
   const size_t kTimeToRunSec = 20;
   RunMPMC(queue, data, numberOfProducers, numberOfConsumers, kTimeToRunSec);
}

TEST(Performance, MPMC_4_to_4_20secRun_LargeData) {
   using namespace std;
   auto queue = QAPI::CreateQueue<mpmc::flexible_lock_queue<string>>(kSmallQueueSize);
   const size_t large = 65554;
   std::string data(large, 'a');
   EXPECT_EQ(large, data.size());
   const size_t numberOfProducers = 4;
   const size_t numberOfConsumers = 4;
   const size_t kTimeToRunSec = 20;
   RunMPMC(queue, data, numberOfProducers, numberOfConsumers, kTimeToRunSec);
}

