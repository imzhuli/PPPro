#include "../lib_utils/all.hpp"

#include <pp_common/_.hpp>

int main(int argc, char ** argv) {

    // auto TimestampMS = xel::GetTimestampMS();
    // auto SecretKey   = std::string("SomeSecretKey1024");
    // auto SomeData    = std::string("Hello world again!!!");

    // auto Sign = AppSign(TimestampMS, SecretKey, SomeData);
    // cout << Sign << endl;

    // auto Valid = ValidateAppSign(Sign, SecretKey, SomeData);
    // cout << "valid=" << YN(Valid) << endl;

    // auto Address  = xNetAddress::Parse("127.0.0.1:1234");
    // auto SignedIp = SignAndPackAddress(GetTimestampMS(), SecretKey, Address);
    // cout << "Signed Ip : " << SignedIp << endl;

    // auto ExtractedAddress = ExtractAddressFromPack(SignedIp, SecretKey);
    // cout << "ExtractedAddress: " << ExtractedAddress.ToString() << endl;

    // auto Address6 = xNetAddress::Parse("[::]");
    // cout << "Address6: " << Address6.ToString() << endl;

    auto IC  = xel::xIoContext();
    auto ICG = xel::xResourceGuard(IC);

    auto TS4  = xTcpService();
    auto TS4G = xResourceGuard(TS4, &IC, xNetAddress::Parse("0.0.0.0:8888"), 1024);

    auto TS6 = xTcpService();
    // auto TS6G = xResourceGuard(TS6, &IC, xNetAddress::Parse("[::]:8888"), 1024);

    TS4.OnClientConnected = [](const xTcpServiceClientConnectionHandle & H) { cout << "Connected 4: remote=" << H.GetRemoteAddress().IpToString() << endl; };
    TS6.OnClientConnected = [](const xTcpServiceClientConnectionHandle & H) { cout << "Connected 6: remote=" << H.GetRemoteAddress().IpToString() << endl; };

    while (true) {
        IC.LoopOnce();
    }

    return 0;
}