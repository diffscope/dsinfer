#include <iostream>
#include <filesystem>

#include <dsinfer/environment.h>
#include <dsinfer/inferenceregistry.h>
#include <dsinfer/singerregistry.h>
#include <dsinfer/format.h>

#include <syscmdline/command.h>
#include <syscmdline/parser.h>
#include <syscmdline/system.h>

#include "loadconfig.h"
#include "statusconfig.h"

namespace fs = std::filesystem;

namespace SCL = SysCmdLine;

namespace DS = dsinfer;

struct Context {
    fs::path appDir;
    fs::path defaultPluginDir;
    fs::path loadConfigPath;

    DS::Environment env;
    LoadConfig loadConfig;
    StatusConfig statusConfig;

    Context() {
        // Get basic directories
        appDir = DS::pathFromString(SCL::appDirectory());
        defaultPluginDir =
            appDir.parent_path() / _TSTR("lib") / _TSTR("plugins") / _TSTR("dsinfer");

        fs::path homeDir =
#ifdef WIN32
            _wgetenv(L"USERPROFILE")
#else
            getenv("HOME")
#endif
            ;
        loadConfigPath = homeDir / _TSTR(".diffsinger") / _TSTR("dsinfer.json");

        // Set default plugin directories
        env.addPluginPath("com.diffsinger.InferenceDriver",
                          defaultPluginDir / _TSTR("inferencedrivers"));
        env.addPluginPath("com.diffsinger.InferenceInterpreter",
                          defaultPluginDir / _TSTR("inferenceinterpreters"));

        // Load config
        std::ignore = loadConfig.load(loadConfigPath);

        // Add paths
        env.addLibraryPaths(loadConfig.paths);
    }
};

static std::vector<fs::path> getCmdPaths(const SCL::ParseResult &result) {
    std::vector<std::filesystem::path> paths;
    auto pathsResult = result.option("--paths").values();
    for (const auto &item : std::as_const(pathsResult)) {
        auto path = fs::absolute(DS::pathFromString(item.toString()));
        if (!fs::is_directory(path)) {
            continue;
        }
        path = fs::canonical(path);
        paths.emplace_back(path);
    }
    return paths;
}

static fs::path searchPackage(const std::vector<fs::path> &paths, const std::string &id,
                              const DS::VersionNumber &version) {
    for (const auto &path : std::as_const(paths)) {
        auto statusConfigPath = path / _TSTR("status.json");
        StatusConfig sc;
        if (!sc.load(statusConfigPath)) {
            continue;
        }

        for (const auto &pkg : std::as_const(sc.packages)) {
            if (pkg.id == id && pkg.version == version) {
                return path / pkg.path;
            }
        }
    }
    return {};
}

static int cmd_stat(const SCL::ParseResult &result) {
    auto paths = getCmdPaths(result);
    const auto &idStr = result.value(0).toString();

    std::string pkgId;
    DS::VersionNumber pkgVersion;
    {
        auto identifier = DS::ContributeIdentifier::fromString(idStr);
        if (!identifier.library().empty() && !identifier.version().isEmpty() &&
            identifier.id().empty()) {
            pkgId = identifier.library();
            pkgVersion = identifier.version();
        } else {
            throw std::runtime_error(DS::formatTextN(R"(invalid package identifier "%1")", idStr));
        }
    }

    Context ctx;
    auto &env = ctx.env;
    env.addLibraryPaths(paths);
    paths = env.libraryPaths();

    // Search package
    fs::path pkgPath = searchPackage(paths, pkgId, pkgVersion);
    if (pkgPath.empty()) {
        throw std::runtime_error(DS::formatTextN(R"(package "%1" not found)", idStr));
    }

    // Try to load
    DS::LibrarySpec *lib;
    {
        DS::Error error;
        lib = env.openLibrary(pkgPath, false, &error);
        if (!lib) {
            throw std::runtime_error(
                DS::formatTextN(R"(failed to open package "%1": %2)", pkgPath, error.message()));
        }
    }

    SCL::u8printf("ID: %s\n", lib->id().data());
    SCL::u8printf("Version: %s\n", lib->version().toString().data());
    SCL::u8printf("CompatVersion: %s\n", lib->compatVersion().toString().data());
    if (!lib->description().isEmpty())
        SCL::u8printf("Description: %s\n", lib->description().text().data());
    if (!lib->vendor().isEmpty())
        SCL::u8printf("Vendor: %s\n", lib->vendor().text().data());
    if (!lib->copyright().isEmpty())
        SCL::u8printf("Copyright: %s\n", lib->copyright().text().data());
    if (!lib->url().empty())
        SCL::u8printf("Url: %s\n", lib->url().data());

    SCL::u8printf("Contributes:\n");
    const auto &inferences = lib->contributes(DS::ContributeSpec::Inference);
    if (!inferences.empty()) {
        SCL::u8printf("    Inferences:\n");
        for (int i = 0; i < inferences.size(); ++i) {
            auto inference = static_cast<DS::InferenceSpec *>(inferences[i]);
            SCL::u8printf("        [%d] %s, %s, level=%d\n", i + 1, inference->id().data(),
                          inference->name().text().data(), inference->apiLevel());
        }
    }
    const auto &singers = lib->contributes(DS::ContributeSpec::Singer);
    if (!singers.empty()) {
        SCL::u8printf("    Singers:\n");
        for (int i = 0; i < singers.size(); ++i) {
            auto singer = static_cast<DS::SingerSpec *>(singers[i]);
            SCL::u8printf("        [%d] %s, %s, model=%s\n", i + 1, singer->id().data(),
                          singer->name().text().data(), singer->model().data());
        }
    }

    const auto &deps = lib->dependencies();
    if (!deps.empty()) {
        SCL::u8printf("Dependencies:\n");
        for (int i = 0; i < deps.size(); ++i) {
            const auto &dep = deps[i];
            SCL::u8printf("    [%d] %s[%s]%s\n", i + 1, dep.id.data(),
                          dep.version.toString().data(), dep.required ? ", required" : "");
        }
    }

    if (auto error = lib->error(); !error.ok()) {
        SCL::u8printf("\n");
        SCL::u8debug(SCL::MT_Warning, false, "Warning: failed to load package: %s\n", error.what());
    }

    std::ignore = env.closeLibrary(lib);
    return 0;
}

static int cmd_list(const SCL::ParseResult &result) {
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
        StatusConfig sc;
        if (!sc.load(statusConfigPath)) {
            continue;
        }
        for (const auto &pkg : std::as_const(sc.packages)) {
            PackageInfo info;
            info.id = pkg.id;
            info.version = pkg.version;

            DS::Error error;
            auto lib = env.openLibrary(path / pkg.path, false, &error);
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
    for (int i = 0; i < infoList.size(); ++i) {
        const auto &info = infoList[i];
        std::string line = DS::formatTextN("[%1] %2[%3]", i + 1, info.id, info.version.toString());
        if (!info.valid) {
            SCL::u8debug(SCL::MT_Critical, true, "%s (invalid)\n", line.data());
            continue;
        }

        if (!info.singers.empty()) {
            line += "; singers: " + DS::join(info.singers, ", ");
        }
        if (!info.operative) {
            SCL::u8debug(SCL::MT_Warning, false, "%s (not work)\n", line.data());
            continue;
        }
        SCL::u8printf("%s\n", line.data());
    }
    return 0;
}

static int cmd_install(const SCL::ParseResult &result) {
    return 0;
}

static int cmd_remove(const SCL::ParseResult &result) {
    return 0;
}

static int cmd_autoRemove(const SCL::ParseResult &result) {
    return 0;
}

static int cmd_exec(const SCL::ParseResult &result) {
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
            throw std::runtime_error(DS::formatTextN(R"(invalid singer identifier "%1")", idStr));
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
        throw std::runtime_error(DS::formatTextN(R"(package "%1" not found)", idStr));
    }

    auto inferenceReg = env.registry(DS::ContributeSpec::Inference)->cast<DS::InferenceRegistry>();
    auto singerReg = env.registry(DS::ContributeSpec::Singer)->cast<DS::SingerRegistry>();

    // Create driver
    const auto &realDriverId = driverId.empty() ? ctx.loadConfig.driver.id : driverId;
    auto driver = inferenceReg->createDriver(realDriverId.data());
    if (!driver) {
        throw std::runtime_error(DS::formatTextN(R"(failed to load driver "%1")", realDriverId));
    }

    // Initialize driver
    {
        DS::Error error;
        if (!driver->initialize(driverInit.empty() ? DS::JsonValue::fromJson(driverInit)
                                                   : ctx.loadConfig.driver.init,
                                &error)) {
            throw std::runtime_error(DS::formatTextN(R"(failed to initialize driver "%1": %2)",
                                                     realDriverId, error.message()));
        }
    }
    inferenceReg->setDriver(driver);

    // Load package
    DS::LibrarySpec *lib;
    {
        DS::Error error;
        lib = env.openLibrary(pkgPath, false, &error);
        if (!lib) {
            throw std::runtime_error(
                DS::formatTextN(R"(failed to open package "%1": %2)", pkgPath, error.message()));
        }
    }
    if (auto error = lib->error(); !error.ok()) {
        throw std::runtime_error(
            DS::formatTextN(R"(failed to load package "%1": %2)", pkgPath, error.message()));
    }

    auto singerSpec = lib->contribute(DS::ContributeSpec::Singer, singerId)->cast<DS::SingerSpec>();
    if (!singerSpec) {
        throw std::runtime_error(DS::formatTextN(R"(singer "%1" not found in package "%2[%3]")",
                                                 singerId, pkgId, pkgVersion.toString()));
    }

    std::vector<DS::Inference *> inferences;
    {
        DS::Error error;
        inferences = singerSpec->createInferences(&error);
        if (!error.ok()) {
            throw std::runtime_error(
                DS::formatTextN(R"(failed to create inferences: %1)", error.message()));
        }
    }

    // Handle inferences
    (void) inferences;

    return 0;
}

static int cmd_pack(const SCL::ParseResult &result) {
    return 0;
}

int main(int argc, char *argv[]) {
    SCL::Command statCommand = [] {
        SCL::Command command("stat", "Display package status");
        command.addArguments({
            SCL::Argument("package", "Package identifier, format: id[version]"),
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
        command.addArguments({
            SCL::Argument("packages", "Package paths").multi(),
        });
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
            msg = DS::ansiToUtf8(e.what());
        }
#endif

        SCL::u8debug(SCL::MT_Critical, true, "Error: %s\n", msg.data());
        ret = -1;
    }
    return ret;
}