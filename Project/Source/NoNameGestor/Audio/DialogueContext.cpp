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

uint64_t DialogueContext::CurrentId() const
{
	return path.empty() ? 0 : path.back();
}
