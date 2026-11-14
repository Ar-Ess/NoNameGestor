#include "Interpreter.h"

#include <unordered_set>
#include "NoNameGestor/External/imgui/imgui_internal.h"

Intent Interpreter::Process(const std::string& sentence)
{
    std::string normalized = Normalize(sentence);

    if (!IsValid(normalized))
        return Intent();

    lastText = normalized;

    // Interpret text to real meaning

    return Intent{ sentence, normalized, "", {} };
}

void Interpreter::Draw()
{
    ImGui::Text("Last Interpreted Text: \"%s\"", lastText.c_str());
}

std::string Interpreter::Normalize(const std::string& sentence)
{
    std::string out;
    out.reserve(sentence.size());
    int depth = 0;          // dins d'un [] o ()
    bool lastSpace = true;  // true al principi: evita espais inicials

    for (unsigned char c : sentence)
    {
        if (c == '[' || c == '(') { ++depth; continue; }
        if (c == ']' || c == ')') { if (depth > 0) --depth; continue; }
        if (depth > 0) continue;

        if (std::isalnum(c) || c == '\'')
        {
            out += (char)std::tolower(c);
            lastSpace = false;
        }
        else if (!lastSpace)
        {
            out += ' ';
            lastSpace = true;
        }
    }

    if (!out.empty() && out.back() == ' ')
        out.pop_back();

    return out;
}

bool Interpreter::IsValid(const std::string& sentence)
{
    static const std::unordered_set<std::string> hallucinations = {
        "thank you", "thanks", "thanks for watching", "you", "bye"
    };

    return !sentence.empty() && !hallucinations.contains(sentence);
}
