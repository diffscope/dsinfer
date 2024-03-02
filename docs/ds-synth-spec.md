# DiffScope 推理规范

本规范是 DiffSinger 歌声合成流程的一种实现，其包括编辑器、合成管理器、推理中间件与底层推理库四部分。

## 概念介绍

### 执行体

+ 编辑器：歌声合成前端，DiffScope 或其他支持本规范的编辑器；
+ 合成管理器：也称为合成引擎（下称管理器），负责声库的管理以及合成任务的调度；
+ 推理中间件：模型抽象层，即本项目`dsinfer`，对上提供以类 MIDI 格式为输入的合成接口，向下输出张量信息到推理库；
+ 底层推理库：目前定为 OnnxRuntime；

### 数据

+ 内置模型：由合管理器自带的一组全局模型，如声码器等；
+ 声库：包括声学、唱法等模型在内的歌声数据库，称为`database`，可以从管理器中安装或卸载；
+ 特性：声库所支持的参数，在声库中一般表现为一个模型（或者一个主模型与若干个附属模型，附属模型被视为资源），在文件系统中的静态数据称为`feature`，推理时在内存中的映像称为`inference`
+ 模型：模型文件，推理时在内存中的映像称为`session`

## 声库规范

本方案中的声库规范对用户而言相当自由，只需要在根目录中

### 目录结构

```
+ dummy
    + assets
        + avatar.png
        + dict.yaml
        + license.txt
    + features
        + acoustic
            + feat.json
            + model.onnx
        + duration
            + feat.json
            + model.onnx
        + pitch
            + feat.json
            + model.onnx
        + shared
            + linguistic.onnx
    + database.json
```

+ `features`：推荐在此存放模型，每个子目录包括一组模型与其对应的配置信息，将声库私有且需要复用的文件放在`shared`目录内；
+ `assets`：推荐存放与声库信息相关的文件；
+ `database.json`：声库描述文件（名称固定）
    ```json
    {
        "id": "dummy-12138",
        "version": "1.0.0",
        "name": "dummy",
        "vendor": "someone",
        "copyright": "someone",
        "description": "Some voice database",
        "url": "https://www.dummy.cn",
        "features": [
            "features/acoustic/feat.json",
            "features/duration/feat.json",
            "features/pitch/feat.json"
        ],
        "characters": [
            {
                "name": "Some singer",
                "avatar": "assets/avatar.png",
                "background": "assets/sprite.png",
                "demoAudio": "assets/demo.wav"
            }
        ],
        "env": {
            "SAMPLE_ENV_VAR": "sample"
        }
    }
    ```
    + 必选字段
        + `id`：声库的唯一标识符
        + `version`：声库版本号
    + 可选字段
        + `name`：声库名
        + `vender`：声库提供方
        + `copyright`：声库版权信息
        + `description`：声库介绍
        + `url`：声库网站
        + `features`：声库支持特性
        + `characters`：声库角色信息
        + `env`：自定义环境变量
    + 声库内置环境变量
        + `DATABASE_ID`
        + `DATABASE_VERSION`
        + `DATABASE_NAME`
        + `DATABASE_VENDOR`
        + `DATABASE_COPYRIGHT`
        + `DATABASE_ROOT_FOLDER`
    + 可添加其他字段

+ `feat.json`：模型描述文件（名称可改）
    ```json
    {
        "id": "dummy-12138-pitch",
        "type": "pitch",
        "level": 1,
        "version": "${DATABASE_VERSION}",
        "path": "model.onnx",
        "arguments": {
            "linguistic": "${DATABASE_ROOT_FOLDER}/features/shared/linguist.onnx"
        },
        "dependencies": {
            "dep1": {
                "id": "dummy-12138-model1",
                "level": 1
            }
        }
    }
    ```
    + 必选字段
        + `id`：特性的唯一标识符，不同声库的特性`id`可以重复
        + `type`：特性类型，目前有`acoustic`、`pitch`、`duration`、`variance`
        + `level`：特性参数集等级
        + `version`：特性版本
    + 可选字段
        + `path`: 模型路径
        + `arguments`：支持的参数
        + `dependencies`：依赖的其他特性
    + 可添加其他字段

## 合成管理器

管理器主要管理合成任务与已安装的声库，以直接链接或子进程或网络服务的形式为编辑器提供歌声渲染的支持。

### 主要功能

+ 启动时根据配置信息加载推荐的内置模型
+ 实时监测硬件资源信息，如显存与内存占用、GPU 使用率等信息
+ 以模型为单位，实时与`dsinfer`同步本地的声库与内置模型的配置信息
+ 在特殊的时间点（如接收到任务、硬件资源条件变化等），通知`dsinfer`加载或释放对应的模型
+ 维护任务队列，检查任务是否合法
+ 维护内置模型与声库的内部目录结构，对管理员提供安装、卸载的功能，对用户提供推理服务，可能涉及的技术方案有引用计数、延迟卸载等，对每个模型进行状态标记，并使用日志机制来保证一致性

## 推理中间件

`dsinfer`是 DiffScope 管理器与 OnnxRuntime 底层推理库的中间件，负责加载、释放模型与执行推理任务，将与模型相关的逻辑尽可能实现，以降低管理器的开发难度，使管理器更专注于任务调度。

`dsinfer`使用 C++ 编写，并提供 C 的接口，管理器应以库链接的形式调用`dsinfer`。

### 初始化

+ OnnxRuntime 支持 CPU、Cuda、DirectML（仅 Windows）等执行器（ExecutionProvider，下称 ep），不同 ep 的代码在不同的`onnxruntime`动态库（运行时）中，`dsinfer`库载入后，需要选择一个执行器并提供对应的运行时所在的路径进行初始化，`dsinfer`将使用动态加载的方式使用之。

+ 初始化完成后，`onnxruntime`动态库将始终在内存中，直到与`dsinfer`的进程退出，因此运行期切换 ep 是不支持的。

管理器需要实时将所有模型的配置信息与`dsinfer`进行同步。

### 加载与释放模型

+ 管理器以特性为单位向`dsinfer`发起加载模型的请求，加载时提供特性的`id`。由于特性之间可能存在依赖关系，因此`dsinfer`将自动维护依赖关系与引用计数，如果缺少依赖将会加载失败。同理，管理器也应使用`id`向`dsinfer`发起释放模型的请求，当模型仍在推理时，释放请求会失败。

+ 一个特性被加载到内存时，将会加载其需要用到的所有模型，即创建一个`inference`与若干个`session`，不同的`inference`之间可以共享`session`，每个`inference`与`session`都有各自的引用计数。

+ 一个`inference`将维护一条自己的`session`链，在其推理任务进行时尽可能减少数据在内存与显存中的拷贝。

+ 对每一种`inference`，`dsinfer`将按照特性配置文件中`type`字段指定的类型选择对应的调用约定进行后续的推理。

### 推理

推理过程在其他线程中执行，可传入回调函数通知结束以避免轮询，需要调用者保证操作的原子性。