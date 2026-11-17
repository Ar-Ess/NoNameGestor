#include "DialogueContext.h"

const std::vector<uint64_t>& DialogueContext::Path() const
{
	 return path;
}

void DialogueContext::Path(std::vector<uint64_t>&& p)
{
	path = std::move(p);
}

void DialogueContext::Clear()
{
	path.clear();
}

void DialogueContext::Truncate(size_t index)
{
	path.resize(index);
}

void DialogueContext::Push(uint64_t id)
{
	path.push_back(id);
}

uint64_t DialogueContext::CurrentId() const
{
	return path.empty() ? 0 : path.back();
}
