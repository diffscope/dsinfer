# DiffSinger 数据格式与推理接口规范 2.3

> DiffSinger Data Format and Inference Interface Specification 2.3

此规范为 OpenVPI 为各种 AI 推理工具制定的标准，旨在为各种模型提供通用的组织结构与调用接口，使 AI 模型的分发与调用更为有序、规范。

本规范主要指导以下几种基础设施的开发：
1. 安装器（Installer）
2. 加载器（Loader）
3. 执行器（Executor）

## 1. 关于 Library（亦称 Package）

### 文件结构

本规范内，可分发的数据包的最小单位是 Library（库），是一个以`dspk`为扩展名的 ZIP 格式的压缩包。

压缩包内基本结构为：
```
+ xxx.dspk
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
    "url": "https://www.example.com",
    "contributes": {
        "inferences": [
            {
                "id": "pitch",
                "class": "com.diffsinger.InferenceInterpreter.PitchInference",
                "configuration": "./inferences/pitch/config.json"
            },
            {
                "id": "variance",
                "class": "com.diffsinger.InferenceInterpreter.VarianceInference",
                "configuration": "./inferences/variance/config.json"
            }
        ],
        "singers": [
            {
                "id": "zhibin",
                "model": "diffsinger",
                "path": "./characters/zhibin/config.json"
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
    + `id`：唯一标识符，不准出现以下字符`/\[]:;'"`
    + `version`：版本号，格式为`x.y[.z.w]`
+ 可选字段
    + `compatVersion`：兼容到的最低版本，如果为`0.0.0.0`表示向下兼容所有，如果与`version`相同则表示不向下兼容，缺省为不向下兼容
    + `vender`：提供者，可提供多语言，多语言形式如下（`_`指定首选名称）
        ```json
        {
            "_": "Tadokoro Koji",
            "zh_CN": "李田所",
            "ja_JP": "田所浩二"
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

为了模块复用与增量更新，`dspk`引入了依赖机制。

- 模块复用：开发者要分发若干个`dspk`（假设为 A、B），但是它们之间存在一些可以复用的内容，为了节省存储时的硬盘资源以及推理时的内存资源，那么可以将这些内容独立到一个`dspk`（C）中，在 A 与 B 的描述文件中声明它们依赖 C 即可。

- 增量更新：开发者要分发一个`dspk`（A），A 中存在稳定与不稳定的部分，不稳定的部分每次更新都需要更改，为了节省网络资源，那么可以将不稳定的内容独立到一个`dspk`（B）中，每次更新时只需更新 B 即可。

### 安装与加载

#### 安装

对于`dspk`文件，安装就是在某个目录中解压它。本规范对这个目录没有任何要求。

#### 加载

由于`dspk`引入了依赖机制，因此一个`dspk`的加载流程中包含依赖查找。

- 对于一个`dspk`，当且仅当它的所有依赖项都能被加载，它才能被加载。
- 一个`dspk`在被成功加载之前，不进行后续对其内部其他文件的任何访问。
- 一个`dspk`的依赖项给定了所需依赖项的`id`与`version`，只有满足以下条件才能被视为合法依赖。
    - `id`与给定`id`一致
    - `version`大于等于给定`version`
    - `compatVersion`小于等于给定`version`
- 一个`dspk`将在其依赖项全部加载后进入初始化，初始化每一步都成功后即为成功加载。

#### 简单方案

本节将给出一种简单的安装加载方案。

安装器将所有`dspk`安装到同一个目录（如`~/.diffsinger/lib`），所有被安装的`dspk`平铺在这个目录中。
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

加载器可由用户在启动时指定一个路径列表（如`~/.diffsinger/lib;~/lib1;~/lib2;~/lib3`），当加载器加载某个`dspk`并伴随着解析其依赖时，加载器将依次遍历这个列表，尝试每个路径，如果在这个路径中找到了符合条件的依赖则加载之，并按同样的方法解析下一个依赖，直到结束。

## 2. 模块

### Inference 模块

Inference 模块负责执行某一项参数的推理任务，承担了最底层、核心的工作。

#### 配置文件

```json
{
    "$version": "1.0",
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
    + `$version`：文件格式版本，当前固定为`1.0`
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
    "$version": "1.0",
    "name": "Zhibin",
    "avatar": "../assets/avatar.png",
    "background": "../assets/sprite.png",
    "demoAudio": "../assets/demo.wav",
    "imports": [
        "acoustic-1"
        {
            "id": "bar/pitch",
            "options": {
                "roles": [
                    "pitch"
                ]
            }
        },
        {
            "id": "variance-A",
            "options": {
                "roles": [
                    "tension",
                    "energy"
                ]
            }
        }
    ],
    "configuration": {
        "dictionary": "../assets/dsdict.json"
    }
}
```
+ 必选字段
    + `$version`：文件格式版本，当前固定为`1.0`
+ 可选字段
    + `name`: 歌手名称，可为多语言，如为空则与`id`一致
    + `imports`：歌手依赖的推理模块
        + `id`：依赖的推理模块 ID，如果是别的库的那么使用`lib[version]/id`的形式，`lib`与`version`可以省略
        + `options`：提供给引用的推理模块的选项，需要符合对应的 API 版本以及推理模块的`schema`的限制
        <!-- + `roles`：在合成流程中的职责 -->
    + `avatar`：头像
    + `background`：可用于 SVS 编辑器显示的立绘背景
    + `demoAudio`：可用于 SVS 编辑器预览的声音
    + `configuration`：其他属性（根据`model`设置）
        + `dictionary`：歌手词典

#### 注意事项

- 每个歌手的预设所用到的`id`都必须在`desc.json`的`dependencies`中声明。

#### 关于 API Level

由于 DiffSinger 引擎架构的复杂性，我们为推理模块引入了 API Level 的概念，在声明文件中用`level`表示。

API Level 代表引擎接口的版本号，是一个正整数，每当模型的输入或输出格式发生更新时，它将向上递增。引擎官方为每个 API Level 制定一套描述模型功能的语法。

每个 Inference 模块在声明文件，使用`level`声明自己所属的 API Level，在`schema`字段中按照该 Level 规定的语法公开自己所支持的功能集合，在`configuration`字段中按照语法填写模型相关参数。

每个 Singer 模块在其`imports`字段中导入其依赖的 Inference 模块，在每个导入项的`options`字段中填写其选择的一部分功能，`options`的值需要符合其依赖的 Inference 模块所属的 API Level 语法。

### 可扩展性

- 具有新功能的模型开发完成后，开发者为之起一个`class`名，再基于现有的推理程序开发一个与这种模型匹配的解释器，这样即可扩展推理功能。

- 非 DiffSinger 甚至非 AI 的开发者，如 UTAU、Vocaloid，亦可通过扩展`class`来支持其他引擎，可以使用混合 Library 将歌手信息与歌声采样放在同一个 Library 中。

### 推荐目录结构

```
+ somedspk
  + assets
    - avatar.png
    - dict.json
  + inferences
    + acoustic
      - config.json
      - acoustic.onnx
    + duration
      - config.json
      - duration.onnx
    - ...
  + singers
    + singer1
      - config.json
  - desc.json
```

- 共享的资源文件放置在`assets`中
- 推理模块放置在`inferences`的子目录中，每个子目录配置一个`config.json`，歌手模块同理
- 根目录固定放置`desc.json`

## 3. 工具开发

dsinfer 库提供一个命令行工具，名称为`dsinfer-cli`。

### 功能贡献

- 校验
- 显示安装的包
- 安装
- 卸载
- 自动卸载
- 命令行推理
- 打包

#### 默认配置文件

`dsinfer-cli`会在以下路径搜索`dsinfer-conf.json`，此文件为其指定默认安装路径与默认推理驱动极其初始化参数。
- `<可执行文件目录>`
- `<用户目录>/.diffinger`

`dsinfer-conf.json`文件内容如下：
```json
{
    "paths": [
        "/home/user/.diffinger/packages"
    ],
    "driver": {
        "id": "onnx",
        "init": {
            "ep": "dml"
        }
    }
}
```

#### 安装状态文件

在一个目录中安装了包后，`dsinfer-cli`会留下一个记忆文件`status.json`。
```json
{
    "packages": [
        {
            "id": "zhibin[5.1]",
            "path": "zhibin-5.1",
            "contributes": [
                "singers",
                "inferences"
            ]
        }
    ]
}
```

### Package 发布

使用`dsinfer-cli pack`命令来将整理好的目录打包为`dspk`，此命令将执行校验与压缩功能。

打包完成的`dspk`中，其根目录会加入`package-info`目录，目录内有`manifest.json`，存储文件校验信息。存在此目录及其文件，且内容合法的`dspk`才能被安装。

```json
{
    "$version": "1.0",
    "files": [
        {
            "path": "desc.json",
            "crc32": "xxx"
        }
    ]
}
```

### 推理插件开发

在`plugins`中添加插件。

#### 推理解释器

创建派生于`InferenceInterpreter`的解释器类。

- `apiLevel`：返回解释器支持的最高 api 等级
- `key`：返回对应的推理参数类型的`class`，如`com.diffsinger.InferenceInterpreter.PitchInference`
- `validate`：校验推理模块是否符合规范，以及使用某个推理模块的歌手模块是否指定了正确的参数
- `create`：创建对应的推理任务类

#### 推理任务

创建派生于`Inference`的推理任务类。

- `initialize`：初始化推理任务，应当加载需要用到的模型
- `start`：开始推理任务，应当对输入的参数进行预处理，并构建推理图
- `startAsync`：`start`的异步版本
- `stop`：立即停止推理任务（同步）
- `state`：推理任务状态
- `result`：推理结果

<!-- 推理任务的输入格式，推荐从以下基本格式进行扩展。
```json
{
    "type": "inference",
    "content": {
         "class": "com.diffsinger.InferenceInterpreter.DurationPrediction",
    }
}
``` -->