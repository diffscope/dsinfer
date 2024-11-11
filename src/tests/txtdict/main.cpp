#include <chrono>

#include <stdcorelib/system.h>
#include <stdcorelib/console.h>
#include <stdcorelib/vla.h>
#include <stdcorelib/path.h>

#include <dsutils/phonemedictionary.h>

namespace cho = std::chrono;

int main(int /*argc*/, char * /*argv*/[]) {
    auto cmdline = stdc::system::command_line_arguments();
    if (cmdline.size() < 2) {
        stdc::u8println("Usage: %1 <dict> [count]", stdc::system::application_name());
        return 1;
    }

    {
        const auto &filepath = stdc::path::from_utf8(cmdline[1]);
        int len = cmdline.size() >= 3 ? std::stoi(cmdline.at(2)) : 1;

        auto start_time = cho::high_resolution_clock::now();

        // Load file
        stdc::vlarray<dsutils::PhonemeDictionary, 1024> dicts(len);
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

        dicts[0].print_front(10);
    }
    return 0;
}