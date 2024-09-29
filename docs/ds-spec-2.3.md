# DiffSinger 数据格式与推理接口规范 2.3

> DiffSinger Data Format and Inference Interface Specification 2.3

此规范为 OpenVPI 为各种 AI 推理工具制定的标准，旨在为各种模型提供通用的组织结构与调用接口，使 AI 模型的分发与调用更为有序、规范。

本规范主要指导以下几种基础设施的开发：
1. 安装器（Installer）
2. 加载器（Loader）
3. 执行器（Executor）

## 1. 关于 Library

### 文件结构

本规范内，可分发的数据包的最小单位是 Library（库），是一个以`dslib`为扩展名的 ZIP 格式的压缩包。

压缩包内基本结构为：
```
+ xxx.dslib
  - desc.json
  - ...
```

Library 内多使用`json`作为声明文件，我们规定，声明文件中使用的相对路径的基路径是这个声明文件的在所目录。

#### 描述文件

`desc.json`是 Library 的描述文件，主要包括以下内容。

```json
{
    "id": "foo",
    "version": "1.0.0.0",
    "compatVersion": "0.0.0.0",
    "vendor": "someone",
    "copyright": "Copyright (C) someone",
    "description": "Some library",
    "readme": "assets/readme.txt",
    "url": "https://www.dummy.cn",
    "contributes": {
        "inferences": [
            {
                "id": "pitch",
                "class": "com.diffsinger.InferenceInterpreter.PitchInference",
                "configuration": "./inferences/pitch.json"
            },
            {
                "id": "variance",
                "class": "com.diffsinger.InferenceInterpreter.VarianceInference",
                "configuration": "./inferences/variance.json"
            }
        ],
        "singers": [
            {
                "id": "zhibin",
                "model": "diffsinger",
                "path": "./characters/zhibin.json"
            }
        ]
    },
    "dependencies": [
        {
            "id": "bar",
            "version": "1.0.0.0"
        }
    ]
}
```
+ 必选字段
    + `id`：唯一标识符
    + `version`：版本号，格式为`x.y[.z.w]`
+ 可选字段
    + `compatVersion`：兼容到的最低版本，如果为`0.0.0.0`表示向下兼容所有，如果与`version`相同则表示不向下兼容，缺省为不向下兼容
    + `vender`：提供者，可提供多语言
        ```json
        {
            "_": "someone",
            "zh_CN": "某人"
        }
        ```
    + `copyright`：版权信息，可为多语言
    + `description`：介绍文字，可为多语言
    + `readme`：放置介绍、许可证等信息的文本
    + `url`：网站
    + `contributes`：功能贡献列表，主要包含子模块
        + `inferences`：推理模块
            + `id`：推理模块 ID
            + `class`：推理类型
            + `configuration`：配置文件
        + `singers`：歌手模块
            + `id`：歌手 ID
            + `model`：歌手架构
            + `path`：歌手信息文件
    + `dependencies`：依赖的库
        + `id`：依赖库 ID
        + `version`：依赖库版本
        + `required`：是否为强制依赖，默认为`true`
    <!-- + `properties`：与加载器或安装器相关的属性
        + `accessory`：表示是其他 Library 的配件，当其所属的 Library 不存在时，可以自动删除，默认为`false`
        + `single`: 同`id`的 Library 只能加载同一个，默认为`false` -->

#### 依赖项

当前 AI 推理现状是，一个模型动辄超过 100MB 甚至 1GB，因此内存与显存是宝贵的，本规范使用了一种常见的方式缓解这个问题。

为了模块复用与增量更新，`dslib`引入了依赖机制。

- 模块复用：开发者要分发若干个`dslib`（假设为 A、B），但是它们之间存在一些可以复用的内容，为了节省存储时的硬盘资源以及推理时的内存资源，那么可以将这些内容独立到一个`dslib`（C）中，在 A 与 B 的描述文件中声明它们依赖 C 即可。

- 增量更新：开发者要分发一个`dslib`（A），A 中存在稳定与不稳定的部分，不稳定的部分每次更新都需要更改，为了节省网络资源，那么可以将不稳定的内容独立到一个`dslib`（B）中，每次更新时只需更新 B 即可。

### 安装与加载

#### 安装

对于`dslib`文件，安装就是在某个目录中解压它。本规范对这个目录没有任何要求。

#### 加载

由于`dslib`引入了依赖机制，因此一个`dslib`的加载流程中包含依赖查找。

- 对于一个`dslib`，当且仅当它的所有依赖项都能被加载，它才能被加载。
- 一个`dslib`在被成功加载之前，不进行后续对其内部其他文件的任何访问。
- 一个`dslib`的依赖项给定了所需依赖项的`id`与`version`，只有满足以下条件才能被视为合法依赖。
    - `id`与给定`id`一致
    - `version`大于等于给定`version`
    - `compatVersion`小于等于给定`version`

#### 简单方案

本节将给出一种简单的安装加载方案。

安装器将所有`dslib`安装到同一个目录（如`~/.diffsinger/lib`），所有被安装的`dslib`平铺在这个目录中。
```
+ ~/.diffsinger
  + lib
      + lib1
        - desc.json
        - ...
      + lib2
        - desc.json
        - ...
  - ...
```

加载器可由用户在启动时指定一个路径列表（如`~/.diffsinger/lib;~/lib1;~/lib2;~/lib3`），当加载器加载某个`dslib`并伴随着解析其依赖时，加载器将依次遍历这个列表，尝试每个路径，如果在这个路径中找到了符合条件的依赖则加载之，并按同样的方法解析下一个依赖，直到结束。

## 2. 模块

### Inference 模块

Inference 模块负责执行某一项参数的推理任务，承担了最底层、核心的工作。

#### 配置文件

```json
{
    "name": "Zhibin - Variance",
    "level": 1,
    "schema": {
        "predictions": [
            "breathness", "duration"
        ]
    },
    "configuration": {
        "hiddenSize": 512
    }
}
```
+ 必选字段
    + `level`: 推理解释器应选择的 API 版本
+ 可选字段
    + `name`: 推理模块名称，可为多语言，如为空则与`id`一致
    + `schema`: 输出参数的限制条件
    + `configuration`：配置信息

### Singer 模块

Singer 模块负责定义一个或若干个歌手的信息，以及其需要使用的推理库。

#### 声明文件

`singer.json`是 Singer 的信息声明文件，主要包括以下内容。

```json
{
    "name": "Zhibin",
    "avatar": "../assets/avatar.png",
    "background": "../assets/sprite.png",
    "demoAudio": "../assets/demo.wav",
    "imports": [
        "acoustic-1",
        "bar/pitch",
        {
            "id": "variance-A",
            "options": {
                "prediction": "duration"
            }
        }
    ],
    "configuration": {
        "dictionary": "../assets/dsdict.json"
    }
}
```
+ 可选字段
    + `name`: 歌手名称，可为多语言，如为空则与`id`一致
    + `imports`：歌手依赖的推理模块
        + `id`：依赖的推理模块 ID，如果是别的库的那么使用`lib[version]/id`的形式，`lib`与`version`可以省略
        + `options`：输出参数，需要符合对应的 API 版本以及推理模块的`schema`的限制
    + `avatar`：头像
    + `background`：可用于 SVS 编辑器显示的立绘背景
    + `demoAudio`：可用于 SVS 编辑器预览的声音
    + `configuration`：其他属性（根据`model`设置）
        + `dictionary`：歌手词典

#### 注意事项

- 每个歌手的预设所用到的`id`都必须在`desc.json`的`dependencies`中声明。

### 可扩展性

- 具有新功能的模型开发完成后，开发者为之起一个`class`名，再基于现有的推理程序开发一个与这种模型匹配的解释器，这样即可扩展推理功能。

- 非 DiffSinger 甚至非 AI 的开发者，如 UTAU、Vocaloid，亦可通过扩展`class`来支持其他引擎，可以使用混合 Library 将歌手信息与歌声采样放在同一个 Library 中。

## 3. 工具开发

下面介绍一种经典的工具套件，命名为`dsinfer-cli`。

### 功能贡献

- 校验
- 安装
- 命令行推理

#### 校验

```sh
dsinfer-cli stat <package>
```

如果是正确的包，则打印信息，否则报错。

#### 安装

```sh
dsinfer-cli install <package> [--path <path>]
```

用户需要先在特定目录（如`/.config/dsinfer`）创建一个名为`config.json`的配置文件，指定默认安装路径（如`/.config/dsinfer/packages`）。

安装成功后，安装工具将会执行校验、解压等操作，将其解压在安装路径的子目录中，并生成记忆文件，记录安装的包名和其功能信息。

#### 命令行推理

```sh
dsinfer-cli exec <singer> [--arg <key> <value>] [--paths <paths>]
```

推理工具将会在所有搜索路径中搜索含有歌手的包，将其元数据全部加载，并执行推理任务。

### 推理插件开发

在`plugins`中添加插件。

#### 推理解释器

创建派生于`InferenceInterpreter`的解释器类。

- `key`：返回对应的推理参数类型的`class`，如`com.diffsinger.InferenceInterpreter.PitchInference`
    ```c++
    const char *key() const override;
    ```
- `validate`：校验推理模块是否符合规范，以及使用某个推理模块的歌手模块是否指定了正确的参数
    ```c++
    bool validate(const InferenceSpec *spec, std::string *message) const override;
    bool validate(const InferenceSpec *spec, const JsonObject &importOptions,
                  std::string *message) const override;
    ```
- `create`：创建对应的推理任务类
    ```c++
    Inference *create(const InferenceSpec *spec, const JsonObject &options,
                      Error *error) const override;
    ```

#### 推理任务

创建派生于`Inference`的推理任务类。

- `initialize`：初始化推理任务，应当加载需要用到的模型
    ```c++
    bool initialize(const JsonObject &args, Error *error) override;
    ```
- `start`：开始推理任务，应当对输入的参数进行预处理，并构建推理图（异步）
    ```c++
    bool start(const JsonValue &input, Error *error) override;
    ```
- `stop`：立即停止推理任务（同步）
    ```c++
    bool stop() override;
    ```
- `state`：推理任务状态
    ```c++
    State state() const override;
    ```
- `result`：推理结果
    ```c++
    JsonValue result() const override;
    ```