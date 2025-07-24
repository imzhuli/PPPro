#include "./region.hpp"

uint32_t CountryCodeToCountryId(const char * CC) {
    RuntimeAssert(CC);
    auto len = strlen(CC);
    if (len != 2) {
        return 0;
    }
    return (static_cast<uint32_t>(CC[0]) << 8) + (static_cast<uint32_t>(CC[1]));
}
