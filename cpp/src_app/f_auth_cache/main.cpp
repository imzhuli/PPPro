#include "../lib_utils/all.hpp"

static std::string ToUpper(std::string_view sv) {
    std::string result(sv);  // 从 string_view 构造 string
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::toupper(c); });
    return result;
}

static xGeoInfo UsernameToGeoSelection(const std::string_view & Username) {
    auto UU        = ToUpper(Username);
    auto CountryId = CountryCodeToCountryId(UU.c_str());

    return xGeoInfo{
        .CountryId = CountryId,
    };
}

int main(int argc, char ** argv) {
    X_VAR xRuntimeEnvGuard(argc, argv);
    auto  CL = RuntimeEnv.LoadConfig();

    cout << UsernameToGeoSelection("BR").CountryId << endl;

    return 0;
}
