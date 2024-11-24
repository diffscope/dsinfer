#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include <dsinfer/error.h>
#include <dsinfer/jsonvalue.h>

namespace dsinfer::dsinterp {
    struct Phoneme;
    struct Note;
    struct Word;
    struct Parameter;
    struct Segment;
    struct SpeakerMixCurve;

    bool from_json(const JsonValue &json, Phoneme &phoneme, Error *error);
    JsonValue to_json(const Phoneme &phoneme);

    bool from_json(const JsonValue &json, Note &note, Error *error);
    JsonValue to_json(const Note &note);

    bool from_json(const JsonValue &json, Word &word, Error *error);
    JsonValue to_json(const Word &word);

    bool from_json(const JsonValue &json, Parameter &parameter, Error *error);
    JsonValue to_json(const Parameter &parameter);

    bool from_json(const JsonValue &json, SpeakerMixCurve &spk, Error *error);
    JsonValue to_json(const SpeakerMixCurve &spk);

    bool from_json(const JsonValue &json, Segment &segment, Error *error);
    JsonValue to_json(const Segment &segment);
}
#endif // JSON_SERIALIZER_H