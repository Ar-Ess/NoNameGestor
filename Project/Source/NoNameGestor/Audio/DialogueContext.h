#pragma once

#include <cstdint>
#include <vector>

class DialogueContext
{
public:

    const std::vector<uint64_t>& Path() const;
    void Path(std::vector<uint64_t>&& p);
    void Clear();

    uint64_t CurrentId() const;

private:

    std::vector<uint64_t> path;

};

