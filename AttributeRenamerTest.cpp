#include "AttributeRenamerTest.h"
#include "MockAttributeRenamer.h"
#include <memory>

namespace {

   bool StringContains(const std::string& input, const std::string& pattern) {
      if (input.find(pattern) != std::string::npos) {
         return true;
      } else {
         std::cout << input << " does not contain " << pattern << std::endl;
      }
      return false;
   }
}
#ifdef LR_DEBUG

TEST_F(AttributeRenamerTest, ConstructAndInitialize) {
   MockAttributeRenamer renamer;
   EXPECT_FALSE(renamer.Initialize("/file/does/not/exist"));
   EXPECT_TRUE(renamer.Initialize("../conf/remapping.yaml"));
   EXPECT_TRUE(renamer.Initialize("../conf/remapping.yaml"));
   {
      std::unique_ptr<MockAttributeRenamer> renamerPtr(new MockAttributeRenamer);
      EXPECT_TRUE(renamerPtr->Initialize("../conf/remapping.yaml"));
   }
}

TEST_F(AttributeRenamerTest, BasicRenaming) {
   MockAttributeRenamer renamer;
   ASSERT_TRUE(renamer.Initialize("resources/remapping.yaml"));

   EXPECT_TRUE("logrhythm_ack_number" == renamer.GetNewName("ack_number", "#"));
   EXPECT_TRUE("#notFound" == renamer.GetNewName("notFound", "#"));
}

TEST_F(AttributeRenamerTest, ResetRenaming) {
   MockAttributeRenamer renamer;
   ASSERT_TRUE(renamer.Initialize("resources/remapping.yaml"));

   EXPECT_TRUE("logrhythm_ack_number" == renamer.GetNewName("ack_number", "#"));
   EXPECT_TRUE("#notFound" == renamer.GetNewName("notFound", "#"));
   ASSERT_FALSE(renamer.Initialize("/file/does/not/exist"));
   EXPECT_TRUE("#ack_number" == renamer.GetNewName("ack_number", "#"));
   EXPECT_TRUE("#notFound" == renamer.GetNewName("notFound", "#"));
}

TEST_F(AttributeRenamerTest, EmptyFields) {
   MockAttributeRenamer attributeRenamer;
   ASSERT_TRUE(attributeRenamer.Initialize("resources/remapping-empty.yaml"));
   EXPECT_TRUE("#ack_number" == attributeRenamer.GetNewName("ack_number", "#"));
   EXPECT_TRUE("#VTP_mgmt_domain" == attributeRenamer.GetNewName("VTP_mgmt_domain", "#"));

}

TEST_F(AttributeRenamerTest, CorruptYaml) {
   MockAttributeRenamer attributeRenamer;
   ASSERT_FALSE(attributeRenamer.Initialize("resources/remapping-corrupt.yaml"));
}

// This doesn't work, there are static fields that are exactly the names in dynamic.  We cannot rename static fields
//TEST_F(AttributeRenamerTest, IsStaticField) {
//   MockAttributeRenamer attributeRenamer;
//   ASSERT_TRUE(attributeRenamer.Initialize("resources/remapping.yaml"));
//   std::unordered_set<std::string> staticFields;
//   staticFields.insert("session_id");
//   staticFields.insert("mac_source");
//   staticFields.insert("mac_dest");
//   staticFields.insert("ip_source");
//   staticFields.insert("ip_dest");
//   staticFields.insert("pkt_path");
//   staticFields.insert("flow_session_count");
//   staticFields.insert("port_source");
//   staticFields.insert("port_dest");
//   staticFields.insert("flow_completed");
//   staticFields.insert("delay");
//   staticFields.insert("proto_id");
//   staticFields.insert("packet_total");
//   staticFields.insert("time_start");
//   staticFields.insert("time_updated");
//   staticFields.insert("bytes_dest");
//   staticFields.insert("bytes_source");
//   staticFields.insert("flow_type");
//   staticFields.insert("packets_delta");
//   staticFields.insert("time_delta");
//   staticFields.insert("bytes_dest_delta");
//   staticFields.insert("bytes_source_delta");
//   staticFields.insert("custom_application");
//   staticFields.insert("flow_state");
//   staticFields.insert("captured");
//   staticFields.insert("child_flow_number");
//   staticFields.insert("bytes_total");
//   staticFields.insert("bytes_total_delta");
//   staticFields.insert("application");
//   staticFields.insert("application_path");
//   staticFields.insert("duration");
//   staticFields.insert("message_size");
//   staticFields.insert("thread_id");
//   staticFields.insert("field_count");
//   staticFields.insert("debug_message");
//   staticFields.insert("application_id");
//   staticFields.insert("latest_update");
//   staticFields.insert("time_previous");
//   staticFields.insert("written");
//   staticFields.insert("capture_removed");
//   for (const auto& field : staticFields) {
//      EXPECT_TRUE(attributeRenamer.IsStaticField(field));
//      EXPECT_EQ(field,attributeRenamer.GetNewName(field,"dontchangethis"));
//   }
//   EXPECT_FALSE(attributeRenamer.IsStaticField("notMe"));
//   EXPECT_EQ("notMe",attributeRenamer.GetNewName("dontchangethisnotMe","dontchangethis"));
//}
#endif

TEST_F(AttributeRenamerTest, Singleton) {
   AttributeRenamer& attributeRenamer = AttributeRenamer::Instance();
   AttributeRenamer& attributeRenamer2 = AttributeRenamer::Instance();
   ASSERT_EQ(&attributeRenamer, &attributeRenamer2);
   ASSERT_TRUE(attributeRenamer.Initialize("../conf/remapping.yaml"));
   EXPECT_FALSE(attributeRenamer.Initialize("../conf/remapping.yaml"));
   EXPECT_FALSE(attributeRenamer2.Initialize("../conf/remapping.yaml"));
}

