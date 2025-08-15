// Example: Using template methods with Argy::Parser
#include "argy.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace Argy;

int main(int argc, char* argv[]) {
    CliParser cli(argc, argv);
    try {

        // Positional argument
        cli.add<string>("filename", "Input file")
           .validate(IsFile());

        // Optional argument with multiple aliases
        cli.add<int>({"-c", "--count", "-n", "--num"}, "Number of items");

        cli.add<bool>({"-v", "--visualize", "-s", "--show"}, "Visualize results"); // adding argument with multiple aliases 

        // Float argument with aliases
        cli.add<float>({"-r", "--ratio"}, "Ratio value", 0.5f)
           .validate(IsValueInRange(0.0f, 1.0f)); // Float argument with validation

        cli.add<string>("--role", "Account role", "user")
           .validate(IsOneOf({"guest", "user", "admin"})); // String argument with validation
        
        cli.add<string>("--email", "Email address", "user@email.com")
           .validate(IsEmail());
        
        cli.add<string>("--url", "Website URL", "https://example.com")
           .validate(IsUrl());

        cli.add<string>("--mac", "MAC address", "00:1A:2B:3C:4D:5E")
           .validate(IsMACAddress());

        cli.add<string>("--ip", "IP address", "192.168.1.1")   
           .validate(IsIPAddress());

        cli.add<string>("--ipv6", "IPv6 address", "2001:0db8:85a3:0000:0000:8a2e:0370:7334")
           .validate(IsIPv6());
        
        cli.add<string>("--ipv4", "IPv4 address", "192.168.0.1")
           .validate(IsIPv4());

        cli.add<string>("--uuid", "UUID", "123e4567-e89b-12d3-a456-426614174000")
           .validate(IsUUID());

        cli.add<string>("--alphaNumeric", "Alpha-numeric string", "abc123")
           .validate(IsAlphaNumeric());
        
        cli.add<string>("--alpha", "Alpha string", "abc")
           .validate(IsAlpha());

        cli.add<string>("--numeric", "Numeric string", "123")
           .validate(IsNumeric());  
        
        cli.add<string>("--path", "File or directory path", "./file.txt")
           .validate(IsPath());   

        // Vector<int> argument
        cli.add<Ints>({"-i", "--ids"}, "List of IDs", Ints{1, 2, 3})
           .validate(IsVectorInRange(1, 100)); // Vector<int> argument with validation

        // Vector<float> argument
        cli.add<Floats>({"-ss", "--scores"}, "List of scores", Floats{0.1f, 0.2f, 0.3f});

        // Vector<bool> argument
        cli.add<Bools>({"-f", "--flags"}, "List of flags", Bools{true, false, true});

        // Vector<string> argument
        cli.add<Strings>({"-t", "--tags"}, "List of tags", Strings{"alpha", "beta"});

        cli.parse();

        auto count = cli.get<int>("count");
        auto filename = cli.get<string>("filename");
        auto visualize = cli.get<bool>("show");
        auto ratio = cli.get<float>("ratio");
        auto ids = cli.get<Ints>("ids");
        auto scores = cli.get<Floats>("scores");
        auto flags = cli.get<Bools>("flags");
        auto tags = cli.get<Strings>("tags");
        auto role = cli.get<string>("role");
        auto email = cli.get<string>("email");

        cout << "Filename: " << filename << "\n";
        cout << "Account Role: " << role << "\n";
        cout << "Email: " << email << "\n";
        cout << "Count: " << count << "\n";
        cout << "Visualize: " << (visualize ? "true" : "false") << "\n";
        cout << "Ratio: " << ratio << "\n";
        cout << "IDs: ";
        for (auto v : ids) cout << v << " ";
        cout << "\nScores: ";
        for (auto v : scores) cout << v << " ";
        cout << "\nFlags: ";
        for (auto v : flags) cout << (v ? "true" : "false") << " ";
        cout << "\nTags: ";
        for (const auto& v : tags) cout << v << " ";
        cout << "\n";
    } catch (const std::exception& ex) {
        cerr << "Error: " << ex.what() << endl;
        return 1;
    }
}
