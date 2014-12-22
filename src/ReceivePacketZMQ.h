#pragma once
//#include "Zombie.h"
#include "Vampire.h"

/**
 *  * Defines a packet to be passed to afc_process function.
 *   */
typedef struct data_pkt {
	struct data_pkt* next;  
	void* user_handle;          
	void* buffer;              
	struct timeval timestamp;  
	unsigned int len;          
	unsigned char* data;        
} data_pkt_t;

typedef struct data_pkt* data_ppacket;

class ReceivePacketZMQ : public Vampire {
public:
   explicit ReceivePacketZMQ(const std::string& binding);
   bool Initialize();
   bool ReceiveDataBlock(std::string& dataReceived, const int nSecs);
   bool ReceiveDataBlockPointer(data_ppacket& dataReceived, const int milliseconds);
   void SetQueueSize(const int size);
protected:
};
