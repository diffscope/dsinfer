#ifndef SINGERCONFIGURATION_H
#define SINGERCONFIGURATION_H

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <dsinfer/displaytext.h>

#include <dsutils/dsutilsglobal.h>

namespace dsutils {

    class DSUTILS_EXPORT SingerConfiguration {
    public:
        //     struct Speaker {
        //         std::string id;
        //         dsinfer::DisplayText name;
        //         std::pair<std::string, std::string> toneRanges;
        //     };

        //     struct LanguageInfo {
        //         std::string id;
        //         dsinfer::DisplayText name;
        //         std::string g2pId;
        //         std::filesystem::path dictPath;
        //     };

        //     SingerConfiguration();

        // public:
        //     int version() const;
        //     const std::vector<Speaker> &speakers() const;
        //     const std::vector<LanguageInfo> &languages() const;
        //     std::string defaultLanguageId() const;

        // protected:
        //     class Impl;
        //     std::shared_ptr<Impl> _impl;
    };

}

#endif // SINGERCONFIGURATION_H
