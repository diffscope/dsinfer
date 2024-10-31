# DiffSinger 推理实现规范（草案）

主要参考：https://github.com/onnx/onnx/blob/main/docs/Operators.md

|           Class           | Description |
| :-----------------------: | :---------: |
| ai.svs.DurationPrediction |             |
|  ai.svs.PitchPrediction   |             |
| ai.svs.VariancePrediction |             |
| ai.svs.AcousticInference  |             |
|  ai.svs.VocoderInference  |             |

## 前置说明

- Schemas: 模块向编辑器开放的信息
- Options：歌手引用此模块时应指定的参数选项
- Configurations：推理引擎需要用到的所有信息
- Variables：模型推理过程中涉及的输入、输出和中间变量

## ai.svs.DurationPrediction

### Schemas

|   name    |        type        |      description       |       example        |
| :-------: | :----------------: | :--------------------: | :------------------: |
| phonemes  | list&lt;string&gt; |      音素名称列表      |    ["SP", "zh/a"]    |
| languages | list&lt;string&gt; |      语言名称列表      |  ["zh", "ja", "en"]  |
| speakers  | list&lt;string&gt; | 说话人（音色）名称列表 | ["zhibin", "qixuan"] |

### Options

无

### Configurations

|        name         |         type         |               description                |                example                |
| :-----------------: | :------------------: | :--------------------------------------: | :-----------------------------------: |
|      phonemes       | map<string, integer> |         音素名称与音素 ID 对应表         |         {"SP": 1, "zh/a": 2}          |
|      languages      | map<string, integer> |         语言名称与语言 ID 对应表         |      {"zh": 1, "ja": 2, "en": 3}      |
|      speakers       |  map<string, path>   | 说话人（音色）与说话人嵌入文件路径对应表 | {"zhibin": "./embeddings/zhibin.emb"} |
|       encoder       |         path         |            编码器模型文件路径            |      "./weights/linguistic.onnx"      |
|      predictor      |         path         |            预测器模型文件路径            |       "./weights/duration.onnx"       |
|     frameWidth      |        double        |               帧宽度（秒）               |           0.011609977324263           |
|    useLanguageId    |       boolean        |           是否启用语言 ID 嵌入           |                 true                  |
| useSpeakerEmbedding |       boolean        |            是否启用说话人嵌入            |                 true                  |
|     hiddenSize      |       integer        |      隐层维度（说话人嵌入向量维度）      |                  256                  |

### Variables

|   model   |     variable     |  I/O   |  type   |            shape            |     description      |    activation condition     |
| :-------: | :--------------: | :----: | :-----: | :-------------------------: | :------------------: | :-------------------------: |
|  encoder  |      tokens      | input  |  int64  |        (1, n_tokens)        |       音素 ID        |              -              |
|  encoder  |    languages     | input  |  int64  |        (1, n_tokens)        |       语言 ID        |    useLanguageId == true    |
|  encoder  |     word_div     | input  |  int64  |        (1, n_words)         |       音节划分       |              -              |
|  encoder  |     word_dur     | input  |  int64  |        (1, n_words)         |    音节长度（帧）    |              -              |
|  encoder  |   encoder_out    | output | float32 | (1, n_tokens, `hiddenSize`) |          -           |              -              |
|  encoder  |     x_masks      | output | boolean |        (1, n_tokens)        |          -           |              -              |
| predictor | encoder_out^[1]^ | input  |    -    |              -              |          -           |              -              |
| predictor |   x_masks^[2]^   | input  |    -    |              -              |          -           |              -              |
| predictor |     ph_midi      | input  |  int64  |        (1, n_tokens)        | 音素粗略音高（半音） |              -              |
| predictor |    spk_embed     | input  | float32 | (1, n_tokens, `hiddenSize`) |  说话人（音色）嵌入  | useSpeakerEmbedding == true |
| predictor |   ph_dur_pred    | output | float32 |        (1, n_tokens)        |    音素长度预测值    |              -              |

[1] 该输入绑定到 encoder.outputs.encoder_out

[2] 该输入绑定到 encoder.outputs.x_masks

## ai.svs.PitchPrediction

### Schemas

|        name         |        type        |      description       |       example        |
| :-----------------: | :----------------: | :--------------------: | :------------------: |
|      phonemes       | list&lt;string&gt; |      音素名称列表      |    ["SP", "zh/a"]    |
|      languages      | list&lt;string&gt; |      语言名称列表      |  ["zh", "ja", "en"]  |
|      speakers       | list&lt;string&gt; | 说话人（音色）名称列表 | ["zhibin", "qixuan"] |
| allowExpressiveness |      boolean       | 是否允许控制表现力因子 |         true         |

### Options

无

### Configurations

|           name            |         type         |                description                |                example                |
| :-----------------------: | :------------------: | :---------------------------------------: | :-----------------------------------: |
|         phonemes          | map<string, integer> |         音素名称与音素 ID 对应表          |         {"SP": 1, "zh/a": 2}          |
|         languages         | map<string, integer> |         语言名称与语言 ID 对应表          |      {"zh": 1, "ja": 2, "en": 3}      |
|         speakers          |  map<string, path>   | 说话人（音色）与说话人嵌入文件路径对应表  | {"zhibin": "./embeddings/zhibin.emb"} |
|          encoder          |         path         |            编码器模型文件路径             |      "./weights/linguistic.onnx"      |
|      linguisticMode       |         enum         | 语言学编码器的工作模式（word 或 phoneme） |               "phoneme"               |
|         predictor         |         path         |            预测器模型文件路径             |        "./weights/pitch.onnx"         |
|        frameWidth         |        double        |               帧宽度（秒）                |           0.011609977324263           |
|       useLanguageId       |       boolean        |           是否启用语言 ID 嵌入            |                 true                  |
|    useSpeakerEmbedding    |       boolean        |            是否启用说话人嵌入             |                 true                  |
|        hiddenSize         |       integer        |      隐层维度（说话人嵌入向量维度）       |                  256                  |
|     useExpressiveness     |       boolean        |          是否启用表现力因子输入           |                 true                  |
|       useRestFlags        |       boolean        |          是否启用休止符记号输入           |                 true                  |
| useContinuousAcceleration |       boolean        |           是否使用连续加速采样            |                 true                  |

### Variables

|   model   |     variable     |  I/O   |  type   |            shape            |        description         |        activation condition        |
| :-------: | :--------------: | :----: | :-----: | :-------------------------: | :------------------------: | :--------------------------------: |
|  encoder  |      tokens      | input  |  int64  |        (1, n_tokens)        |          音素 ID           |                 -                  |
|  encoder  |    languages     | input  |  int64  |        (1, n_tokens)        |          语言 ID           |       useLanguageId == true        |
|  encoder  |     word_div     | input  |  int64  |        (1, n_words)         |          音节划分          |      linguisticMode == "word"      |
|  encoder  |     word_dur     | input  |  int64  |        (1, n_words)         |       音节长度（帧）       |      linguisticMode == "word"      |
|  encoder  |      ph_dur      | input  |  int64  |        (1, n_tokens)        |       音素长度（帧）       |    linguisticMode == "phoneme"     |
|  encoder  |   encoder_out    | output | float32 | (1, n_tokens, `hiddenSize`) |             -              |                 -                  |
| predictor | encoder_out^[1]^ | input  |    -    |              -              |             -              |                 -                  |
| predictor |      ph_dur      | input  |  int64  |        (1, n_tokens)        |       音素长度（帧）       |                 -                  |
| predictor |    note_midi     | input  | float32 |        (1, n_notes)         |      音符音高（半音）      |                 -                  |
| predictor |    note_rest     | input  | boolean |        (1, n_notes)         |         休止符记号         |        useRestFlags == true        |
| predictor |     note_dur     | input  |  int64  |        (1, n_notes)         |       音符长度（帧）       |                 -                  |
| predictor |      pitch       | input  |  int64  |        (1, n_frames)        | 音高（半音，作为重录条件） |                 -                  |
| predictor |       expr       | input  | float32 |        (1, n_frames)        |         表现力因子         |     useExpressiveness == true      |
| predictor |      retake      | input  | boolean |        (1, n_frames)        |          重录标记          |                 -                  |
| predictor |    spk_embed     | input  | float32 | (1, n_frames, `hiddenSize`) |     说话人（音色）嵌入     |    useSpeakerEmbedding == true     |
| predictor |     speedup      | input  |  int64  |           scalar            |           加速比           | useContinuousAcceleration == false |
| predictor |      steps       | input  |  int64  |           scalar            |          采样步数          | useContinuousAcceleration == true  |
| predictor |    pitch_pred    | output | float32 |        (1, n_frames)        |         音高预测值         |                 -                  |

[1] 该输入绑定到encoder.outputs.encoder_out

## ai.svs.VariancePrediction

### Schemas

|    name     |        type        |                      description                       |          example           |
| :---------: | :----------------: | :----------------------------------------------------: | :------------------------: |
|  phonemes   | list&lt;string&gt; |                      音素名称列表                      |       ["SP", "zh/a"]       |
|  languages  | list&lt;string&gt; |                      语言名称列表                      |     ["zh", "ja", "en"]     |
|  speakers   | list&lt;string&gt; |                 说话人（音色）名称列表                 |    ["zhibin", "qixuan"]    |
| predictions |  list&lt;enum&gt;  | 预测输出参数列表（energy/breathiness/voicing/tension） | ["breathiness", "tension"] |

### Options

|      name      |       type       |    description     |   example   |
| :------------: | :--------------: | :----------------: | :---------: |
| usePredictions | list&lt;enum&gt; | 使用的预测参数列表 | ["tension"] |

### Configurations

|           name            |         type         |                   description                    |                example                |
| :-----------------------: | :------------------: | :----------------------------------------------: | :-----------------------------------: |
|         phonemes          | map<string, integer> |             音素名称与音素 ID 对应表             |         {"SP": 1, "zh/a": 2}          |
|         languages         | map<string, integer> |             语言名称与语言 ID 对应表             |      {"zh": 1, "ja": 2, "en": 3}      |
|         speakers          |  map<string, path>   |     说话人（音色）与说话人嵌入文件路径对应表     | {"zhibin": "./embeddings/zhibin.emb"} |
|          encoder          |         path         |                编码器模型文件路径                |      "./weights/linguistic.onnx"      |
|      linguisticMode       |         enum         |    语言学编码器的工作模式（word 或 phoneme）     |               "phoneme"               |
|         predictor         |         path         |                预测器模型文件路径                |       "./weights/multivar.onnx"       |
|        predictions        |   list&lt;enum&gt;   | 待预测参数（energy/breathiness/voicing/tension） |      ["breathiness", "tension"]       |
|        frameWidth         |        double        |                   帧宽度（秒）                   |           0.011609977324263           |
|       useLanguageId       |       boolean        |               是否启用语言 ID 嵌入               |                 true                  |
|    useSpeakerEmbedding    |       boolean        |                是否启用说话人嵌入                |                 true                  |
|        hiddenSize         |       integer        |          隐层维度（说话人嵌入向量维度）          |                  256                  |
| useContinuousAcceleration |       boolean        |               是否使用连续加速采样               |                 true                  |

### Variables

|   model   |     variable     |  I/O   |  type   |               shape               |     description      |        activation condition        | since |
| :-------: | :--------------: | :----: | :-----: | :-------------------------------: | :------------------: | :--------------------------------: | :---: |
|  encoder  |      tokens      | input  |  int64  |           (1, n_tokens)           |       音素 ID        |                 -                  |   1   |
|  encoder  |    languages     | input  |  int64  |           (1, n_tokens)           |       语言 ID        |       useLanguageId == true        |   1   |
|  encoder  |     word_div     | input  |  int64  |           (1, n_words)            |       音节划分       |      linguisticMode == "word"      |   1   |
|  encoder  |     word_dur     | input  |  int64  |           (1, n_words)            |    音节长度（帧）    |      linguisticMode == "word"      |   1   |
|  encoder  |      ph_dur      | input  |  int64  |           (1, n_tokens)           |    音素长度（帧）    |    linguisticMode == "phoneme"     |   1   |
|  encoder  |   encoder_out    | output | float32 |    (1, n_tokens, `hiddenSize`)    |          -           |                 -                  |   1   |
| predictor | encoder_out^[1]^ | input  |    -    |                 -                 |          -           |                 -                  |   1   |
| predictor |      ph_dur      | input  |  int64  |           (1, n_tokens)           |    音素长度（帧）    |                 -                  |   1   |
| predictor |      pitch       | input  | float32 |           (1, n_frames)           |     音高（半音）     |                 -                  |   1   |
| predictor |      energy      | input  | float32 |           (1, n_frames)           | 能量（作为重录条件） |      "energy" in predictions       |   1   |
| predictor |   breathiness    | input  | float32 |           (1, n_frames)           | 气声（作为重录条件） |    "breathiness" in predictions    |   1   |
| predictor |     voicing      | input  | float32 |           (1, n_frames)           | 发声（作为重录条件） |      "voicing" in predictions      |   1   |
| predictor |     tension      | input  | float32 |           (1, n_frames)           | 发声（作为重录条件） |      "tension" in predictions      |   1   |
| predictor |      retake      | input  | boolean | (1, n_frames, `len(predictions)`) |       重录标记       |                 -                  |   1   |
| predictor |    spk_embed     | input  | float32 |    (1, n_frames, `hiddenSize`)    |  说话人（音色）嵌入  |    useSpeakerEmbedding == true     |       |
| predictor |     speedup      | input  |  int64  |              scalar               |        加速比        | useContinuousAcceleration == false |   1   |
| predictor |      steps       | input  |  int64  |              scalar               |       采样步数       | useContinuousAcceleration == true  |   1   |
| predictor |   energy_pred    | output | float32 |           (1, n_frames)           |      能量预测值      |      "energy" in predictions       |   1   |
| predictor | breathiness_pred | output | float32 |           (1, n_frames)           |      气声预测值      |    "breathiness" in predictions    |   1   |
| predictor |   voicing_pred   | output | float32 |           (1, n_frames)           |      发声预测值      |      "voicing" in predictions      |   1   |
| predictor |   tension_pred   | output | float32 |           (1, n_frames)           |      张力预测值      |      "tension" in predictions      |   1   |

[1] 该输入绑定到 encoder.outputs.encoder_out

## ai.svs.AcousticInference

### Schemas

|        name        |        type        |                         description                          |          example           |
| :----------------: | :----------------: | :----------------------------------------------------------: | :------------------------: |
|      phonemes      | list&lt;string&gt; |                         音素名称列表                         |       ["SP", "zh/a"]       |
|     languages      | list&lt;string&gt; |                         语言名称列表                         |     ["zh", "ja", "en"]     |
|      speakers      | list&lt;string&gt; |                    说话人（音色）名称列表                    |    ["zhibin", "qixuan"]    |
|  varianceControls  |  list&lt;enum&gt;  | 需要输入的唱法参数列表（energy/breathiness/voicing/tension） | ["breathiness", "tension"] |
| transitionControls |  list&lt;enum&gt;  |        支持的偏移变换类型参数列表（gender/velocity）         |   ["gender", "velocity"]   |

### Options

无

### Configurations

|           name            |         type         |                         description                          |                     example                      |
| :-----------------------: | :------------------: | :----------------------------------------------------------: | :----------------------------------------------: |
|         phonemes          | map<string, integer> |                   音素名称与音素 ID 对应表                   |               {"SP": 1, "zh/a": 2}               |
|         languages         | map<string, integer> |                   语言名称与语言 ID 对应表                   |           {"zh": 1, "ja": 2, "en": 3}            |
|         speakers          |  map<string, path>   |           说话人（音色）与说话人嵌入文件路径对应表           |      {"zhibin": "./embeddings/zhibin.emb"}       |
|           model           |         path         |                       声学模型文件路径                       |            "./weights/acoustic.onnx"             |
|       useLanguageId       |       boolean        |                     是否启用语言 ID 嵌入                     |                       true                       |
|    useSpeakerEmbedding    |       boolean        |                      是否启用说话人嵌入                      |                       true                       |
|        hiddenSize         |       integer        |                隐层维度（说话人嵌入向量维度）                |                       256                        |
|        parameters         |   list&lt;enum&gt;   | 启用的参数列表（energy/breathiness/voicing/tension/gender/velocity） | ["breathiness", "tension", "gender", "velocity"] |
| useContinuousAcceleration |       boolean        |                     是否使用连续加速采样                     |                       true                       |
|     useVariableDepth      |       boolean        |                     是否使用可变深度采样                     |                       true                       |
|         maxDepth          |        double        |                        允许的最大深度                        |                       0.6                        |
|        sampleRate         |       integer        |                          音频采样率                          |                      44100                       |
|          hopSize          |       integer        |                        梅尔频谱帧跨度                        |                       512                        |
|          winSize          |       integer        |                        梅尔频谱窗大小                        |                       2048                       |
|          fftSize          |       integer        |                      梅尔频谱 FFT 维度                       |                       2048                       |
|        melChannels        |       integer        |                        梅尔频谱通道数                        |                       128                        |
|        melMinFreq         |       integer        |                    梅尔频谱最小频率（Hz）                    |                        40                        |
|        melMaxFreq         |       integer        |                    梅尔频谱最大频率（Hz）                    |                      16000                       |
|          melBase          |         enum         |                     梅尔频谱底数（e/10）                     |                       "e"                        |
|         melScale          |         enum         |                    melScale（slaney/htk）                    |                     "slaney"                     |

### Variables

|  variable   |  I/O   |  type   |            shape             |     description      |                     activation condition                     |
| :---------: | :----: | :-----: | :--------------------------: | :------------------: | :----------------------------------------------------------: |
|   tokens    | input  |  int64  |        (1, n_tokens)         |       音素 ID        |                              -                               |
|  languages  | input  |  int64  |        (1, n_tokens)         |       语言 ID        |                    useLanguageId == true                     |
|  durations  | input  |  int64  |        (1, n_tokens)         |    音素长度（帧）    |                              -                               |
|     f0      | input  | float32 |        (1, n_frames)         |      基频（Hz）      |                              -                               |
|   energy    | input  | float32 |        (1, n_frames)         |         能量         |                    "energy" in parameters                    |
| breathiness | input  | float32 |        (1, n_frames)         |         气声         |                 "breathiness" in parameters                  |
|   voicing   | input  | float32 |        (1, n_frames)         |         发声         |                   "voicing" in parameters                    |
|   tension   | input  | float32 |        (1, n_frames)         |         张力         |                   "tension" in parameters                    |
|   gender    | input  | float32 |        (1, n_frames)         |       性别偏移       |                    "gender" in parameters                    |
|  velocity   | input  | float32 |        (1, n_frames)         |       发音速度       |                   "velocity" in parameters                   |
|  spk_embed  | input  | float32 | (1, n_frames, `hiddenSize`)  |  说话人（音色）嵌入  |                 useSpeakerEmbedding == true                  |
|    depth    | input  |  int64  |            scalar            | 采样深度（离散加速） | useVariableDepth == true && useContinuousAcceleration == false |
|    depth    | input  | float32 |            scalar            | 采样深度（连续加速） | useVariableDepth == true && useContinuousAcceleration == true |
|   speedup   | input  |  int64  |            scalar            |        加速比        |              useContinuousAcceleration == false              |
|    steps    | input  |  int64  |            scalar            |       采样步数       |              useContinuousAcceleration == true               |
|     mel     | output | float32 | (1, n_frames, `melChannels`) |       梅尔频谱       |                              -                               |

## ai.svs.VocoderInference

### Schemas

待定

### Options

无

### Configurations

|    name     |  type   |      description       |         example          |
| :---------: | :-----: | :--------------------: | :----------------------: |
|    model    |  path   |   声码器模型文件路径   | "./weights/vocoder.onnx" |
| sampleRate  | integer |       音频采样率       |          44100           |
|   hopSize   | integer |     梅尔频谱帧跨度     |           512            |
|   winSize   | integer |     梅尔频谱窗大小     |           2048           |
|   fftSize   | integer |    梅尔频谱FFT维度     |           2048           |
| melChannels | integer |     梅尔频谱通道数     |           128            |
| melMinFreq  | integer | 梅尔频谱最小频率（Hz） |            40            |
| melMaxFreq  | integer | 梅尔频谱最大频率（Hz） |          16000           |
|   melBase   |  enum   |  梅尔频谱底数（e/10）  |           "e"            |
|  melScale   |  enum   | melScale（slaney/htk） |         "slaney"         |

### Variables

| variable |  I/O   |  type   |            shape             | description | activation condition |
| :------: | :----: | :-----: | :--------------------------: | :---------: | :------------------: |
|   mel    | input  | float32 | (1, n_frames, `melChannels`) |  梅尔频谱   |          -           |
|    f0    | input  | float32 |        (1, n_frames)         | 基频（Hz）  |          -           |
| waveform | output | float32 |        (1, n_samples)        |    波形     |          -           |

## 遗留问题

1. 目前音素名仍在采用 `zh/a` 这种格式，是否考虑换用更清晰的写法，例如 `{"language": "zh", "token": "a"}` 这种格式
2. 声学模型 maxDepth 是否应该放到 schemas
3. 声学模型和声码器的 mel 参数（共 9 个）需要用于校验彼此之间是否兼容，是否需要放到 schemas
