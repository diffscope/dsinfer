#include <chrono>
#include <string_view>

#include <stdcorelib/system.h>
#include <stdcorelib/console.h>
#include <stdcorelib/vla.h>

#include <dsutils/phonemedictionary.h>

namespace cho = std::chrono;

static std::string join(std::string_view v[], int size, const std::string_view &delimiter) {
    if (size == 0) {
        return {};
    }
    std::string res;
    for (int i = 0; i < size - 1; ++i) {
        res.append(v[i]);
        res.append(delimiter);
    }
    res.append(v[size - 1]);
    return res;
}

int main(int /*argc*/, char * /*argv*/[]) {
    auto cmdline = stdc::System::commandLineArguments();
    if (cmdline.size() < 2) {
        stdc::u8println("Usage: %1 <dict> [count]", stdc::System::applicationName());
        return 1;
    }

    {
        const auto &filepath = stdc::utf8ToPath(cmdline[1]);
        int len = cmdline.size() >= 3 ? std::stoi(cmdline.at(2)) : 1;

        auto start_time = cho::high_resolution_clock::now();

        // Load file
        stdc::VarLengthArray<dsutils::PhonemeDictionary, 1024> dicts(len);
        for (auto &dict : dicts) {
            if (!dict.load(filepath)) {
                stdc::u8println("Failed to read dictionary \"%1\".", filepath);
                return EXIT_FAILURE;
            }
        }

        auto end_time = cho::high_resolution_clock::now();
        auto duration = cho::duration_cast<cho::milliseconds>(end_time - start_time);

        stdc::u8println("Timeout: %1ms", duration.count());

        stdc::u8println("Press Enter to continue...");
        std::ignore = std::getchar();

        {
            int i = 0;
            auto &dict = dicts[0];
            auto it = dict.get().begin();
            for (; i < 10; ++i, ++it) {
                stdc::VarLengthArray<std::string_view> values(it->second.count);
                dict.readEntry(it->second, values.data());
                stdc::u8println("Phoneme %1: %2 - %3", i, it->first,
                                join(values.data(), values.size(), " ").c_str());
            }
        }
    }
    return 0;
}