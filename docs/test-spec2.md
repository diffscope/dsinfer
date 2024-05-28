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
    "url": "https://www.dummy.cn"
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

### 歌手库

歌手库的根目录需要再提供一个`singers.json`的文件，文件格式如下。
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
                    "version": "1.0.0.0",
                    "features": [
                        "acoustic"
                    ]
                },
                {
                    "id": "variance-A",
                    "version": "1.0.0.0",
                    "required": false,
                    "features": [
                        "pitch"
                    ]
                },
                {
                    "id": "variance-B",
                    "version": "1.0.0.0",
                    "required": false,
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