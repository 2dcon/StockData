#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <cstring>
#include <string>
#include <iostream>

namespace StockData
{
    constexpr size_t SYMBOL_SIZE = 8; // 6 characters + null terminator + 1 padding
    constexpr size_t TICK_INFO_SIZE = StockData::SYMBOL_SIZE - sizeof(uint32_t) - sizeof(size_t);
    constexpr size_t BAR_INFO_SIZE = 6 + sizeof(int);
    
    struct Tick
    {
        uint64_t time; // time of the day, e.g. 103003
        double price;
        double transactionCount;
        double tickVolume;
        double tickAmount;
        double dayVolume;
        double dayAmount;
        double askVolumes[5];
        double askPrices[5];
        double bidVolumes[5];
        double bidPrices[5];
    };
    
    struct Ticks
    {
        char symbol[SYMBOL_SIZE];
        uint64_t date;
        size_t tickCount;
        Tick* data;
    };
    
    enum class DataFrequency
    {
        Undefined = 0,
        Tick = 3,
        Bar1m = 60,
        Bar1d = 14400 //  4 hours * 60 minutes * 60 seconds
    };

    struct Bar
    {
        uint64_t time;  // for 1d bar or 1m bar
        double open;
        double high;
        double low;
        double close;
        double volume;
        double amount;
    };

    struct Bars
    {
        char symbol[SYMBOL_SIZE];
        DataFrequency frequency;
        Bar* data = nullptr;
        size_t barCount = 0; // not stored in the file

        ~Bars()
        {
            // std::cout << "Deleting bars for " << symbol << '\n';
            if (data != nullptr && barCount > 0)
            {
                delete[] data;
                data = nullptr;
                barCount = 0;
            }
        }
    };

    void ReadTicks(const char* buffer, const size_t& bufferSize, Ticks& ticks);
    void ReadTicks(const std::string& filePath, Ticks& ticks);

    void ReadBars(const std::string& filePath, Bars& bars);

    enum EventTypes
    {
        LimitUp,
        LimitDown
    };

    struct Event
    {
        std::string Symbol;
        EventTypes EventType;
        uint32_t Date;
        uint32_t Time = 0;

        Event(const std::string& symbol, EventTypes eventType, uint32_t date, uint32_t time = 0)
            : Symbol(symbol), EventType(eventType), Date(date), Time(time)
            {}
    };
}