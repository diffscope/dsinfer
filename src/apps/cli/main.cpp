#include <iostream>
#include <filesystem>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <thread>
#include <sstream>

#include <zlib.h>

#include <stdcorelib/format.h>
#include <stdcorelib/console.h>

#include <dsinfer/environment.h>
#include <dsinfer/inferenceregistry.h>
#include <dsinfer/singerregistry.h>
#include <dsinfer/log.h>

#include <syscmdline/command.h>
#include <syscmdline/parser.h>
#include <syscmdline/system.h>

#include "startupconfig.h"
#include "statusconfig.h"

namespace fs = std::filesystem;

namespace SCL = SysCmdLine;

namespace DS = dsinfer;

static void log_report_callback(int level, const char *category, const char *fmt, va_list args) {
    using namespace DS;
    using namespace stdc;

    auto t = std::time(nullptr);
    auto tm = std::localtime(&t);

    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    auto dts = ss.str();

    Console::Color foreground, background;
    if (level <= Log::Verbose) {
        foreground = Console::Default;
        background = Console::White;
    } else if (level <= Log::Information) {
        foreground = Console::Blue;
        background = foreground;
    } else if (level <= Log::Warning) {
        foreground = Console::Yellow;
        background = foreground;
    } else {
        foreground = Console::Red;
        background = foreground;
    }

    const char *sig = "I";
    switch (level) {
        case Log::Trace:
            sig = "T";
            break;
        case Log::Debug:
            sig = "D";
            break;
        case Log::Verbose:
            sig = "V";
            break;
        case Log::Warning:
            sig = "W";
            break;
        case Log::Critical:
            sig = "C";
            break;
        case Log::Fatal:
            sig = "F";
            break;
        default:
            break;
    }
    Console::printf(foreground, Console::Default, "[%s] %-15s", dts.c_str(), category);
    Console::printf(Console::Black, background, " %s ", sig);
    Console::printf(Console::Default, Console::Default, "  ");
    Console::vprintf(foreground, Console::Default, fmt, args);
}

struct Context {
    fs::path appDir;
    fs::path defaultPluginDir;
    fs::path startupConfigPath;

    cli::StartupConfig startupConfig;
    cli::StatusConfig statusConfig;

    DS::Environment env;
    DS::Log::Category logger = {"dsinfer-cli"};

    Context() {
        // Get basic directories
        appDir = stdc::utf8ToPath(SCL::appDirectory());
        defaultPluginDir =
            appDir.parent_path() / _TSTR("lib") / _TSTR("plugins") / _TSTR("dsinfer");

        // Set default plugin directories
        env.addPluginPath("com.diffsinger.InferenceDriver",
                          defaultPluginDir / _TSTR("inferencedrivers"));
        env.addPluginPath("com.diffsinger.InferenceInterpreter",
                          defaultPluginDir / _TSTR("inferenceinterpreters"));

        // Load startup config
        fs::path homeDir =
#ifdef WIN32
            _wgetenv(L"USERPROFILE")
#else
            getenv("HOME")
#endif
            ;

        const std::filesystem::path startupConfigDirs[] = {
            appDir,
            homeDir / _TSTR(".diffsinger"),
        };

        for (const auto &dir : std::as_const(startupConfigDirs)) {
            auto path = dir / _TSTR("dsinfer-conf.json");
            if (!startupConfig.load(path)) {
                continue;
            }
            logger.debug(R"(Successfully read user configuration "%1")", path);
            startupConfigPath = path;
            break;
        }
        if (startupConfigPath.empty()) {
            logger.debug(R"(Failed to find user configuration")");
        }

        // Add paths
        env.addLibraryPaths(startupConfig.paths);
    }

    template <class... Args>
    static inline void info(const std::string &format, Args &&...args) {
        using namespace stdc;
        Console::printf(Console::Default, Console::Default, "%s\n",
                        formatTextN(format, args...).c_str());
    }

    template <class... Args>
    static inline void warning(const std::string &format, Args &&...args) {
        using namespace stdc;
        Console::printf(Console::Yellow, Console::Default, "%s\n",
                        formatTextN(format, args...).c_str());
    }

    template <class... Args>
    static inline void critical(const std::string &format, Args &&...args) {
        using namespace stdc;
        Console::printf(Console::Red, Console::Default, "%s\n",
                        formatTextN(format, args...).c_str());
    }
};

static std::vector<fs::path> getCmdPaths(const SCL::ParseResult &result) {
    std::vector<std::filesystem::path> paths;
    auto pathsResult = result.option("--paths").values();
    for (const auto &item : std::as_const(pathsResult)) {
        auto path = fs::absolute(stdc::utf8ToPath(item.toString()));
        if (!fs::is_directory(path)) {
            continue;
        }
        path = fs::canonical(path);
        paths.emplace_back(path);
    }
    return paths;
}

static void updateLogger(const SCL::ParseResult &result) {
    auto opt = result.option("--debug");
    if (opt.isSet()) {
        int level = opt.value().toInt();
        switch (level) {
            case 1:
                DS::Log::setLevel(DS::Log::Verbose);
                break;
            case 2:
                DS::Log::setLevel(DS::Log::Debug);
                break;
            case 3:
                DS::Log::setLevel(DS::Log::Trace);
                break;
            default:
                break;
        }
    }
    DS::Log::setCallback(log_report_callback);
}

static fs::path searchPackage(const std::vector<fs::path> &paths, const std::string &id,
                              const DS::VersionNumber &version) {
    for (const auto &path : std::as_const(paths)) {
        auto statusConfigPath = path / _TSTR("status.json");
        cli::StatusConfig sc;
        if (!sc.load(statusConfigPath)) {
            continue;
        }

        for (const auto &pkg : std::as_const(sc.packages)) {
            if (pkg.id == id && pkg.version == version) {
                return path / pkg.relativeLocation;
            }
        }
    }
    return {};
}

static int cmd_stat(const SCL::ParseResult &result) {
    updateLogger(result);
    auto paths = getCmdPaths(result);
    const auto &idStr = result.value(0).toString();

    std::string pkgId;
    DS::VersionNumber pkgVersion;
    fs::path pkgPath;
    {
        auto identifier = DS::ContributeIdentifier::fromString(idStr);
        if (!identifier.library().empty() && !identifier.version().isEmpty() &&
            identifier.id().empty()) {
            pkgId = identifier.library();
            pkgVersion = identifier.version();
        } else if (auto path = stdc::utf8ToPath(idStr);
                   fs::is_regular_file(path / _TSTR("desc.json"))) {
            pkgPath = fs::canonical(path);
        } else {
            throw std::runtime_error(
                stdc::formatTextN(R"(invalid package identifier "%1")", idStr));
        }
    }

    Context ctx;
    auto &env = ctx.env;
    env.addLibraryPaths(paths);
    paths = env.libraryPaths();

    // Search package
    if (pkgPath.empty()) {
        pkgPath = searchPackage(paths, pkgId, pkgVersion);
        if (pkgPath.empty()) {
            throw std::runtime_error(stdc::formatTextN(R"(package "%1" not found)", idStr));
        }
    }

    // Try to load
    DS::LibrarySpec *lib;
    if (DS::Error error; !(lib = env.openLibrary(pkgPath, false, &error))) {
        throw std::runtime_error(
            stdc::formatTextN(R"(failed to open package "%1": %2)", pkgPath, error.message()));
    }

    Context::info("ID: %1", lib->id());
    Context::info("Version: %1", lib->version().toString());
    Context::info("CompatVersion: %1", lib->compatVersion().toString());
    if (!lib->description().isEmpty())
        Context::info("Description: %1", lib->description().text());
    if (!lib->vendor().isEmpty())
        Context::info("Vendor: %1", lib->vendor().text());
    if (!lib->copyright().isEmpty())
        Context::info("Copyright: %1", lib->copyright().text());
    if (!lib->url().empty())
        Context::info("Url: %1", lib->url());

    Context::info("Contributes:");
    const auto &inferences = lib->contributes(DS::ContributeSpec::Inference);
    if (!inferences.empty()) {
        Context::info("    Inferences:");
        for (int i = 0; i < inferences.size(); ++i) {
            auto inference = inferences[i]->cast<DS::InferenceSpec>();
            Context::info("        [%1] %2, %3, level=%4", i + 1, inference->id(),
                          inference->name().text(), inference->apiLevel());
        }
    }
    const auto &singers = lib->contributes(DS::ContributeSpec::Singer);
    if (!singers.empty()) {
        Context::info("    Singers:");
        for (int i = 0; i < singers.size(); ++i) {
            auto singer = singers[i]->cast<DS::SingerSpec>();
            Context::info("        [%1] %2, %3, model=%4", i + 1, singer->id(),
                          singer->name().text(), singer->model());
        }
    }

    const auto &deps = lib->dependencies();
    if (!deps.empty()) {
        Context::info("Dependencies:");
        for (int i = 0; i < deps.size(); ++i) {
            const auto &dep = deps[i];
            Context::info("    [%1] %2[%3]%4", i + 1, dep.id, dep.version.toString(),
                          dep.required ? ", required" : "");
        }
    }

    if (auto error = lib->error(); !error.ok()) {
        printf("\n");
        Context::warning(R"(Warning: failed to load package "%1": %2)", lib->path(),
                         error.message());
    }

    std::ignore = env.closeLibrary(lib);
    return 0;
}

static int cmd_list(const SCL::ParseResult &result) {
    updateLogger(result);
    auto paths = getCmdPaths(result);

    Context ctx;
    auto &env = ctx.env;
    env.addLibraryPaths(paths);
    paths = env.libraryPaths();

    struct PackageInfo {
        std::string id;
        DS::VersionNumber version;
        bool valid = true;
        bool operative = false;
        std::vector<std::string> singers;
    };

    std::vector<PackageInfo> infoList;
    for (const auto &path : std::as_const(paths)) {
        auto statusConfigPath = path / _TSTR("status.json");
        cli::StatusConfig sc;
        if (!sc.load(statusConfigPath)) {
            continue;
        }

        // Try load each library
        for (const auto &pkg : std::as_const(sc.packages)) {
            PackageInfo info;
            info.id = pkg.id;
            info.version = pkg.version;

            DS::Error error;
            auto lib = env.openLibrary(path / pkg.relativeLocation, false, &error);
            if (!lib) {
                info.valid = false;
                infoList.emplace_back(info);
                continue;
            }

            if (lib->error().ok()) {
                info.operative = true;
            }
            for (const auto &item : lib->contributes(DS::ContributeSpec::Singer)) {
                info.singers.emplace_back(item->cast<DS::SingerSpec>()->id());
            }
            env.closeLibrary(lib);
            infoList.emplace_back(info);
        }
    }

    // Print
    for (int i = 0; i < infoList.size(); ++i) {
        const auto &info = infoList[i];
        std::string line =
            stdc::formatTextN("[%1] %2[%3]", i + 1, info.id, info.version.toString());
        if (!info.valid) {
            Context::critical("%1 (invalid)", line);
            continue;
        }

        if (!info.singers.empty()) {
            line += "; singers: " + stdc::join(info.singers, ", ");
        }
        if (!info.operative) {
            Context::warning("%1 (inoperative)", line);
            continue;
        }
        Context::info("%1", line);
    }
    return 0;
}

static int cmd_install(const SCL::ParseResult &result) {
    updateLogger(result);
    return 0;
}

static int cmd_remove(const SCL::ParseResult &result) {
    updateLogger(result);
    return 0;
}

static int cmd_autoRemove(const SCL::ParseResult &result) {
    updateLogger(result);
    return 0;
}

static bool isApiLevelSupported(const std::string &cls, int level) {
    std::ignore = cls;
    return level == 1;
}

static int cmd_exec(const SCL::ParseResult &result) {
    updateLogger(result);
    auto paths = getCmdPaths(result);
    const auto &idStr = result.value(0).toString();
    const auto &input = result.value(1).toString();
    const auto &driverId = result.valueForOption("--driver").toString();
    const auto &driverInit = result.valueForOption("--init").toString();

    std::string pkgId;
    DS::VersionNumber pkgVersion;
    std::string singerId;
    {
        auto identifier = DS::ContributeIdentifier::fromString(idStr);
        if (!identifier.library().empty() && !identifier.version().isEmpty() &&
            !identifier.id().empty()) {
            pkgId = identifier.library();
            pkgVersion = identifier.version();
        } else {
            throw std::runtime_error(stdc::formatTextN(R"(invalid singer identifier "%1")", idStr));
        }
    }

    Context ctx;
    auto &env = ctx.env;
    for (const auto &item : paths) {
        env.addLibraryPath(item);
    }
    paths = env.libraryPaths();

    // Search package
    fs::path pkgPath = searchPackage(paths, pkgId, pkgVersion);
    if (pkgPath.empty()) {
        throw std::runtime_error(stdc::formatTextN(R"(package "%1" not found)", idStr));
    }

    auto inferenceReg = env.registry(DS::ContributeSpec::Inference)->cast<DS::InferenceRegistry>();
    auto singerReg = env.registry(DS::ContributeSpec::Singer)->cast<DS::SingerRegistry>();

    // Create driver
    const auto &realDriverId = driverId.empty() ? ctx.startupConfig.driver.id : driverId;
    auto driver = inferenceReg->createDriver(realDriverId.c_str());
    if (!driver) {
        throw std::runtime_error(stdc::formatTextN(R"(failed to load driver "%1")", realDriverId));
    }

    // Initialize driver
    {
        DS::Error error;
        if (!driver->initialize(driverInit.empty() ? DS::JsonValue::fromJson(driverInit, true)
                                                   : ctx.startupConfig.driver.init,
                                &error)) {
            throw std::runtime_error(stdc::formatTextN(R"(failed to initialize driver "%1": %2)",
                                                       realDriverId, error.message()));
        }
    }
    inferenceReg->setDriver(driver);

    // Load package
    DS::LibrarySpec *lib;

    if (DS::Error error; lib = env.openLibrary(pkgPath, false, &error), !lib) {
        throw std::runtime_error(
            stdc::formatTextN(R"(failed to open package "%1": %2)", pkgPath, error.message()));
    }
    if (auto error = lib->error(); !error.ok()) {
        throw std::runtime_error(
            stdc::formatTextN(R"(failed to load package "%1": %2)", pkgPath, error.message()));
    }

    auto singerSpec = lib->contribute(DS::ContributeSpec::Singer, singerId)->cast<DS::SingerSpec>();
    if (!singerSpec) {
        throw std::runtime_error(stdc::formatTextN(R"(singer "%1" not found in package "%2[%3]")",
                                                   singerId, pkgId, pkgVersion.toString()));
    }

    std::vector<DS::Inference *> inferences;
    if (DS::Error error; inferences = singerSpec->createInferences(&error), !error.ok()) {
        throw std::runtime_error(
            stdc::formatTextN(R"(failed to create inferences: %1)", error.message()));
    }

    // Check inference api levels
    for (const auto &inference : std::as_const(inferences)) {
        auto spec = inference->spec();
        if (!isApiLevelSupported(spec->className(), spec->apiLevel())) {
            throw std::runtime_error(stdc::formatTextN(R"(inference "%1" level %2 not supported)",
                                                       spec->className(), spec->apiLevel()));
        }
    }

    // Execute inferences
    if (false) {
        std::unique_ptr<DS::InferenceContext> ic(driver->createContext());
        DS::JsonObject args = {
            {"ctx", ic->id()},
        };
        for (const auto &inference : std::as_const(inferences)) {
            DS::Error error;
            if (!inference->start(args, &error)) {
                // TODO: error
            }

            // Wait for finished
            while (inference->state() == DS::Inference::Running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }
    return 0;
}

static int cmd_pack(const SCL::ParseResult &result) {
    updateLogger(result);

    const auto &pkgPath = stdc::utf8ToPath(result.value(0).toString());

    Context ctx;
    auto &env = ctx.env;

    // Try to load
    if (DS::Error error; !env.openLibrary(pkgPath, false, &error)) {
        throw std::runtime_error(
            stdc::formatTextN(R"(failed to open package "%1": %2)", pkgPath, error.message()));
    }

    // TODO: Calculate CRC
    printf("Compress: %p", compress);

    return 0;
}

int main(int argc, char *argv[]) {
    SCL::Command statCommand = [] {
        SCL::Command command("stat", "Display package status");
        command.addArguments({
            SCL::Argument("package", "Package path or identifier, format: id[version]"),
        });
        command.addOptions({
            SCL::Option("--paths", R"(Add library paths)").arg(SCL::Argument("path").multi()),
        });
        command.setHandler(cmd_stat);
        return command;
    }();
    SCL::Command listCommand = [] {
        SCL::Command command("list", "List installed packages");
        command.addOptions({
            SCL::Option("--paths", R"(Add library paths)").arg(SCL::Argument("path").multi()),
        });
        command.setHandler(cmd_list);
        return command;
    }();
    SCL::Command installCommand = [] {
        SCL::Command command("install", "Install packages");
        command.addArguments({
            SCL::Argument("packages", "Package paths").multi(),
        });
        command.addOptions({
            SCL::Option("--path", R"(Override default package path)").arg(SCL::Argument("path")),
        });
        command.setHandler(cmd_install);
        return command;
    }();
    SCL::Command removeCommand = [] {
        SCL::Command command("remove", "Remove packages");
        command.addArguments({
            SCL::Argument("packages", "Package paths").multi(),
        });
        command.addOptions({
            SCL::Option("--path", R"(Override default package path)").arg(SCL::Argument("path")),
        });
        command.setHandler(cmd_remove);
        return command;
    }();
    SCL::Command autoRemoveCommand = [] {
        SCL::Command command("autoremove", "Remove unused packages automatically");
        command.addOptions({
            SCL::Option("--paths", R"(Add library paths)").arg(SCL::Argument("path").multi()),
        });
        command.setHandler(cmd_autoRemove);
        return command;
    }();
    SCL::Command execCommand = [] {
        SCL::Command command("exec", "Execute an inference task");
        command.addArguments({
            SCL::Argument("singer", "Singer identifier, format: id[version]/sid"),
            SCL::Argument("input", "Input arguments"),
        });
        command.addOptions({
            SCL::Option("--paths", R"(Add library paths)").arg(SCL::Argument("path").multi()),
            SCL::Option("--driver", R"(Override default driver)").arg("id"),
            SCL::Option("--init", R"(Override default driver initialzing arguments)").arg("arg"),
        });
        command.setHandler(cmd_exec);
        return command;
    }();
    SCL::Command packCommand = [] {
        SCL::Command command("pack", "Make DiffSinger package");
        command.addArguments({
            SCL::Argument("dir", "Directory containing package files"),
            SCL::Argument("output", "Output file name").required(false),
        });
        command.addOptions({
            SCL::Option("-y", "Force overwrite output file"),
        });
        command.setHandler(cmd_pack);
        return command;
    }();

    SCL::Command rootCommand(SCL::appName(), "DiffSinger package manager and inference tool.");
    rootCommand.addCommands({
        statCommand,
        listCommand,
        installCommand,
        removeCommand,
        autoRemoveCommand,
        execCommand,
        packCommand,
    });
    SCL::Option debugOption = []() {
        SCL::Option option("--debug", "Specify debug level: 0~3");
        option.addArgument(SCL::Argument("level").expect({
            0,
            1,
            2,
            3,
        }));
        option.setGlobal(true);
        option.setRole(SCL::Option::Debug);
        return option;
    }();
    rootCommand.addOption(debugOption);
    rootCommand.addVersionOption(TOOL_VERSION);
    rootCommand.addHelpOption(false, true);
    rootCommand.setHandler([](const SCL::ParseResult &result) -> int {
        result.showHelpText();
        return 0;
    });

    SCL::Parser parser(rootCommand);
    parser.setPrologue(TOOL_DESC);
    parser.setDisplayOptions(SCL::Parser::AlignAllCatalogues);

    int ret;
    try {
#ifdef _WIN32
        std::ignore = argc;
        std::ignore = argv;
        ret = parser.invoke(SCL::commandLineArguments());
#else
        ret = parser.invoke(argc, argv);
#endif
    } catch (const std::exception &e) {
        std::string msg = e.what();

#ifdef _WIN32
        if (typeid(e) == typeid(fs::filesystem_error)) {
            msg = stdc::ansiToUtf8(e.what());
        }
#endif

        SCL::u8debug(SCL::MT_Critical, true, "Error: %s\n", msg.c_str());
        ret = -1;
    }
    return ret;
}