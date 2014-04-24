#include <gtest/gtest.h>
#include <cstdlib>
#include <string>

class DiskPcapMoveTest : public ::testing::Test {
public:
   DiskPcapMoveTest() : mTestDirectory("/tmp/TestOfDiskPcapMover"){};
   virtual ~DiskPcapMoveTest() = default;
   
   
protected:
   virtual void SetUp() {
      std::string mkTestdir {"mkdir -p " + mTestDirectory};
      
      EXPECT_EQ(0, system(mkTestdir.c_str()));   
   }
   
   
   
   virtual void TearDown() {
      std::string removeDir{"rm -rf " + mTestDirectory};
      //EXPECT_EQ(0, system(removeDir.c_str()));
   }  
   
   
   virtual std::string CreateSubDirectory(const std::string& directory){
      std::string mkTestSubDir{"mkdir -p " + mTestDirectory + "/" + directory};
      if (0 != system(mkTestSubDir.c_str())) {
         return {};
      }      
      return {mTestDirectory + "/" + directory};
   }
   
   virtual std::string CreateFile(const std::string& directory, const std::string& filename){
      std::string mkTestSubDir{"touch " + directory + "/" + filename};
      if (0 != system(mkTestSubDir.c_str())) {
         return {};
      }
      return {directory + "/" + filename};
   }
   
   
   const std::string mTestDirectory;
   
 };