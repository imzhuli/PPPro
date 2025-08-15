#include <pp_common/_.hpp>

int main(int, char **) {

    auto TimestampMS = xel::GetTimestampMS();
    auto SecretKey   = std::string("SomeSecretKey1024");
    auto SomeData    = std::string("Hello world again!!!");

    auto Sign = AppSign(TimestampMS, SecretKey, SomeData);
    cout << Sign << endl;

    auto Valid = ValidateAppSign(Sign, SecretKey, SomeData);
    cout << "valid=" << YN(Valid) << endl;

    auto Address  = xNetAddress::Parse("127.0.0.1:1234");
    auto SignedIp = SignAndPackAddress(GetTimestampMS(), SecretKey, Address);
    cout << "Signed Ip : " << SignedIp << endl;

    auto ExtractedAddress = ExtractAddressFromPack(SignedIp, SecretKey);
    cout << "ExtractedAddress: " << ExtractedAddress.ToString() << endl;

    return 0;
}