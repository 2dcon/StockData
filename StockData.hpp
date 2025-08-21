#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <cstring>
#include <string>
#include <iostream>

#include "utils/misc.hpp"

namespace StockData
{
    constexpr size_t SYMBOL_SIZE = 8; // 6 characters + null terminator + 1 padding
    constexpr size_t BARS_SYMBOL_SIZE = 6;
    constexpr size_t TICK_INFO_SIZE = StockData::SYMBOL_SIZE - sizeof(uint32_t) - sizeof(size_t);
    constexpr size_t BAR_INFO_SIZE = 6 + sizeof(int);
    constexpr size_t AUGMENTED_BAR_INFO_SIZE = BAR_INFO_SIZE + sizeof(double); // + average distance

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

        friend std::ostream& operator<<(std::ostream& os, const Bar& bar)
        {
            os << "[" << bar.time
               << "] " << Utils::roundPrice(bar.open)
               << " | " << Utils::roundPrice(bar.high)
               << " | " << Utils::roundPrice(bar.low)
               << " | " << Utils::roundPrice(bar.close)
               << " | " << Utils::roundAsLong(bar.volume)
               << " | " << Utils::roundAsLong(bar.amount);
            return os;
        }
    };

    struct Bars
    {
        std::string symbol;
        DataFrequency frequency = DataFrequency::Undefined;
        std::vector<Bar> data;

        // Default constructor
        Bars() = default;

        // Copy constructor
        Bars(const Bars& other) : frequency(other.frequency), data(other.data)
        {
            symbol = other.symbol;
            frequency = other.frequency;
            data = other.data;
        }

        /// @brief Find a specific number of bars from a given date (included)
        /// @param date the date from which to search
        /// @param count positive for bars after the given date, negative for otherwise
        /// @param results the vector to store the results, gets cleared in this function, ordered by date in ascending order. Dates not found will be filled with nullptr
        /// @return true if any bars were found, false otherwise
        bool GetNBarsFromDate(size_t date, size_t count, bool backward, std::vector<const Bar*>& results) const
        {
            results.clear();

            size_t dateIdx = 0;
            bool dateFound = false;
            for (size_t i = 0; i < data.size(); ++i)
            {
                if (data[i].time == date)
                {
                    dateIdx = i;
                    dateFound = true;
                    break;
                }
            }

            if (dateFound)
            {
                bool fillData = false; // fill with nullptr when no data is available
                long startIdx, endIdx, currentIdx;
                long min = 0, max = data.size() - 1;
                if (backward)
                {
                    startIdx = dateIdx - count + 1;
                    endIdx = dateIdx;
                }
                else
                {
                    startIdx = dateIdx;
                    endIdx = dateIdx + count;
                }

                currentIdx = startIdx;
                if (currentIdx < min)
                {
                    for (long i = currentIdx; i < min; ++i)
                    {
                        results.push_back(nullptr);
                    }
                    currentIdx = 0;
                }

                for (long i = currentIdx; i <= endIdx; ++i)
                {
                    if (i > max)
                    {
                        for (long j = i; j <= endIdx; ++j)
                        {
                            results.push_back(nullptr);
                        }
                        break;
                    }

                    // std::cout << "pushing: " << i << std::endl;
                    results.push_back(&data[i]);
                    // std::cout << "pushed: " << i << std::endl;
                }

                return true;
            }
            else
            {
                // std::cout << "Returning false" << std::endl;
                return false;
            }
        }

        void Clear()
        {
            symbol = "UNDEFINED";
            frequency = DataFrequency::Undefined;
            data.clear();
        }

        Bars& operator=(const Bars& other)
        {

            symbol = other.symbol;
            frequency = other.frequency;
            data = other.data;

            return *this;
        }

        ~Bars()
        {
            data.clear();
        }
    };

    struct AugmentedBar
    {
        uint64_t time;  // for 1d bar or 1m bar
        double open;
        double openDistance;
        double high;
        double highDistance;
        double low;
        double lowDistance;
        double close;
        double closeDistance;
        double average;
        double averageDistance;
        double volume;
        double volumeDistance;
        double amount;
        double amountDistance;
        double barDistance;
        uint64_t HasDistances;

        friend std::ostream& operator<<(std::ostream& os, const AugmentedBar& bar)
        {
            os << "[" << bar.time
               << "] " << Utils::roundPrice(bar.open)
               << " | " << Utils::roundPrice(bar.high)
               << " | " << Utils::roundPrice(bar.low)
               << " | " << Utils::roundPrice(bar.close)
               << " | " << Utils::roundAsLong(bar.volume)
               << " | " << Utils::roundAsLong(bar.amount);
            return os;
        }
    };

    struct AugmentedBars
    {
        std::string symbol;
        DataFrequency frequency = DataFrequency::Undefined;
        double averageDistance;
        std::vector<AugmentedBar> data;

        AugmentedBars()
        {
            symbol = "UNDEFINED";
            frequency = DataFrequency::Undefined;
            averageDistance = 0.0;
            data.clear();
        }

        /// @brief Copy constructor
        /// @param buffer
        /// @param bufferSize
        /// @return
        AugmentedBars(const AugmentedBars& other):
            symbol(other.symbol),
            frequency(other.frequency),
            averageDistance(other.averageDistance),
            data(other.data)
        {}

        AugmentedBars(const Bars& rawBars)
        {
            symbol = rawBars.symbol;
            frequency = rawBars.frequency;
            averageDistance = 0.0;
            data.clear();
            for (const auto& bar : rawBars.data)
            {
                data.push_back(AugmentedBar{
                    time: bar.time,
                    open: bar.open,
                    openDistance: 0.0,
                    high: bar.high,
                    highDistance: 0.0,
                    low: bar.low,
                    lowDistance: 0.0,
                    close: bar.close,
                    closeDistance: 0.0,
                    average: (bar.amount / bar.volume),
                    averageDistance: 0.0,
                    volume: bar.volume,
                    volumeDistance: 0.0,
                    amount: bar.amount / bar.volume,
                    amountDistance: 0.0,
                    barDistance: 0.0,
                    HasDistances: 0
                });
            }
        }

        /// @brief Read an AugmentedBars from bytes
        /// @param buffer
        /// @param bufferSize
        AugmentedBars(char* buffer, const size_t& bufferSize)
        {
            char* currentPos = buffer;

            if (bufferSize < AUGMENTED_BAR_INFO_SIZE)
            {
                std::cerr << "Buffer size is too small for AugmentedBars" << std::endl;
                return;
            }
            size_t dataCount = (bufferSize - AUGMENTED_BAR_INFO_SIZE) / sizeof(AugmentedBar);

            symbol.resize(BARS_SYMBOL_SIZE, '\0');
            memcpy(symbol.data(), currentPos, BARS_SYMBOL_SIZE);
            currentPos += BARS_SYMBOL_SIZE;

            frequency = static_cast<DataFrequency>(*reinterpret_cast<int*>(currentPos));
            currentPos += sizeof(int);

            averageDistance = *reinterpret_cast<double*>(currentPos);
            currentPos += sizeof(double);

            data.resize(dataCount);
            memcpy(data.data(), currentPos, dataCount * sizeof(AugmentedBar));

            for (size_t i = 0; i < 10; ++i)
            {
                std::cout << data[i].time << '\n';
            }
        }

        /// @brief Find a specific number of bars from a given date (included)
        /// @param date the date from which to search
        /// @param count positive for bars after the given date, negative for otherwise
        /// @param results the vector to store the results, gets cleared in this function, ordered by date in ascending order. Dates not found will be filled with nullptr
        /// @return true if any bars were found, false otherwise
        bool GetNBarsFromDate(size_t date, size_t count, bool backward, std::vector<const AugmentedBar*>& results) const
        {
            results.clear();

            size_t dateIdx = 0;
            bool dateFound = false;
            for (size_t i = 0; i < data.size(); ++i)
            {
                if (data[i].time == date)
                {
                    dateIdx = i;
                    dateFound = true;
                    break;
                }
            }

            if (dateFound)
            {
                bool fillData = false; // fill with nullptr when no data is available
                long startIdx, endIdx, currentIdx;
                long min = 0, max = data.size() - 1;
                if (backward)
                {
                    startIdx = dateIdx - count + 1;
                    endIdx = dateIdx;
                }
                else
                {
                    startIdx = dateIdx;
                    endIdx = dateIdx + count;
                }

                currentIdx = startIdx;
                if (currentIdx < min)
                {
                    for (long i = currentIdx; i < min; ++i)
                    {
                        results.push_back(nullptr);
                    }
                    currentIdx = 0;
                }

                for (long i = currentIdx; i <= endIdx; ++i)
                {
                    if (i > max)
                    {
                        for (long j = i; j <= endIdx; ++j)
                        {
                            results.push_back(nullptr);
                        }
                        break;
                    }

                    // std::cout << "pushing: " << i << std::endl;
                    results.push_back(&data[i]);
                    // std::cout << "pushed: " << i << std::endl;
                }

                return true;
            }
            else
            {
                // std::cout << "Returning false" << std::endl;
                return false;
            }
        }

        void Clear()
        {
            symbol = "UNDEFINED";
            frequency = DataFrequency::Undefined;
            averageDistance = 0.0;
            data.clear();
        }

        friend std::ostream& operator<<(std::ostream& os, const AugmentedBars& bars)
        {
            os << bars.averageDistance << " | " << bars.symbol << "\n";
            return os;
        }
    };

    void ReadTicks(const char* buffer, const size_t& bufferSize, Ticks& ticks);
    void ReadTicks(const std::string& filePath, Ticks& ticks);

    /// @brief Reads bars data from bytes
    /// @param buffer
    /// @param bufferSize
    /// @param bars
    void ReadBars(const char* buffer, size_t bufferSize, Bars& bars);

    /// @brief Reads bars data from a binary file
    /// @param filePath
    /// @param bars
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