#include "AwakeDetector.h"

AwakeDetector::AwakeDetector(Configuration& config) :
	config(config)
{
}

AwakeDetector::Result AwakeDetector::Filter(std::string& normalized)
{
    if (normalized.empty())
        return Result::IGNORED;

    using clock = std::chrono::steady_clock;
    const auto now = clock::now();

    std::string& word = config.activeListenWordNormalized;
    
    const size_t pos = FindWholeWord(normalized, word);

    if (pos != std::string::npos)
    {
        // Tot el que hi ha fins a la paraula (inclosa) es descarta.
        size_t after = pos + word.size();
        if (after < normalized.size())
            ++after; // salta l'espai que ve just després
        normalized.erase(0, after);

        awakeUntil = now + window;
        return normalized.empty() ? Result::WOKE : Result::COMMAND;
    }

    if (now >= awakeUntil)
        return Result::IGNORED;

    awakeUntil = now + std::chrono::seconds{ config.activeListenWordAwakeTime }; // Renova
    return Result::COMMAND;
}

size_t AwakeDetector::FindWholeWord(const std::string& text, const std::string& word)
{
    if (word.empty())
        return std::string::npos;

    size_t pos = text.find(word);
    while (pos != std::string::npos)
    {
        const size_t end = pos + word.size();

        const bool startOk = (pos == 0) || text[pos - 1] == ' ';
        const bool endOk = (end == text.size()) || text[end] == ' ';

        if (startOk && endOk)
            return pos;

        pos = text.find(word, pos + 1);    // era dins d'una altra paraula: segueix buscant
    }
    return std::string::npos;
}
