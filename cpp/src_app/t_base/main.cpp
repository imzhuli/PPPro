#include <pp_common/_.hpp>

int main(int, char **) {

    auto TimestampMS = xel::GetTimestampMS();
    auto SecretKey   = std::string("SomeSecretKey1024");
    auto SomeData    = std::string("Hello world again!!!");

    auto Sign = AppSign(TimestampMS, SecretKey, SomeData);
    cout << Sign << endl;

    auto Valid = ValidateAppSign(Sign, SecretKey, SomeData);
    cout << "valid=" << YN(Valid) << endl;

    return 0;
}