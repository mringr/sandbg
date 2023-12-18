//
// Created by Madhav Ramesh on 12/18/23.
//

#ifndef HELPERS_HPP
#define HELPERS_HPP
#include <vector>
#include <string>
#include <sstream>

class Helpers
{
    public:
        static std::vector<std::string> split (const std::string& line, const char delimiter) {
            std::vector<std::string> out {};
            std::stringstream ss {line};
            std::string token;

            while( std::getline(ss, token, delimiter)) {
                out.push_back(token);
            }
            return out;
        }

        static bool is_prefix(const std::string& s, const std::string& of) {
            if(s.size() > of.size()) return false;
            return std::equal(s.begin(), s.end(), of.begin());
        }

};

#endif //HELPERS_HPP
