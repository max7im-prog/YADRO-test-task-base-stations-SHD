#include "club.h"
#include <gtest/gtest.h>
#include <chrono>

class ClubTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize club with 3 tables, working hours 09:00-19:00, $10/hour
        club.numTables = 3;
        club.tables.resize(club.numTables);
        club.timeBegin = minutesAfterMidnight("09:00");
        club.timeEnd = minutesAfterMidnight("19:00");
        club.costPerHour = 10;
    }

    Club club;
};

TEST_F(ClubTest, TimeConversionWorks)
{
    auto time = minutesAfterMidnight("09:00");
    EXPECT_EQ(time.count(), 540);

    time = minutesAfterMidnight("23:59");
    EXPECT_EQ(time.count(), 1439);

    time = minutesAfterMidnight("00:00");
    EXPECT_EQ(time.count(), 0);
}

TEST_F(ClubTest, ClientCanEnterDuringWorkingHours)
{
    Event event;
    event.timestamp = minutesAfterMidnight("10:00");
    event.eventID = EventID::inClientEnter;
    event.client = "client1";

    bool result = club.processEventIn(event);
    EXPECT_TRUE(result);
    EXPECT_EQ(club.clients.size(), 1);
}

TEST_F(ClubTest, ClientCannotEnterOutsideWorkingHours)
{
    Event event;
    event.timestamp = minutesAfterMidnight("08:00");
    event.eventID = EventID::inClientEnter;
    event.client = "client1";

    club.processEventIn(event);
    EXPECT_EQ(club.clients.size(), 0);
    EXPECT_EQ(club.eventsOut.back().eventID, EventID::outError);
    EXPECT_EQ(club.eventsOut.back().errorBody, "NotOpenYet");
}

TEST_F(ClubTest, ClientCanSitAtFreeTable)
{
    // Client enters
    Event enterEvent;
    enterEvent.timestamp = minutesAfterMidnight("10:00");
    enterEvent.eventID = EventID::inClientEnter;
    enterEvent.client = "client1";
    club.processEventIn(enterEvent);

    // Client sits at table 1
    Event sitEvent;
    sitEvent.timestamp = minutesAfterMidnight("10:05");
    sitEvent.eventID = EventID::inClientTable;
    sitEvent.client = "client1";
    sitEvent.numTable = 0; // Tables are 0-indexed in code
    club.processEventIn(sitEvent);

    EXPECT_TRUE(club.tables[0].beingUsed);
    EXPECT_EQ(club.clients["client1"], 0);
}

TEST_F(ClubTest, ClientCannotSitAtOccupiedTable)
{
    // First client enters and sits
    Event enter1, sit1;
    enter1.timestamp = minutesAfterMidnight("10:00");
    enter1.eventID = EventID::inClientEnter;
    enter1.client = "client1";
    club.processEventIn(enter1);

    sit1.timestamp = minutesAfterMidnight("10:05");
    sit1.eventID = EventID::inClientTable;
    sit1.client = "client1";
    sit1.numTable = 0;
    club.processEventIn(sit1);

    // Second client tries to sit at same table
    Event enter2, sit2;
    enter2.timestamp = minutesAfterMidnight("10:10");
    enter2.eventID = EventID::inClientEnter;
    enter2.client = "client2";
    club.processEventIn(enter2);

    sit2.timestamp = minutesAfterMidnight("10:15");
    sit2.eventID = EventID::inClientTable;
    sit2.client = "client2";
    sit2.numTable = 0;
    club.processEventIn(sit2);

    EXPECT_EQ(club.eventsOut.back().eventID, EventID::outError);
    EXPECT_EQ(club.eventsOut.back().errorBody, "PlaceIsBusy");
}

TEST_F(ClubTest, RevenueCalculationIsCorrect)
{
    // Client enters and sits
    Event enter, sit;
    enter.timestamp = minutesAfterMidnight("10:00");
    enter.eventID = EventID::inClientEnter;
    enter.client = "client1";
    club.processEventIn(enter);

    sit.timestamp = minutesAfterMidnight("10:05");
    sit.eventID = EventID::inClientTable;
    sit.client = "client1";
    sit.numTable = 0;
    club.processEventIn(sit);

    // Client exits after 45 minutes (should pay for 1 hour)
    Event exit;
    exit.timestamp = minutesAfterMidnight("10:50");
    exit.eventID = EventID::inClientExit;
    exit.client = "client1";
    club.processEventIn(exit);

    EXPECT_EQ(club.tables[0].earnings, 10);         // $10 for 1 hour
    EXPECT_EQ(club.tables[0].timeUsed.count(), 45); // 45 minutes usage
}

TEST_F(ClubTest, QueueBehaviorWorks)
{
    // First client enters and sits at table 1
    Event enter1, sit1;
    enter1.timestamp = minutesAfterMidnight("10:00");
    enter1.eventID = EventID::inClientEnter;
    enter1.client = "client1";
    club.processEventIn(enter1);

    sit1.timestamp = minutesAfterMidnight("10:05");
    sit1.eventID = EventID::inClientTable;
    sit1.client = "client1";
    sit1.numTable = 0; // Table 1 (0-indexed)
    club.processEventIn(sit1);

    // Second client enters and sits at table 2
    Event enter2, sit2;
    enter2.timestamp = minutesAfterMidnight("10:10");
    enter2.eventID = EventID::inClientEnter;
    enter2.client = "client2";
    club.processEventIn(enter2);

    sit2.timestamp = minutesAfterMidnight("10:15");
    sit2.eventID = EventID::inClientTable;
    sit2.client = "client2";
    sit2.numTable = 1; // Table 2
    club.processEventIn(sit2);

    // Third client enters and sits at table 3
    Event enter3, sit3;
    enter3.timestamp = minutesAfterMidnight("10:20");
    enter3.eventID = EventID::inClientEnter;
    enter3.client = "client3";
    club.processEventIn(enter3);

    sit3.timestamp = minutesAfterMidnight("10:25");
    sit3.eventID = EventID::inClientTable;
    sit3.client = "client3";
    sit3.numTable = 2; // Table 3
    club.processEventIn(sit3);

    // Now all tables are occupied
    // Fourth client enters and should wait
    Event enter4, wait;
    enter4.timestamp = minutesAfterMidnight("10:30");
    enter4.eventID = EventID::inClientEnter;
    enter4.client = "client4";
    club.processEventIn(enter4);

    wait.timestamp = minutesAfterMidnight("10:35");
    wait.eventID = EventID::inClientWait;
    wait.client = "client4";
    club.processEventIn(wait);

    EXPECT_EQ(club.queue.size(), 1);
    EXPECT_EQ(club.queue.front(), "client4");

    // First client leaves - should trigger fourth client to take table 1
    Event exit;
    exit.timestamp = minutesAfterMidnight("11:00");
    exit.eventID = EventID::inClientExit;
    exit.client = "client1";
    club.processEventIn(exit);

    EXPECT_EQ(club.queue.size(), 0);
    EXPECT_EQ(club.clients["client4"], 0); // Should be at table 1 (0-indexed)

    // Verify the outClientTable event was generated
    bool foundTableEvent = false;
    for (const auto &event : club.eventsOut)
    {
        if (event.eventID == EventID::outClientTable &&
            event.client == "client4" &&
            event.numTable == 0)
        {
            foundTableEvent = true;
            break;
        }
    }
    EXPECT_TRUE(foundTableEvent);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}