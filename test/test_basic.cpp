// #include <gtest/gtest.h>
// #include "main.cpp" // Include the main implementation

// class ClubTest : public ::testing::Test {
// protected:
//     void SetUp() override {
//         // Common setup for all tests
//         club.numTables = 3;
//         club.tables.resize(club.numTables);
//         club.timeBegin = minutesAfterMidnight("09:00");
//         club.timeEnd = minutesAfterMidnight("19:00");
//         club.costPerHour = 10;
//     }

//     Club club;
// };

// TEST_F(ClubTest, TestTimeConversion) {
//     auto time = minutesAfterMidnight("09:00");
//     EXPECT_EQ(time.count(), 540);
    
//     time = minutesAfterMidnight("23:59");
//     EXPECT_EQ(time.count(), 1439);
    
//     time = minutesAfterMidnight("00:00");
//     EXPECT_EQ(time.count(), 0);
// }

// TEST_F(ClubTest, TestClientEnterValid) {
//     Event event;
//     event.timestamp = minutesAfterMidnight("09:30");
//     event.eventID = EventID::inClientEnter;
//     event.client = "client1";
    
//     club.processEventIn(event);
//     EXPECT_EQ(club.clients.size(), 1);
//     EXPECT_EQ(club.eventsOut.size(), 1); // Only the input event is added
// }

// TEST_F(ClubClientEnterTest, TestClientEnterOutsideWorkingHours) {
//     Event event;
//     event.timestamp = minutesAfterMidnight("08:30");
//     event.eventID = EventID::inClientEnter;
//     event.client = "client1";
    
//     club.processEventIn(event);
//     EXPECT_EQ(club.clients.size(), 0);
//     EXPECT_EQ(club.eventsOut.size(), 2); // Input event + error
    
//     auto errorEvent = club.eventsOut.back();
//     EXPECT_EQ(errorEvent.eventID, EventID::outError);
//     EXPECT_EQ(errorEvent.errorBody, "NotOpenYet");
// }

// TEST_F(ClubTest, TestClientSitAtTable) {
//     // First client enters
//     Event enterEvent;
//     enterEvent.timestamp = minutesAfterMidnight("10:00");
//     enterEvent.eventID = EventID::inClientEnter;
//     enterEvent.client = "client1";
//     club.processEventIn(enterEvent);
    
//     // Client sits at table 1
//     Event sitEvent;
//     sitEvent.timestamp = minutesAfterMidnight("10:05");
//     sitEvent.eventID = EventID::inClientTable;
//     sitEvent.client = "client1";
//     sitEvent.numTable = 0; // Tables are 0-indexed in code
//     club.processEventIn(sitEvent);
    
//     EXPECT_EQ(club.tables[0].beingUsed, true);
//     EXPECT_EQ(club.clients["client1"], 0);
// }

// TEST_F(ClubTest, TestClientExitAndQueue) {
//     // Setup two clients
//     Event enter1, enter2;
//     enter1.timestamp = minutesAfterMidnight("10:00");
//     enter1.eventID = EventID::inClientEnter;
//     enter1.client = "client1";
//     club.processEventIn(enter1);
    
//     enter2.timestamp = minutesAfterMidnight("10:05");
//     enter2.eventID = EventID::inClientEnter;
//     enter2.client = "client2";
//     club.processEventIn(enter2);
    
//     // Client1 sits at table 1
//     Event sitEvent;
//     sitEvent.timestamp = minutesAfterMidnight("10:10");
//     sitEvent.eventID = EventID::inClientTable;
//     sitEvent.client = "client1";
//     sitEvent.numTable = 0;
//     club.processEventIn(sitEvent);
    
//     // Client2 waits
//     Event waitEvent;
//     waitEvent.timestamp = minutesAfterMidnight("10:15");
//     waitEvent.eventID = EventID::inClientWait;
//     waitEvent.client = "client2";
//     club.processEventIn(waitEvent);
    
//     EXPECT_EQ(club.queue.size(), 1);
    
//     // Client1 exits - should trigger client2 to take the table
//     Event exitEvent;
//     exitEvent.timestamp = minutesAfterMidnight("11:00");
//     exitEvent.eventID = EventID::inClientExit;
//     exitEvent.client = "client1";
//     club.processEventIn(exitEvent);
    
//     EXPECT_EQ(club.queue.size(), 0);
//     EXPECT_EQ(club.clients["client2"], 0);
//     EXPECT_EQ(club.eventsOut.back().eventID, EventID::outClientTable);
// }

// TEST_F(ClubTest, TestEndOfDayCleanup) {
//     // Client enters and sits at table
//     Event enterEvent;
//     enterEvent.timestamp = minutesAfterMidnight("18:00");
//     enterEvent.eventID = EventID::inClientEnter;
//     enterEvent.client = "client1";
//     club.processEventIn(enterEvent);
    
//     Event sitEvent;
//     sitEvent.timestamp = minutesAfterMidnight("18:05");
//     sitEvent.eventID = EventID::inClientTable;
//     sitEvent.client = "client1";
//     sitEvent.numTable = 0;
//     club.processEventIn(sitEvent);
    
//     // Force end of day cleanup
//     club.handleRemainingClients();
    
//     // Should have exit event for client1
//     bool foundExit = false;
//     for (const auto& event : club.eventsOut) {
//         if (event.eventID == EventID::outClientExit && event.client == "client1") {
//             foundExit = true;
//             break;
//         }
//     }
//     EXPECT_TRUE(foundExit);
// }

// TEST_F(ClubTest, TestRevenueCalculation) {
//     // Client enters and sits at table
//     Event enterEvent;
//     enterEvent.timestamp = minutesAfterMidnight("10:00");
//     enterEvent.eventID = EventID::inClientEnter;
//     enterEvent.client = "client1";
//     club.processEventIn(enterEvent);
    
//     Event sitEvent;
//     sitEvent.timestamp = minutesAfterMidnight("10:05");
//     sitEvent.eventID = EventID::inClientTable;
//     sitEvent.client = "client1";
//     sitEvent.numTable = 0;
//     club.processEventIn(sitEvent);
    
//     // Client exits after 45 minutes (should pay for 1 hour)
//     Event exitEvent;
//     exitEvent.timestamp = minutesAfterMidnight("10:50");
//     exitEvent.eventID = EventID::inClientExit;
//     exitEvent.client = "client1";
//     club.processEventIn(exitEvent);
    
//     EXPECT_EQ(club.tables[0].earnings, 10);
//     EXPECT_EQ(club.tables[0].timeUsed.count(), 45);
// }

// TEST_F(ClubTest, TestParseLine) {
//     std::string line = "10:15 2 client1 3";
//     Event event = parseLine(line);
    
//     EXPECT_EQ(event.timestamp.count(), 615); // 10*60 + 15
//     EXPECT_EQ(event.eventID, EventID::inClientTable);
//     EXPECT_EQ(event.client, "client1");
//     EXPECT_EQ(event.numTable, 2); // 3-1 as per the code
// }

// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }