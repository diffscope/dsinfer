# 第一阶段推理任务格式

建图框架完成之前，为了使 InferenceInterpreter 能正常完成推理工作，特制定单模型推理的方案用于过渡。

`InferenceTask`的输入如下：

```json
{
    "session": 1,
    "context": 1,
    "input": [
        {
            "name": "f0",               // input argument name
            "format": "bytes",          // input data format
            "data": {        
                "type": "float",        // data type
                "shape": [ 1, 100 ],    // tensor shape
                "value": "<bytes>"      // raw data
            }
        },
        {
            "name": "steps",
            "format": "bytes",
            "data": {
                "type": "int64",
                "shape": [ 1 ],
                "value": "<bytes>"
            }
        }
    ],
    "output": [
        {
            "name": "x_masks",          // output argument name
            "format": "bytes"           // output data format
        }
    ]
}
```
- `session`：模型 ID
- `context`：当前推理上下文 ID
- `input`：输入参数
  - `name`：输入参数名
  - `format`：输入数据格式，可选`bytes`、`array`、`reference`
  - `data`：输入数据
    - `type`：数据类型
    - `shape`：向量形状
    - `value`：数据值
- `output`：输出参数
  - `name`：输出参数名
  - `format`：输出数据格式，可选`bytes`、`array`、`reference`

`InferenceTask`的输出为一个 json object，结构如下。
```json
{
    "format": "bytes",
    "data": {
        "type": "int64",
        "shape": [ 1 ],
        "value": "<bytes>"
    }
}
```
若输出数据格式选择`reference`，则推理完成后，输出的`value`字段是一个随机生成的 key，推理输出的向量将以`Ort::Value`的形式留在内存中，该实例以这个 key 存入指定的 context 中。

如果后续有其他模型的输入需要使用这个实例，那么输入数据的格式指定为`reference`，`value`指定为这个`key`即可。

上层可以使用向 context 中插入以下格式的 json object 来构造`Ort::Value`实例。
```json
{
    "type": "object",
    "content": {
        "class": "Ort::Value",
        "format": "bytes",
        "data": {
            "type": "int64",
            "shape": [ 1 ],
            "value": "<bytes>"
        }
    }
}
```
当上层插入这种 json object 时，下层将构造对应的`Ort::Value`。
当上层获取`Ort::Value`时，下层应返回对应的 json object。