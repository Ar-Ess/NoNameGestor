#pragma once

#include <cstdint>
#include <vector>
#include "DetailLevel.h"

class DialogueContext
{
    friend class VoiceManager;

public:

    const std::vector<uint64_t>& Path() const;
    void Path(std::vector<uint64_t>&& p);
    void Clear();
    void Truncate(size_t index);
    void Push(uint64_t id);
    uint64_t CurrentId() const;

private:

    std::vector<uint64_t> path;
    bool reading = false;
    DetailLevel readDetail = DetailLevel::BRIEF;
    uint64_t scopeId = 0;

};

