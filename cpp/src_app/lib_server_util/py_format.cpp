#include "./py_format.hpp"

static const char * TrySkipSpaces(const char * Stream) {
    auto PC = Stream;
    while (auto C = *PC) {
        if (isspace(C)) {
            ++PC;
            continue;
        }
        return PC;
    }
    return PC;
}

static const char * TryParseString(const char * StringBegin, std::string & output) {
    assert(*StringBegin);
    assert(output.empty());

    auto PC      = StringBegin;
    auto EndCode = *PC;  // compat with python-like rules

    auto IsConverting = false;
    while (auto C = *++PC) {
        if (IsConverting) {
            output      += C;
            IsConverting = false;
            continue;
        }
        if (C == '\\') {
            assert(!IsConverting);
            IsConverting = true;
            continue;
        }
        if (C == '\'' || C == '"') {
            if (C == EndCode) {
                return PC;
            } else {
                output += C;
                continue;
            }
        }
        output += C;
    }
    return nullptr;  // not found
}

xOptional<std::vector<std::string>> ParsePythonStringArray(const std::string & Source) {
    auto PC = Source.c_str();
    PC      = TrySkipSpaces(PC);

    if (*PC++ != '[') {  // no array begin
        return {};
    }
    std::vector<std::string> Segs;
    while (true) {
        PC = TrySkipSpaces(PC);
        if (*PC == ']') {  // test correct finish
            PC = TrySkipSpaces(++PC);
            if (*PC == '\0') {  // ok, array finished with no error
                return { std::move(Segs) };
            }
            return {};  // error, something strange is here
        }
        if (*PC == '\0') {
            return {};  // unfinished segs
        }
        if (*PC == '\'' || *PC == '\"') {
            auto TempString = std::string();
            auto PStringEnd = TryParseString(PC, TempString);
            if (!PStringEnd) {  // error: unfinished string
                return {};
            }
            Segs.push_back(std::move(TempString));
            PC = PStringEnd;
            ++PC;
            continue;
        }
        if (*PC == ',') {
            ++PC;
            continue;
        }
        break;  // invalid charactor
    }
    return {};
}
