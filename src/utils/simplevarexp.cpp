#include "simplevarexp.h"

#include <regex>

namespace dsinfer {

    static const char Slash = '/';

    static const char DefaultPattern[] = R"(\$\{(\w+)\})";

    SimpleVarExp::SimpleVarExp() : SimpleVarExp(DefaultPattern) {
    }

    SimpleVarExp::SimpleVarExp(const std::string &pattern) : Pattern(pattern) {
    }

    static std::string dfs(std::string s, const std::regex &reg,
                           const std::unordered_map<std::string, std::string> &vars,
                           bool recursive = true) {
        std::smatch match;
        bool hasMatch = false;
        std::string::const_iterator searchStart(s.cbegin());

        while (std::regex_search(searchStart, s.cend(), match, reg)) {
            hasMatch = true;

            const auto &name = match[1].str();
            std::string val;
            auto it = vars.find(name);
            if (it == vars.end()) {
                val = name;
            } else {
                val = it->second;
            }

            s.replace(match.position(0), match[0].length(), val);
            searchStart = s.begin() + match.position(0);
        }
        if (!hasMatch) {
            return s;
        }
        return dfs(s, reg, vars, recursive);
    }

    std::string SimpleVarExp::parse(const std::string &exp) const {
        return dfs(exp, std::regex(Pattern), Variables);
    }

}