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
      EXPECT_EQ(0, system(removeDir.c_str()));
   }
   
   
   virtual bool CreateSubDirectory(const std::string& directory){
      std::string mkTestSubDir{"mkdir -p " + mTestDirectory + "/" + directory};
      return (0 == system(mkTestSubDir.c_str()));      
   }
   
   virtual bool CreateFile(const std::string& directory, const std::string& filename){
      std::string mkTestSubDir{"touch " + directory + "/" + filename};
      return (0 == system(mkTestSubDir.c_str()));    
   }
   
   
   const std::string mTestDirectory;
   
 };