#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>
#include <algorithm>
#include <map>

#include "club.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Run with <program's name> <filename>" << std::endl;
        return -1;
    }

    std::string filename = argv[1];
    if (!std::filesystem::exists(filename))
    {
        std::cerr << "File " << filename << " does not exist" << std::endl;
        return -1;
    }

    std::ifstream fin;
    std::ofstream fout;

    std::string temp;

    // Input club's characteristics
    Club club = Club();

    fin.open(filename);
    fin >> club.numTables;
    club.tables.resize(club.numTables);
    fin >> temp;
    try
    {
        club.timeBegin = minutesAfterMidnight(temp);
    }
    catch (...)
    {
        std::cerr << "Format error in \"" << temp << "\"" << std::endl;
        return -1;
    }
    fin >> temp;
    try
    {
        club.timeEnd = minutesAfterMidnight(temp);
    }
    catch (...)
    {
        std::cerr << "Format error in \"" << temp << "\"" << std::endl;
        return -1;
    }
    if (club.timeEnd < club.timeBegin)
    {
        club.timeEnd = std::chrono::minutes(club.timeEnd.count() + 24 * 60);
    }
    fin >> club.costPerHour;
    fin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Read events from file
    while (std::getline(fin, temp))
    {
        Event event = parseLine(temp);

        // Adjust for 12:00 pm
        if (event.timestamp < club.timeBegin)
        {
            Event newEvent = event;
            newEvent.timestamp = std::chrono::minutes(event.timestamp.count() + 24 * 60);
            if (club.timeBegin <= newEvent.timestamp && newEvent.timestamp <= club.timeEnd)
            {
                event = newEvent;
            }
        }
        club.eventsIn.push_back(event);
    }
    fin.close();

    // Check if events have consistent times and formats
    auto lastTimestamp = std::chrono::minutes(0);
    for (auto e : club.eventsIn)
    {
        if (e.timestamp < lastTimestamp)
        {
            std::cerr << "Timestamp error in line \"" << e.fullString << "\"" << std::endl;
            return -1;
        }
        lastTimestamp = e.timestamp;
        if (e.eventID == -1)
        {
            std::cerr << "Format error in line \"" << e.fullString << "\"" << std::endl;
            return -1;
        }
    }

    // Iterate over eventsIn and compose eventsOut
    club.eventsOut = {};
    for (auto event : club.eventsIn)
    {
        club.processEventIn(event);
    }

    // Remove every client in the club from the club
    club.handleRemainingClients();

    // Write output into output file
    fout.open("output.txt");
    if (!fout.is_open())
    {
        std::cerr << "error trying to create/open output file" << std::endl;
        return -1;
    }

    fout << fromMinToString(club.timeBegin) << std::endl;
    for (auto event : club.eventsOut)
    {
        fout << event << std::endl;
    }
    fout << fromMinToString(club.timeEnd) << std::endl;
    for (int i = 0; i < club.numTables; i++)
    {
        fout << i + 1 << " " << club.tables[i].earnings << " " << fromMinToString(club.tables[i].timeUsed) << std::endl;
    }

    fout.close();
    std::cout << "Program completed with no errors" << std::endl;
    return 0;
}
