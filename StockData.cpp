#include "StockData.hpp"
#include <iostream>
#include <fstream>

void StockData::ReadTicks(const char* buffer, const size_t& bufferSize, Ticks& ticks)
{
    char* bufferPos = (char*)buffer;
    size_t dataSize = bufferSize - StockData::TICK_INFO_SIZE;
    size_t dataCount = dataSize / sizeof(StockData::Tick);

    memcpy(&ticks.symbol, bufferPos, StockData::SYMBOL_SIZE);
    bufferPos += StockData::SYMBOL_SIZE;
    memcpy(&ticks.date, bufferPos, sizeof(uint64_t));
    bufferPos += sizeof(uint64_t);
    memcpy(&ticks.tickCount, bufferPos, sizeof(size_t));
    bufferPos += sizeof(size_t);

    if (ticks.tickCount != dataCount)
    {
        std::cerr << "Data count mismatch: " << ticks.tickCount << " != " << dataCount << '\n';
    }
    ticks.data = new StockData::Tick[dataCount];
    memcpy(ticks.data, bufferPos, dataSize);
}

void StockData::ReadTicks(const std::string &filePath, Ticks &ticks)
{
    std::ifstream file(filePath, std::ios::binary);
    if (file.is_open())
    {
        // std::cout << "Opened file: " << filePath << '\n';
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        size_t dataSize = fileSize - StockData::TICK_INFO_SIZE;
        size_t dataCount = dataSize / sizeof(StockData::Tick);

        file.read((char*)&ticks.symbol, StockData::SYMBOL_SIZE);
        file.read((char*)&ticks.date, sizeof(uint64_t));
        file.read((char*)&ticks.tickCount, sizeof(size_t));

        if (ticks.tickCount != dataCount)
        {
            std::cerr << "Data count mismatch: " << ticks.tickCount << " != " << dataCount << '\n';
        }
        ticks.data = new StockData::Tick[dataCount];
        file.read((char*)ticks.data, dataSize);

        file.close();
    }
    else
    {
        std::cerr << "Failed to open file: " << filePath << '\n';
    }
}

void StockData::ReadBars(char *buffer, size_t bufferSize, Bars &bars)
{
    char *bufferPos = buffer;
    size_t dataSize = bufferSize - StockData::BAR_INFO_SIZE;
    bars.barCount = dataSize / sizeof(StockData::Bar);

    memcpy(&bars.symbol, bufferPos, StockData::BARS_SYMBOL_SIZE);
    bufferPos += StockData::BARS_SYMBOL_SIZE;
    memcpy(&bars.frequency, bufferPos, sizeof(DataFrequency));
    bufferPos += sizeof(DataFrequency);

    bars.data = new StockData::Bar[bars.barCount];
    memcpy(bars.data, bufferPos, dataSize);
}

void StockData::ReadBars(const std::string &filePath, Bars &bars)
{
    const size_t symbolSize = 6;
    std::ifstream file(filePath, std::ios::binary);
    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        size_t dataSize = fileSize - StockData::BAR_INFO_SIZE;
        size_t dataCount = dataSize / sizeof(StockData::Bar);

        bars.data = new StockData::Bar[dataCount];
        file.read((char*)&bars.symbol, symbolSize);
        file.read((char*)&bars.frequency, sizeof(DataFrequency));
        file.read((char*)bars.data, dataSize);
        bars.barCount = dataCount;

        file.close();
    }
    else
    {
        std::cerr << "Failed to open file: " << filePath << '\n';
    }
}