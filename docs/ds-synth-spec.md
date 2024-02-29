# DiffScope 推理规范

本规范是 DiffSinger 歌声合成流程的一种实现，其包括编辑器、合成管理器、推理中间件与底层推理库四部分。

## 概念介绍

### 执行体

+ 编辑器：歌声合成前端，DiffScope 或其他支持本规范的编辑器；
+ 合成管理器：也称为合成引擎（后称管理器），负责声库的管理以及合成任务的调度；
+ 推理中间件：模型抽象层，即本项目`dsinfer`，对上提供以类 MIDI 格式为输入的合成接口，向下输出张量信息到推理库；
+ 底层推理库：目前定为 OnnxRuntime；

### 数据

+ 内置模型：由合管理器自带的一组全局模型，如声码器等；
+ 声库：包括声学、唱法等模型在内的歌声数据库，可以从管理器中安装或卸载；

## 声库规范

### 目录结构

```
+ dummy
    + assets
        + avatar.png
        + dict.yaml
        + license.txt
    + features
        + acoustic
            + config.json
            + model.onnx
        + duration
            + config.json
            + model.onnx
        + pitch
            + config.json
            + model.onnx
        + ...
    + metadata.json
```

+ `features`：必选目录，每个子目录包括一组模型与其对应的配置信息；
+ `assets`：存放与声库信息相关的文件，以及一些全局共用的文件；
+ `metadata.json`：声库描述文件
    ```json
    {
        "id": "dummy-12138",
        "version": "1.0.0",
        "vendor": "someone",
        "copyright": "someone",
        "url": "https://www.dummy.cn",
        "updateUrl": "https://www.dummy.cn/update.json",
        "dict": "assets/dict.yaml",
        "character": {
            "name": "dummy",
            "gender": "helicopter",
            "avatar": "assets/avatar.png",
            "background": "assets/sprite.png",
            "demoAudio": "assets/demo.wav"
        }
    }
    ```
    + 必选字段
        + `id`：声库的唯一标识符
        + `version`：声库版本号
    + 可选字段
        + `name`：声库名，用于前端显示
        + `avatar`：声库头像路径
        + `background`: 声库背景图片路径
        + `dict`：声库字典文件路径
        + `vender`：声库提供方
        + `copyright`：声库版权信息
        + `url`：声库网站
    + 可添加其他字段

+ `features/xxx/config.json`：模型描述文件
    ```json
    {
        "id": "dummy-12138-pitch",
        "type": "pitch",
        "level": 1,
        "path": "model.onnx",
        "version": null,
        "arguments": {
            ...
        },
        "dependencies": {
            "linguistic": {
                "id": "dummy-12138-linguist",
                "level": 1
            }
        }
    }
    ```
    + 必选字段
        + `id`：模型的唯一标识符，不同声库的模型`id`可以重复
        + `type`：模型类型，目前有`acoustic`、`pitch`、`duration`、`variance`
        + `level`：模型参数集等级
        + `path`: 模型路径
    + 可选字段
        + `version`：模型版本，如果不存在或者为`null`则视为与声库一致
        + `arguments`：模型支持的参数
        + `dependencies`：模型所依赖的其他模型


`features`的子目录名在推理过程中没有实际用处，只为了方便辨别模型类型，一个声库内不重复即可；
管理器的内置模型文件结构与`features`的每个子目录一致，且地位平等，都有唯一标识符；

## 合成管理器

管理器启动时根据配置信息加载推荐的内置模型，在内存中维护所有声库的元数据。

管理器应当自行决定何时加载或释放模型，可以在启动时就加载，也可以第一次推理时加载，根据实时硬件资源信息决定是否临时释放模型。

同时，在接受到合成请求时，需要检查合成请求是否合法、依赖的模型是否存在等。


## 推理中间件

`dsinfer`是 DiffScope 管理器与 OnnxRuntime 底层推理库的中间件，负责加载、释放模型与执行推理任务。

### 初始化

OnnxRuntime 支持 CPU、Cuda、DirectML（仅 Windows）等执行器（ExecutionProvider，后称 ep），`dsinfer`库载入后，需要首先选择一个执行器进行初始化。

不同 ep 的代码在不同的`onnxruntime`动态库中，初始化时还需要提供一个目录路径，这个路径中应存在`onnxruntime`动态库及其依赖库，`dsinfer`将使用运行时动态加载的方式使用之。

初始化完成后，`onnxruntime`动态库将始终在内存中，直到与`dsinfer`的进程退出，因此运行期切换 ep 是不支持的。

### 加载与释放模型

加载模型时应提供模型对应的`config.json`的路径，`dsinfer`将按照`type`字段指定的模型类型选择对应的调用约定进行后续的推理。

### 推理

推理过程在其他线程中执行，可传入回调函数通知结束以避免轮询，需要调用者保证操作的原子性。