#include <iostream>
#include <string>
#include "mpi.h"
#include <chrono>
#include <string>
#include <g3log/logworker.hpp>
#include <g3log/g3log.hpp>
#include <g3log/std2_make_unique.hpp>
#include <g3sinks/LogRotate.h>
#include "../src/Rifle.h"
#include "../src/Vampire.h"
#include <functional>
#include <cstring>
int process_rank = 0;
void print_benchmark_summary(const std::string& caller,
                             int packet_size,
                             long num_packets,
                             float total_ms,
                             long total_bytes){
    float Mbps = packet_size * num_packets / total_ms / 125000 * 1000;
    float transactions_per_sec = num_packets / total_ms * 1000;
    LOG(INFO) << "process rank " << process_rank
              << " stats for caller: " << caller << std::endl
              << "total ms: " << total_ms << std::endl
              << " Mbps: " << Mbps << std::endl
              << " transactions_per_sec: " << transactions_per_sec << std::endl
              << " packet size: " << packet_size << std::endl
              << " num packets: " << num_packets << std::endl
              << " MBytes transferred: " << (float)total_bytes / 1000 / 1000
              << std::endl;
}
void TestDeleteString(void*, void* data) {
    std::string* theString = reinterpret_cast<std::string*> (data);
    delete theString;
}
void pull_receive(Vampire& puller,
                  long num_pkts,
                  int pkt_size,
                  const std::string& url){
   LOG(INFO) << "process rank " << process_rank
             << " about to receive " << num_pkts
             << " messages from url "  << url;
   std::string received;
   long total_bytes = 0;
#ifdef OLD_HAT
   puller.PrepareToBeShot();
   puller.SetOwnSocket(true);
#endif
   auto start_time = std::chrono::high_resolution_clock::now();
   for(int i = 0; i < num_pkts; i++){
#ifdef OLD_HAT
      puller.GetShot(received, 60);
#else
      received = {puller.GetShot()};
#endif
      // LOG(INFO) << "process rank " << process_rank
      //           << " received data: " << received;
      total_bytes += received.size() * sizeof(char);
   }
   auto end_time = std::chrono::high_resolution_clock::now();
   float total_ms = std::chrono::duration_cast
      <std::chrono::milliseconds>(end_time - start_time).count();
    print_benchmark_summary("nano receiver", pkt_size, num_pkts, total_ms, total_bytes);
}
void push_send(Rifle& pusher,
               long num_pkts,
               int pkt_size,
               const std::string& example_data,
               const std::string& url){
   LOG(INFO) << "process rank " << process_rank
             << " about to send " << num_pkts
             << " messages to url "  << url;
#ifdef OLD_HAT
   pusher.Aim();
#endif
   auto start_time = std::chrono::high_resolution_clock::now();
   long total_bytes = 0;
   for(int i = 0; i < num_pkts; i++){
#ifdef OLD_HAT
      std::string* test_data = new std::string(example_data);
      total_bytes += test_data->size() * sizeof(char);
      pusher.FireZeroCopy(test_data, test_data->size(), TestDeleteString, 10000);
#else
      total_bytes += example_data.size() * sizeof(char);
      pusher.Fire(example_data);
#endif
   }
   auto end_time = std::chrono::high_resolution_clock::now();
   float total_ms = std::chrono::duration_cast
      <std::chrono::milliseconds>(end_time - start_time).count();
   print_benchmark_summary("nano sender", pkt_size, num_pkts, total_ms, total_bytes);
}
bool enough_procs_for_mode(const std::string& mode, int num_procs){
   std::function<bool(int)> test = [](int procs)->bool{
      return true;
   };
   std::map<std::string, std::function<bool(int)>> mode_minimum_procs = {
      {"one_to_one", [](int procs)->bool{
            auto result =  procs == 2;
            if (!result)
               LOG(INFO) << "one to one can only happen between two processes";
            return result;
         }},
      {"one_to_many", [](int procs)->bool{
            auto result = procs >= 3;
            if (!result)
               LOG(INFO) << "one to many needs at least three processes";
            return result;
         }},
      {"many_to_many", [](int procs)->bool{
            auto result = procs >= 4 && procs % 2 == 0;
            if (!result)
               LOG(INFO) << "many to many requires at least 4 processes"
                         << ", and an even number of processes";
            return result;
         }},
      {"many_to_one", [](int procs)->bool{
            auto result = procs >= 3;
            if (!result)
               LOG(INFO) << "many to one requires at least three processes";
            return result;
         }}
   };
   return mode_minimum_procs[mode](num_procs);
}
bool check_run_mode(const std::string& run_mode){
   std::vector<std::string> available_run_modes = {"one_to_one",
                                                   "one_to_many",
                                                   "many_to_many",
                                                   "many_to_one"};
   if (std::none_of(available_run_modes.begin(), available_run_modes.end(),
                    [&](std::string& mode){
                       return run_mode == mode;
                    })){
      LOG(INFO) << "invalid run mode specified";
      return false;
   }
   return true;
}
bool check_enough_procs(const std::string& run_mode,
                        int world_size,
                        int& num_senders,
                        int& num_receivers,
                        bool& is_sender){
   if (enough_procs_for_mode(run_mode, world_size)){
      if (run_mode == "one_to_one" || run_mode == "many_to_many"){
         // split senders are receivers evenly
         if (process_rank%2 != 0){
            //receivers are always odd numbered
            is_sender = false;
         }
         num_receivers = world_size / 2;
         num_senders = world_size / 2;
      }
      else if (run_mode == "one_to_many"){
         if (process_rank != 0){
            //every process except rank 0 is a receiver
            is_sender = false;
         }
         num_receivers = world_size - 1;
         num_senders = 1;
      }
      else if (run_mode == "many_to_one"){
         if (process_rank == 0){
            //pin the one receiver
            is_sender = false;
         }
         num_receivers = 1;
         num_senders = world_size - 1;
      }
      return true;
   } else{
      return false;
   }
}
// #define SOCKET_ADDRESS "ipc:///tmp/nano.ipc"
// void *gen_msg(const std::string& data){
//    LOG(INFO) << "generating msg for: " << data;
//    void *buffer = nn_allocmsg(data.size(), 0);
//    auto error = nn_errno ();
//    if (error == ENOMEM) {
//       throw std::runtime_error("not enough memory to allocate NanoMsg");
//    }
//    std::memcpy(buffer, data.c_str(), data.size());
//    return buffer;
// }
// std::string test_recv(int socket, const std::string& expected){
//    void * buf;
//    auto num_bytes_received = nn_recv(socket, &buf, NN_MSG, 0);
//    std::string nanoString(static_cast<char*>(buf), num_bytes_received);
//    LOG(INFO) << "received string: " << nanoString;
//    nn_freemsg(buf);
//    return nanoString;
// }
// void test_send(int socket, const std::string& data){
//    void * generated_msg = gen_msg(data);
//    LOG(INFO) << "sending buffer: " << generated_msg;
//    auto num_bytes_sent = nn_send(socket,
//                                  &generated_msg,
//                                  NN_MSG,
//                                  0);
// }
int main(int argc, char*argv[]){
   auto uniqueLoggerPtr = g3::LogWorker::createLogWorker();
   auto handle = uniqueLoggerPtr->addSink(std2::make_unique<LogRotate>("queuenado_process_benchmark", "/tmp/"), &LogRotate::save);

   g3::initializeLogging(uniqueLoggerPtr.get());
   MPI::Init();
   std::vector<std::string> params(argv, argv+argc);
   auto run_mode = params[1];
   auto num_packets_to_send = std::stoi(params[2]);
   auto data_size = std::stoi(params[3]);
   auto ipc_mechanism = params[4];
   std::string url;
   if (ipc_mechanism == "ipc"){
      url = "ipc:///tmp/nano.ipc";
   } else if (ipc_mechanism == "tcp"){
      url = "tcp://127.0.0.1:5555";
   } else{
      // invalid transport mechanism selected
      LOG(WARNING) << "unsupported transport mechanism: " << ipc_mechanism;
      MPI::Finalize();
      return 1;
   }
   // run modes control the rifle to vampire ratio, respectively
   if (!check_run_mode(run_mode)){
      MPI::Finalize();
      return 1;
   }
   // we have a run mode, start being smart about dividing up the work
   std::string example_data(data_size, 'z');
   auto packet_size = example_data.size() * sizeof(char);
   process_rank = MPI::COMM_WORLD.Get_rank();
   auto world_size = MPI::COMM_WORLD.Get_size();
   auto is_sender = true;
   int num_senders = 0;
   int num_receivers = 0;
   if (!check_enough_procs(run_mode, world_size, num_senders, num_receivers, is_sender)){
      LOG(WARNING) << "invalid proc configuration";
      MPI::Finalize();
      return 1;
   }
   if (num_packets_to_send % num_senders != 0){
      LOG(WARNING) << "num_senders: " << num_senders
                   << " does not divide " << num_packets_to_send
                   << " packets evenly";
      MPI::Finalize();
      return 1;
   }
   if (num_packets_to_send % num_receivers != 0){
      LOG(WARNING) << "num_receivers: " << num_receivers
                   << " does not divide " << num_packets_to_send
                   << " packets evenly";
      MPI::Finalize();
      return 1;
   }
   long num_send_packets = num_packets_to_send / num_senders;
   long num_receive_packets = num_packets_to_send / num_receivers;
   LOG(INFO) << "num senders: " << num_senders << "num receivers: " << num_receivers;
   if (num_send_packets * num_senders == num_receive_packets * num_receivers){
      // we will receive all packets that we send, good to go!
      if (is_sender){
         Rifle sender(url);
         std::this_thread::sleep_for(std::chrono::milliseconds(500));
         push_send(sender, num_send_packets, packet_size, example_data, url);
         MPI::Finalize();
      } else{
         Vampire receiver(url);
         std::this_thread::sleep_for(std::chrono::milliseconds(50));
         pull_receive(receiver, num_receive_packets, packet_size, url);
         MPI::Finalize();
      }
   }
   return 0;
}
