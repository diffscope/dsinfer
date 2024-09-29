#include <iostream>
#include <filesystem>

#include <dsinfer/environment.h>
#include <dsinfer/inferenceregistry.h>
#include <dsinfer/format.h>

#include <syscmdline/command.h>
#include <syscmdline/parser.h>
#include <syscmdline/system.h>

#include "loadconfig.h"
#include "statusconfig.h"

namespace fs = std::filesystem;

namespace SCL = SysCmdLine;

namespace DS = dsinfer;

static int cmd_stat(const SCL::ParseResult &result) {
    return 0;
}

static int cmd_list(const SCL::ParseResult &result) {
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
    // DS::Error error;

    // // Configure environment
    // DS::Environment env;
    // env.addPluginPath("com.diffsinger.InferenceInterpreter", fs::current_path().parent_path() /
    // "lib" /
    //                                                           "plugins" / "dsinfer" /
    //                                                           "inferenceinterpreters");

    // fs::path libPath = fs::current_path() / "lib";
    // env.addLibraryPath(libPath);

    // auto driver = new DS::MyInferenceDriver();
    // if (!driver->initialize(
    //         {
    //             {"ep", "DML"},
    // },
    //         &error)) {
    //     printf("Error: %s\n", error.what());
    //     return -1;
    // }

    // // Set driver
    // auto inf_reg = env.registry(DS::ContributeSpec::Inference)->cast<DS::InferenceRegistry>();
    // inf_reg->setDriver(driver);

    // // Load library
    // auto lib1 = env.openLibrary(libPath / "zhibin-0.5.1.0", false, &error);
    // if (!lib1) {
    //     printf("Error: %s\n", error.what());
    //     return -1;
    // }

    // // Get inference
    // auto inf_spec =
    //     lib1->contribute(DS::ContributeSpec::Inference, "pitch")->cast<DS::InferenceSpec>();

    // auto inf1 = inf_spec->create({}, &error);
    // if (!inf1) {
    //     printf("Error: %s\n", error.what());
    //     return -1;
    // }

    // if (!inf1->initialize({}, &error)) {
    //     printf("Error: %s\n", error.what());
    //     return -1;
    // }

    // // Prepare input
    // DS::JsonObject input;
    // // ...

    // // Start inf1
    // if (!inf1->start(input, &error)) {
    //     printf("Error: %s\n", error.what());
    //     return -1;
    // }

    // while (inf1->state() == DS::Inference::Running) {
    //     _sleep(1000);
    // }

    // // Get result
    // auto result = inf1->result();

    // // Process result
    // // ...
    return 0;
}

int main(int argc, char *argv[]) {
    SCL::Command statCommand = [] {
        SCL::Command command("stat", "Display package status");
        command.addArguments({
            SCL::Argument("package", "Package identifier, format: id[version]"),
        });
        command.addOptions({
            SCL::Option("--paths", R"(Add searching paths)").arg(SCL::Argument("path").multi()),
        });
        command.setHandler(cmd_stat);
        return command;
    }();
    SCL::Command listCommand = [] {
        SCL::Command command("list", "List installed packages");
        command.addOptions({
            SCL::Option("--paths", R"(Add searching paths)").arg(SCL::Argument("path").multi()),
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
            SCL::Option("--paths", R"(Add searching paths)").arg(SCL::Argument("path").multi()),
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
            SCL::Option("--paths", R"(Add searching paths)").arg(SCL::Argument("path").multi()),
            SCL::Option("--driver", R"(Override default driver)").arg("id"),
            SCL::Option("--init", R"(Override default driver initialzing arguments)").arg("arg"),
        });
        command.setHandler(cmd_exec);
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
    });
    rootCommand.addVersionOption(TOOL_VERSION);
    rootCommand.addHelpOption(true, true);
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