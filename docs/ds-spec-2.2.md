# DiffSinger 数据格式与推理接口规范 2.2

> DiffSinger Data Format and Inference Interface Specification 2.2

此规范为 OpenVPI 为各种 AI 推理工具制定的标准，旨在为各种模型提供通用的组织结构与调用接口，使 AI 模型的分发与调用更为有序、规范。

本规范主要指导以下几种基础设施的开发：
1. 安装器（Installer）
2. 加载器（Loader）
3. 推理器（Actuator）

## 关于 Library

### 文件结构

本规范内，可分发的数据包的最小单位是 Library（库），是一个以`dslib`为扩展名的 ZIP 格式的压缩包。

压缩包内基本结构为：
```
- xxx.dslib
  - desc.json
  - ...
```

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
    "url": "https://www.dummy.cn",
    "dependencies": [
        {
            "id": "bar",
            "version": "1.0.0.0"
        }
    ],
    "require": [
        "singer"
    ],
    "properties": {
        "accessory": true,
        "single": true
    }
}
```
+ 必选字段
    + `id`：唯一标识符
    + `version`：版本号，格式为`x.y[.z.w]`
+ 可选字段
    + `compatVersion`：兼容到的最低版本，如果为`0.0.0.0`表示向下兼容所有，如果与`version`相同则表示不向下兼容，缺省为不向下兼容
    + `vender`：提供方
    + `copyright`：版权信息
    + `description`：介绍文字
    + `url`：网站
    + `dependencies`：依赖的库
        + `id`：依赖库 ID
        + `version`：依赖库版本
        + `required`：是否为强制依赖，默认为`true`
    + `require`：引入的附加声明文件列表，只有一个的话可以写字符串（不需要扩展名）
    + `properties`：与加载器或安装器相关的属性
        + `accessory`：表示是其他 Library 的配件，当其所属的 Library 不存在时，可以自动删除，默认为`false`
        + `single`: 同`id`的 Library 只能加载同一个，默认为`false`

`require`引入的必须是一个`json`文件，其格式如下。
```json
{
    "type": "<type name>"
}
```
所有`json`文件中的相对路径都是相对于所在的`json`文件。

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
- ~/.diffsinger
  - lib
      - lib1
        - desc.json
        - ...
      - lib2
        - desc.json
        - ...
  - ...
```

加载器可由用户在启动时指定一个路径列表（如`~/.diffsinger/lib;~/lib1;~/lib2;~/lib3`），当加载器加载某个`dslib`并伴随着解析其依赖时，加载器将依次遍历这个列表，尝试每个路径，如果在这个路径中找到了符合条件的依赖则加载之，并按同样的方法解析下一个依赖，直到结束。

### 附加声明文件

通过`require`可引入附加声明文件，目前支持的`type`有以下几种。

- `inference`：包含可推理的模型
- `singer`：包含歌手信息，歌手预设

如果`require`为空，那么说明此 Library 只包含公用数据供其他 Library 访问，不提供其他功能。

## Inference Library

Inference Library 负责执行某一项参数的推理任务，承担了最底层、核心的工作。

声明文件中的`type`字段应为`inference`。

### 声明文件

`inference.json`是 Library 所支持的推理形式的声明文件，主要包括以下内容。

```json
{
    "type": "inference",
    "inferences": [
        {
            "id": "pitch",
            "class": "org.DiffSinger.PitchPrediction",
            "level": 1,
            "arguments": {
                "properties": {
                    "prediction": {
                        "type": "string",
                        "enum": ["breathiness", "voicing"]
                    }
                },
                "required": ["prediction"]
            },
            "internal": {
                "config": {
                    "linguistic": "./linguistic.onnx"
                }
            }
        },
        {
            "id": "variance",
            "class": "org.DiffSinger.VariancePrediction",
            "level": 1,
            "internal": {
                "config": {
                    "hiddenSize": 256
                }
            }
        }
    ]
}
```
+ 必选字段
    + `id`: 推理模型标识符
    + `level`: 推理解释器应选择的调用约定版本
+ 可选字段
    + `class`: 对应的推理解释器，如果不填则与`id`一致
    + `arguments`：其他可选/可调模型参数，为一个 Json Schema
    + `internal`：解释器内部使用配置信息

### 注意事项

- 不同的`class`值代表不同的可以被推理的参数类型，如声音、音高、音素，推理程序中必须存在一种与之匹配的解释器。

- 同一个 Library 的所有`inferences`中不可出现`id`相同的对象，否则将视为非法 Library。

## Singer Library

Singer Library 负责定义一个或若干个歌手的信息，以及其需要使用的推理库。

声明文件中的`type`字段应为`singer`。

### 声明文件

`singer.json`是 Singer 的信息声明文件，主要包括以下内容。

```json
{
    "type": "singer",
    "singers": [
        {
            "name": "Some singer",
            "avatar": "assets/avatar.png",
            "background": "assets/sprite.png",
            "demoAudio": "assets/demo.wav",
            "preset": [
                {
                    "id": "acoustic-1",
                    "inference": "acoustic"
                },
                {
                    "id": "variance-A",
                    "inference": {
                        "id": "variance",
                        "arguments": {
                            "prediction": "duration"
                        }
                    }
                }
            ]
        }
    ]
}
```
+ 必选字段
    + `name`: 歌手名称
    + `preset`：歌手预设
+ 可选字段
    + `avatar`：头像
    + `background`：可用于 SVS 编辑器显示的立绘背景
    + `demoAudio`：可用于 SVS 编辑器预览的声音

### 注意事项

- 每个歌手的预设所用到的`id`都必须在`desc.json`的`dependencies`中声明。

- 每个`preset`中的`arguments`必须与对应的`inference`中定义的 Json Schema 匹配，否则将加载失败。

- 同一个 Library 的所有`singers`中不可出现`name`相同的对象，否则将视为非法 Library。

## 混合 Library

若一个 Library 的`desc.json`中，`require`引入了多个声明文件，既包含`singer`也包含`inference`，那么是一个混合 Library，只需满足这两种 Library 的注意事项中的限制条件即可。

如果将歌手信息与其预设中依赖的推理库都放在混合 Library 中，那么`preset`中的项可以不需要`id`，没有`id`则`id`默认为当前 Library。

## 后记

### 可扩展性

- 具有新功能的模型开发完成后，开发者为之起一个`class`名，再基于现有的推理程序开发一个与这种模型匹配的解释器，这样即可扩展推理功能。

- 非 DiffSinger 甚至非 AI 的开发者，如 UTAU、Vocaloid，亦可通过扩展`class`来支持其他引擎，可以使用混合 Library 将歌手信息与歌声采样放在同一个 Library 中。