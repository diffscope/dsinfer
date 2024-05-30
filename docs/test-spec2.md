# 声库规范 2.0

## 库

本声库规范中，安装的最小单元被称为库（library），分为以下两种：
+ Singer Library（歌手库）
+ Module Library（组件库）

库的根目录必须包含一个名为`desc.json`的文件，文件格式如下。

```json
{
    "id": "lib1",
    "version": "1.0.0.0",
    "type": "singer",
    "compatVersion": "0.0.0.0",
    "vendor": "someone",
    "copyright": "Copyright (C) someone",
    "description": "Some library",
    "url": "https://www.dummy.cn",
    "dependencies": [
        {
            "id": "acoustic-1",
            "version": "1.0.0.0"
        },
        {
            "id": "variance-A",
            "version": "1.0.0.0",
            "required": false
        },
        {
            "id": "variance-B",
            "version": "1.0.0.0",
            "required": false
        }
    ]
}
```

+ 必选字段
    + `id`：唯一标识符
    + `version`：版本号，格式为`x.y[.z.w]`，
    + `type`：类型，可为`singer`、`module`
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

### 歌手库

歌手库的根目录需要再提供一个`singer.json`的文件，文件格式如下。
```json
{
    "singers": [
        {
            "name": "Some singer",
            "avatar": "assets/avatar.png",
            "background": "assets/sprite.png",
            "demoAudio": "assets/demo.wav",
            "preset": [
                {
                    "id": "acoustic-1",
                    "features": [
                        "acoustic"
                    ]
                },
                {
                    "id": "variance-A",
                    "features": [
                        "pitch"
                    ]
                },
                {
                    "id": "variance-B",
                    "features": [
                        "dur"
                    ]
                }
            ]
        }
    ]
}
```

### 组件库

组件库的根目录需要再提供一个`module.json`的文件，文件格式如下。
```json
{
    "features": [
        {
            "class": "svs.Variance.PitchPrediction",
            "level": 1,
            "attributes":{
                "linguistic": "./linguistic.onnx"
            }
        },
        {
            "class": "svs.Variance.VariancePrediction",
            "level": 1,
            "attributes": {
                "predictions": ["breathiness", "voicing"],
                "hiddenSize": 256
            }
        }
    ]
}
```

## 安装包

一个安装包是一个扩展名为`dspk`的 zip 格式文件，内部包含若干个库，并列在根目录中。

```
+ zhibin-1.0.dspk
    + zhibin-singers
    + zhibin-acoustic
    + zhibin-pitch
```

## 推理库

### 库管理器

给定一组路径，作为搜索库的搜索路径，类似于 PATH 环境变量。当加载一个库的时候，依次从这组路径中搜索其依赖并递归加载。

每个库在内存中只有一份`LibraryImage`数据，使用引用计数维护。

开发者使用`Library`对象访问`LibraryImage`，`Library`的`open`与`close`相当于`LibraryImage`引用计数的增减。

### 模型管理器
每个`onnx`模型在内存中只有一份`ModelImage`数据，使用引用计数维护。

开发者使用`Session`对象访问`ModelImage`，`Session`的`open`与`close`相当于`ModelImage`引用计数的增减。

### 推理解释器

解释器（Interpreter）规定了一种`feature`的推理协议，每个解释器有一个`key`，与之对应的`feature`的`class`值相同。

给定一组路径，在这些路径中放置一系列具有唯一导出函数`dsinfer_interpreter_instance`的动态库作为插件，`dsinfer`在初始化时将加载它们，每个插件提供一个解释器指针。

当一个模块被加载时，其拥有的`feature`也将被加载。`dsinfer`将查找是否存在对应的解释器，如果不存在或`level`不兼容，则跳过。

在解释器初始化时，它将根据`feature`中的参数动态地创建一个推理图，称为`Inference`，一个`Inference`中具有多一个以`Session`为节点的图，推理过程即从起点运算至终点。