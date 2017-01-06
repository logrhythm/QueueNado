#include <nanomsg/nn.h>
#include <string>
#include <vector>
/*
 * class for encapsulating nanomsg zero copy allocation/deletion
 */
class NanoMsg {
   typedef const std::vector<std::pair<void*, unsigned int>> PacketHashVector;
public:
   NanoMsg(PacketHashVector& data);
   NanoMsg(void*& data);
   NanoMsg(const std::string& data);
   ~NanoMsg();
   NanoMsg& operator=(const NanoMsg&) = delete;
   NanoMsg(const NanoMsg&) = delete;
   NanoMsg() = delete;
   bool IsSent() {
      return mSent;
   }
   void SetSent() {
      mSent = true;
   }
   void** GetBufferReference() {
      return &mBuffer;
   }
private:
   void* mBuffer;
   bool mSent;
};
