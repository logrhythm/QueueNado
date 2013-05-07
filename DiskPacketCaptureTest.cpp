
#include "DiskPacketCaptureTest.h"
#include <thread>
#include <future>

TEST_F(DiskPacketCaptureTest, Construct) {
   DiskPacketCapture capture;
   DiskPacketCapture* pCapture = new DiskPacketCapture;
   delete pCapture;
}

TEST_F(DiskPacketCaptureTest, GetRunningPackets) {
   MockDiskPacketCapture capture;

   ASSERT_EQ(capture.GetRunningPackets("abc123"), capture.GetRunningPackets("abc123"));
   ASSERT_NE(capture.GetRunningPackets("abc123"), capture.GetRunningPackets("123abc"));

   //A promise object that can be used for sharing a variable between threads
   std::promise<std::vector<std::pair<struct pcap_pkthdr*, uint8_t*> >* > promisedPacket;
   
   // A future variable that reads from the promise, allows you to wait on the other thread
   std::future<std::vector<std::pair<struct pcap_pkthdr*, uint8_t*> >*> futurePacket = promisedPacket.get_future();
   
   // A thread that does a call to the capture object, stores the result in the promise.
   //
   //  This uses what is refered to as a lambda function ( expressed as [] ).  This is a full function
   // Defined as an L-Value of call.  These are broken down as follows:
   // [] ( Argument definitions ) { Normal C++ code }
   //
   // The remaining arguments of the thread constructor are the list of arguments.  using std::ref you allow
   // the arguments to be references rather than pointers in this context.  
   //
   // Finally the .detach() breaks the thread off to execute in the background
   std::thread([](std::promise<std::vector<std::pair<struct pcap_pkthdr*, uint8_t*> >* >& pointer,
           MockDiskPacketCapture & capture) {
      pointer.set_value(capture.GetRunningPackets("abc123")); }, std::ref(promisedPacket), std::ref(capture)).detach();
      
   // Wait for the thread to set the promise object to a new value
   futurePacket.wait();
   
   // Verify that the code run in the thread gives you a different pointer 
   ASSERT_NE(capture.GetRunningPackets("abc123"), futurePacket.get());
}