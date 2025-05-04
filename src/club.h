#ifndef CLUB_H
#define CLUB_H

#include <chrono>
#include <string>
#include <list>
#include <vector>
#include <map>

enum EventID
{
    inClientEnter = 1,
    inClientTable = 2,
    inClientWait = 3,
    inClientExit = 4,

    outClientExit = 11,
    outClientTable = 12,
    outError = 13
};

struct Event
{
    std::chrono::minutes timestamp;
    int eventID;
    std::string client;
    int numTable;
    std::string errorBody;
    std::string fullString;
};

std::ostream &operator<<(std::ostream &os, const Event &e);

class Table
{
public:
    int earnings;
    std::chrono::minutes timeUsed;
    bool beingUsed;
    std::chrono::minutes timeBegin;
    std::chrono::minutes timeEnd;
    Table();
};

class Club
{
public:
    std::list<Event> eventsIn;
    std::list<Event> eventsOut;

    int numTables;
    std::chrono::minutes timeBegin, timeEnd;
    int costPerHour;

    std::map<std::string, int> clients;
    std::vector<Table> tables;
    std::list<std::string> queue;

    Club();
    std::list<int> getFreeTables();
    void handleRemainingClients();
    bool exitClient(std::string client, std::chrono::minutes timestamp);
    bool enterTable(int tableNum, std::chrono::minutes timestamp, std::string client);
    bool exitTable(std::chrono::minutes timestamp, std::string client);
    bool processEventIn(Event event);
};

std::chrono::minutes minutesAfterMidnight(std::string time_hh_mm);
Event parseLine(std::string line);
std::string fromMinToString(std::chrono::minutes m);

#endif // CLUB_H