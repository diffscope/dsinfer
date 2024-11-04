#include <chrono>

#include <stdcorelib/system.h>
#include <stdcorelib/console.h>
#include <stdcorelib/vla.h>

#include <dsutils/phonemedictionary.h>

namespace cho = std::chrono;

int main(int /*argc*/, char * /*argv*/[]) {
    auto cmdline = stdc::System::commandLineArguments();
    if (cmdline.size() != 2) {
        stdc::u8println("Usage: %1 <dict>", stdc::System::applicationName());
        return 1;
    }

    {
        auto start_time = cho::high_resolution_clock::now();

        // Load file
        dsutils::PhonemeDictionary dicts[10];
        const auto &filepath = stdc::utf8ToPath(cmdline[1]);
        for (auto &dict : dicts) {
            if (!dict.load(filepath)) {
                stdc::u8println("Failed to read dictionary \"%1\".", filepath);
            }
        }

        auto end_time = cho::high_resolution_clock::now();
        auto duration = cho::duration_cast<cho::milliseconds>(end_time - start_time);

        stdc::u8println("Timeout: %1ms", duration.count());

        stdc::u8println("Press Enter to continue...");
        std::ignore = std::getchar();

        {
            int i = 0;
            auto it = dicts[0].get().begin();
            for (; i < 10; ++i, ++it) {
                VLA_NEW(std::string_view, values, 2);
                it->second.read(values, 2);
                stdc::u8println("Phoneme %1: %2 - %3 %4", i, it->first, values[0], values[1]);
            }
        }
    }
    return 0;
}