#ifndef SIMPLEVAREXP_H
#define SIMPLEVAREXP_H

#include <string>
#include <unordered_map>

namespace dsinfer {

    class SimpleVarExp {
    public:
        SimpleVarExp();
        explicit SimpleVarExp(const std::string &pattern);

        std::string Pattern;
        std::unordered_map<std::string, std::string> Variables;

    public:
        std::string parse(const std::string &exp) const;
    };

}

#endif // SIMPLEVAREXP_H
