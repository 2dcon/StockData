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

std::string StockData::GetFilePath(const std::string &symbol, DataFrequency frequency, ulong date)
{
    switch(frequency)
    {
        case DataFrequency::Bar1m:
            return DATA_DIR_1M + '/' + symbol + '/' + std::to_string(date) + ".1m.bars";
        case DataFrequency::Bar1d:
            return DATA_DIR_1D + '/' + symbol + ".1d.bars";
        case DataFrequency::Tick:
            // if (date != 0)
            // {
            //     return DATA_DIR_TICK + "/" + symbol + "." + std::to_string(date) + ".ticks";
            // }
            // else
            // {
            //     std::cerr << "Date is required for tick data.\n";
            //     return std::string();
            // }
            return std::string(); // tick data loading is not supported yet
        default:
            std::cerr << "Unsupported frequency.\n";
            return std::string();
    }
}

void StockData::ReadBars(const char *buffer, size_t bufferSize, Bars &bars)
{
    const char *bufferPos = buffer;
    size_t dataSize = bufferSize - StockData::BAR_INFO_SIZE;
    size_t barCount = dataSize / sizeof(StockData::Bar);

    bars.symbol.resize(BARS_SYMBOL_SIZE, '\0');
    memcpy(bars.symbol.data(), bufferPos, StockData::BARS_SYMBOL_SIZE);
    bufferPos += StockData::BARS_SYMBOL_SIZE;

    memcpy(&bars.frequency, bufferPos, sizeof(DataFrequency));
    bufferPos += sizeof(DataFrequency);

    bars.data = std::vector<StockData::Bar>(barCount);
    memcpy(bars.data.data(), bufferPos, dataSize);
}

void StockData::ReadBars(const std::string &filePath, Bars &bars)
{
    std::ifstream file(filePath, std::ios::binary);
    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        size_t dataSize = fileSize - StockData::BAR_INFO_SIZE;

        std::string fileSuffix = filePath.substr(filePath.size() - 8);
        if (fileSuffix == ".1m.bars")
        {
            dataSize -= sizeof(uint16_t);
        }
        size_t dataCount = dataSize / sizeof(StockData::Bar);
        
        bars.symbol.resize(BARS_SYMBOL_SIZE + 1, '\0'); // +1 for null terminator
        file.read(bars.symbol.data(), BARS_SYMBOL_SIZE);

        file.read((char*)&bars.frequency, sizeof(DataFrequency));
        
        bars.data = std::vector<StockData::Bar>(dataCount);
        file.read((char*)bars.data.data(), dataSize);

    }
    else
    {
        std::cerr << "Failed to open file: " << filePath << '\n';
    }
}