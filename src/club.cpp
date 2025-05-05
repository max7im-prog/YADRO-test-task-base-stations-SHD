#include "club.h"
#include <algorithm>
#include <sstream>

Club::Club()
{
}

std::list<int> Club::getFreeTables()
{
    std::list<int> ret;
    for (int i = 0; i < numTables; i++)
    {
        if (!this->tables[i].beingUsed)
        {
            ret.push_back(i);
        }
    }
    return ret;
}

std::string fromMinToString(std::chrono::minutes m)
{
    std::string ret = "";
    // return std::to_string(m.count()/60) + ":" + std::to_string(m.count()%60);
    if ((m.count() / 60) % 24 < 10)
    {
        ret += "0";
    }
    ret += std::to_string((m.count() / 60) % 24);
    ret += ":";
    if ((m.count() % 60) < 10)
    {
        ret += "0";
    }
    ret += std::to_string((m.count() % 60));

    return ret;
}

std::chrono::minutes minutesAfterMidnight(std::string time_hh_mm)
{
    std::chrono::minutes ret;
    std::istringstream ss(time_hh_mm);
    int hours, minutes;
    char colon;
    ss >> hours >> colon >> minutes;
    ret = std::chrono::minutes(hours * 60 + minutes);

    return ret;
}

Event parseLine(std::string line)
{
    std::istringstream ss(line);
    std::string temp;
    Event ret = {};
    int a;

    ss >> temp;
    ret.timestamp = minutesAfterMidnight(temp);

    ss >> ret.eventID;

    if (ret.eventID == EventID::inClientTable)
    {
        ss >> ret.client;
        ss >> ret.numTable;
        ret.numTable -= 1; // Numbering starts from 1 instead of 0, so subtract 1
    }
    else
    {
        ss >> ret.client;
    }

    ret.fullString = line;

    return ret;
}

Table::Table()
{
    this->earnings = 0;
    this->timeUsed = std::chrono::minutes(0);
    this->beingUsed = false;
}

void Club::handleRemainingClients()
{
    std::list<std::string> temp;
    for (const auto &pair : this->clients)
    {
        temp.push_back(pair.first);
    }
    temp.sort();
    for (auto client : temp)
    {
        Event newEvent;
        newEvent.timestamp = this->timeEnd;
        newEvent.client = client;
        newEvent.eventID = EventID::outClientExit;
        this->eventsOut.push_back(newEvent);
        this->exitTable(newEvent.timestamp, client);

        this->exitClient(client, newEvent.timestamp);
    }
}

// Client exits a club
bool Club::exitClient(std::string client, std::chrono::minutes timestamp)
{
    if (this->clients.find(client) == this->clients.end())
    {
        return false;
    }
    this->exitTable(timestamp, client);

    // remove client from the queue
    this->queue.remove(client);

    // remove client from the client map
    this->clients.erase(client);

    return true;
}

// Client enters a table
bool Club::enterTable(int tableNum, std::chrono::minutes timestamp, std::string client)
{
    if (tableNum < 0 || tableNum > this->numTables)
    {
        return false;
    }

    if (tables[tableNum].beingUsed)
    {
        return false;
    }

    if (this->clients[client] != -1)
    {
        this->exitTable(timestamp, client);
    }

    this->clients[client] = tableNum;
    this->tables[tableNum].timeBegin = timestamp;
    this->tables[tableNum].beingUsed = true;

    return true;
}

// Client exits a table, but not the club
bool Club::exitTable(std::chrono::minutes timestamp, std::string client)
{
    int tableNum = this->clients[client];

    if (tableNum < 0 || tableNum > this->numTables)
    {
        return false;
    }

    if (!tables[tableNum].beingUsed)
    {
        return true;
    }

    tables[tableNum].beingUsed = false;
    this->clients[client] = -1;

    auto timeAdd = std::chrono::minutes(timestamp - tables[tableNum].timeBegin);
    int earningsAdd = ((int(timeAdd.count()) + 59) / 60) * this->costPerHour;

    tables[tableNum].timeUsed += timeAdd;
    tables[tableNum].earnings += earningsAdd;

    return true;
}

// Processes incoming event by adding resulting events to eventsOut of a club
bool Club::processEventIn(Event event)
{

    bool ret = true;
    this->eventsOut.push_back(event);

    Event newEvent = event;

    if (event.eventID == EventID::inClientEnter)
    {
        if (this->clients.find(event.client) != this->clients.end())
        {
            newEvent.eventID = EventID::outError;
            newEvent.errorBody = "YouShallNotPass";
            this->eventsOut.push_back(newEvent);
        }
        else if (event.timestamp < this->timeBegin || this->timeEnd < event.timestamp)
        {
            newEvent.eventID = EventID::outError;
            newEvent.errorBody = "NotOpenYet";
            this->eventsOut.push_back(newEvent);
        }
        else
        {
            this->clients[event.client] = -1;
        }
    }
    else if (event.eventID == EventID::inClientTable)
    {
        if (this->tables[event.numTable].beingUsed)
        {
            newEvent.eventID = EventID::outError;
            newEvent.errorBody = "PlaceIsBusy";
            this->eventsOut.push_back(newEvent);
        }
        else if (this->clients.find(event.client) == this->clients.end())
        {
            newEvent.eventID = EventID::outError;
            newEvent.errorBody = "ClientUnknown";
            this->eventsOut.push_back(newEvent);
        }
        else
        {
            this->enterTable(event.numTable, event.timestamp, event.client);
        }
    }
    else if (event.eventID == EventID::inClientWait)
    {
        if (this->getFreeTables().size() != 0)
        {
            newEvent.eventID = EventID::outError;
            newEvent.errorBody = "ICanWaitNoLonger!";
            this->eventsOut.push_back(newEvent);
        }
        else if (this->queue.size() > this->numTables)
        {
            newEvent.eventID = EventID::outClientExit;
            this->eventsOut.push_back(newEvent);
            this->exitClient(newEvent.client, newEvent.timestamp);
        }
        else
        { // Add client to queue only if he isn't already in the queue
            if (std::find(this->queue.begin(), this->queue.end(), event.client) == this->queue.end())
            {
                this->queue.push_back(event.client);
            }
        }
    }
    else if (event.eventID == EventID::inClientExit)
    {
        if (this->clients.find(event.client) == this->clients.end())
        {
            newEvent.eventID = EventID::outError;
            newEvent.errorBody = "ClientUnknown";
            this->eventsOut.push_back(newEvent);
        }
        else
        {
            int table = this->clients[event.client];
            if (table != -1)
            {
                this->exitTable(event.timestamp, event.client);
                if (this->queue.size() != 0)
                {
                    std::string newClient = this->queue.front();
                    this->queue.pop_front();
                    this->enterTable(table, event.timestamp, newClient);

                    newEvent.eventID = EventID::outClientTable;
                    newEvent.client = newClient;
                    newEvent.numTable = table;
                    this->eventsOut.push_back(newEvent);
                }
                this->exitClient(event.client, event.timestamp);
            }
        }
    }
    else
    { // Wrong type of event
        ret = false;
    }

    return ret;
}

std::ostream &operator<<(std::ostream &os, const Event &e)
{
    // os << "time: " << e.timestamp.count() << ", event ID: " << e.eventID << ", client: " << e.client << ", table number: " << e.numTable << ", error: {" << e.errorBody << "}";
    os << fromMinToString(e.timestamp) << " " << e.eventID;
    if (e.eventID == EventID::outError)
    {
        os << " " << e.errorBody;
    }
    else if (e.eventID == EventID::inClientTable || e.eventID == EventID::outClientTable)
    {
        os << " " << e.client << " " << e.numTable;
    }
    else
    {
        os << " " << e.client;
    }
    return os;
}