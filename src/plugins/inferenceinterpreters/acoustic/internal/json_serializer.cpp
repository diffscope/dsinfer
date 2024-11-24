#include "json_serializer.h"
#include "json_utils.h"
#include "project.h"

namespace dsinfer::dsinterp {

    bool from_json(const JsonValue &json, Phoneme &phoneme, Error *error) {
        if (!json.isObject()) {
            if (error) {
                *error = Error(Error::InvalidFormat, "Invalid format of Phoneme: json value is not object");
            }
            return false;
        }

        if (!get_input<std::string>("Phoneme", json, "token", error, phoneme.token)) {
            return false;
        }
        phoneme.language = json["language"].toString(); // can be omitted
        if (!get_input<double>("Phoneme", json, "start", error, phoneme.start)) {
            return false;
        }
        return true;
    }

    JsonValue to_json(const Phoneme &phoneme) {
        return JsonObject {
            {"token", phoneme.token},
            {"language", phoneme.language},
            {"start", phoneme.start},
        };
    }

    bool from_json(const JsonValue &json, Note &note, Error *error) {
        if (!json.isObject()) {
            if (error) {
                *error = Error(Error::InvalidFormat, "Invalid format of Note: json value is not object");
            }
            return false;
        }

        if (!get_input<int>("Note", json, "key", error, note.key)) {
            return false;
        }
        note.cents = json["cents"].toInt(); // can be omitted
        if (!get_input<int>("Note", json, "key", error, note.key)) {
            return false;
        }
        const std::string glide = json["glide"].toString(); // can be omitted;
        if (glide == "up") {
            note.glide = Glide_Up;
        } else if (glide == "down") {
            note.glide = Glide_Down;
        } else {
            note.glide = Glide_None;
        }
        if (!get_input<bool>("Note", json, "is_rest", error, note.is_rest)) {
            return false;
        }
        return true;
    }

    JsonValue to_json(const Note &note) {
        return JsonObject {
            {"key", note.key},
            {"cents", note.cents},
            {"duration", note.duration},
            {"glide", note.glide == Glide_Up ? "up" : note.glide == Glide_Down ? "down" : "none"},
            {"is_rest", note.is_rest},
        };
    }

    bool from_json(const JsonValue &json, Word &word, Error *error) {
        if (!json.isObject()) {
            if (error) {
                *error = Error(Error::InvalidFormat, "Invalid format of Word: json value is not object");
            }
            return false;
        }
        auto j_phones = json["phones"];
        if (!j_phones.isArray()) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid format of Word: missing key \"phones\" or the value is not an array");
            }
            return false;
        }
        auto j_notes = json["notes"];
        if (!j_notes.isArray()) {
            if (error) {
                *error = Error(Error::InvalidFormat,
               "Invalid format of Word: missing key \"notes\" or the value is not an array");
            }
            return false;
        }
        auto phones = j_phones.toArray();
        auto notes = j_notes.toArray();

        word.phones.reserve(phones.size());
        word.notes.reserve(notes.size());

        Error tmpError;
        for (size_t i = 0; i < phones.size(); ++i) {
            Phoneme phone;
            from_json(phones[i], phone, &tmpError);
            if (!tmpError.ok()) {
                if (error) {
                    *error = Error(tmpError.type(), tmpError.message() + " [index " + std::to_string(i) + "]");
                }
                return false;
            }
            word.phones.emplace_back(std::move(phone));
        }
        for (size_t i = 0; i < notes.size(); ++i) {
            Note note;
            from_json(notes[i], note, &tmpError);
            if (!tmpError.ok()) {
                if (error) {
                    *error = Error(tmpError.type(), tmpError.message() + " [index " + std::to_string(i) + "]");
                }
                return false;
            }
            word.notes.emplace_back(std::move(note));
        }
        return true;
    }

    JsonValue to_json(const Word &word) {
        return JsonObject {
            {"phones", to_json_array(word.phones)},
            {"notes", to_json_array(word.notes)},
        };
    }

    bool from_json(const JsonValue &json, Parameter &parameter, Error *error) {
        if (!json.isObject()) {
            if (error) {
                *error = Error(Error::InvalidFormat, "Invalid format of Parameter: json value is not object");
            }
            return false;
        }

        if (!get_input<std::string>("Parameter", json, "tag", error, parameter.tag)) {
            return false;
        }
        if (json["dynamic"].toBool(false)) {
            if (!get_input<double>("Parameter", json, "interval", error, parameter.sample_curve.timestep)) {
                return false;
            }
            auto j_values = json["values"];
            if (!j_values.isArray()) {
                if (error) {
                    *error = Error(Error::InvalidFormat, "Invalid format of Parameter: \"value\" is not array");
                }
                return false;
            }
            auto values = j_values.toArray();
            parameter.sample_curve.samples.reserve(values.size());
            for (const auto &value: values) {
                // May need to validate type of each value
                parameter.sample_curve.samples.emplace_back(value.toDouble());
            }
        } else {
            parameter.sample_curve.samples.resize(1);
            if (!get_input<double>("Parameter", json, "value", error, parameter.sample_curve.samples[0])) {
                return false;
            }
            parameter.sample_curve.timestep = json["interval"].toDouble();
        }

        auto j_retake = json["retake"];
        if (j_retake.isObject()) {
            if (!get_input("Parameter", j_retake, "start", error, parameter.retake_start)) {
                return false;
            }
            auto retake_end = j_retake["end"];
            if (retake_end.isInt()) {
                parameter.retake_end = retake_end.toInt64();
            } else {
                // missing "end", retake till end
                parameter.retake_end = parameter.sample_curve.samples.size();
            }
        } else {
            // missing "retake", retake whole range
            parameter.retake_end = 0;
            parameter.retake_end = parameter.sample_curve.samples.size();
        }
        return true;
    }

    JsonValue to_json(const Parameter &parameter) {
        return JsonObject {
            {"tag", parameter.tag},
            {"interval", parameter.sample_curve.timestep},
            {"dynamic", true},
            {"values", to_json_array(parameter.sample_curve.samples)},
            {"retake", JsonObject{
                    {"start", static_cast<int64_t>(parameter.retake_start)},
                    {"end", static_cast<int64_t>(parameter.retake_end)}
            }}
        };
    }

    bool from_json(const JsonValue &json, SpeakerMixCurve &spk, Error *error) {
        if (!json.isArray()) {
            if (error) {
                *error = Error(Error::InvalidFormat, "Invalid format of SpeakerMixCurve: json value is not array");
            }
            return false;
        }

        for (const auto &item: json.toArray()) {
            if (!item.isObject()) {
                if (error) {
                    *error = Error(Error::InvalidFormat,
                            "Invalid format of SpeakerMixCurve: json values in the array are not objects");
                }
                return false;
            }
            SampleCurve sc;
            if (item["dynamic"].toBool(false)) {
                auto j_values = item["values"];
                if (!j_values.isArray()) {
                    if (error) {
                        *error = Error(Error::InvalidFormat, "Invalid format of SpeakerMixCurve: \"value\" is not array");
                    }
                    return false;
                }
                auto values = j_values.toArray();
                sc.samples.reserve(values.size());
                for (const auto &value: values) {
                    // May need to validate type of each value
                    sc.samples.emplace_back(value.toDouble());
                }
                if (!get_input<double>("SpeakerMixCurve", item, "interval", error, sc.timestep)) {
                    return false;
                }
            } else {
                // not dynamic
                sc.samples.resize(1);
                if (!get_input<double>("SpeakerMixCurve", item, "value", error, sc.samples[0])) {
                    return false;
                }
                sc.timestep = item["interval"].toDouble();
            }
            std::string name;
            if (!get_input("SpeakerMixCurve", item, "name", error, name)) {
                return false;
            }
            spk.spk.emplace(std::move(name), std::move(sc));
        }
        return true;
    }

    JsonValue to_json(const SpeakerMixCurve &spk) {
        JsonArray j;
        for (const auto &[name, sc] : spk.spk) {
            if (sc.samples.size() == 1) {
                j.emplace_back(JsonObject {
                    {"name", name},
                    {"dynamic", false},
                    {"value", sc.samples[0]},
                });
            } else {
                j.emplace_back(JsonObject {
                    {"name", name},
                    {"dynamic", true},
                    {"interval", sc.timestep},
                    {"values", to_json_array(sc.samples)},
                });
            }
        }
        return j;
    }

    bool from_json(const JsonValue &json, Segment &segment, Error *error) {
        if (!json.isObject()) {
            if (error) {
                *error = Error(Error::InvalidFormat, "Invalid format of Segment: json value is not object");
            }
            return false;
        }

        // offset
        segment.offset = json["offset"].toDouble();

        // words
        auto j_words = json["words"];
        if (!j_words.isArray()) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid format of Segment: missing key \"words\" or the value is not an array");
            }
            return false;
        }
        auto words = j_words.toArray();
        segment.words.reserve(words.size());
        Error tmpError;
        for (size_t i = 0; i < words.size(); ++i) {
            Word word;
            from_json(words[i], word, &tmpError);
            if (!tmpError.ok()) {
                if (error) {
                    *error = Error(tmpError.type(), tmpError.message() + " [index " + std::to_string(i) + "]");
                }
                return false;
            }
            segment.words.emplace_back(std::move(word));
        }

        // parameters
        auto j_parameters = json["parameters"];
        if (j_parameters.isArray()) {
            auto parameters = j_parameters.toArray();
            for (size_t i = 0; i < parameters.size(); ++i) {
                Parameter parameter;
                from_json(parameters[i], parameter, &tmpError);
                if (!tmpError.ok()) {
                    if (error) {
                        *error = Error(tmpError.type(), tmpError.message() + " [index " + std::to_string(i) + "]");
                    }
                    return false;
                }
                segment.parameters.emplace(parameter.tag, parameter);
            }
        }

        // speakers
        auto j_speakers = json["speakers"];
        if (j_speakers.isArray()) {
            if (!from_json(json["speakers"], segment.speakers, error)) {
                return false;
            }
        }

        return true;
    }

    JsonValue to_json(const Segment &segment) {
        JsonObject j = {
            {"offset", segment.offset},
            {"words", to_json_array(segment.words)},
        };

        if (!segment.parameters.empty()) {
            JsonArray params;
            params.reserve(segment.parameters.size());
            for (const auto &p : segment.parameters) {
                params.emplace_back(to_json(p.second));
            }
            j["parameters"] = params;
        }

        if (!segment.speakers.empty()) {
            j["speakers"] = to_json(segment.speakers);
        }

        return j;
    }
}
