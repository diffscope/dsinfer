#ifndef PROJECT_H
#define PROJECT_H

#include <map>
#include <vector>

#include <dsinfer/error.h>
#include <dsinfer/jsonvalue.h>

#include "sample_curve.h"
#include "speaker_embed.h"

namespace dsinfer::dsinterp {
    enum GlideType {
        Glide_None = 0,
        Glide_Up = 1,
        Glide_Down = 2,
    };

    struct Phoneme {
        std::string token;
        std::string language;
        double start = 0.0;
    };

    struct Note {
        int key = 0;
        int cents = 0;
        double duration = 0.0;
        GlideType glide = Glide_None;
        bool is_rest = false;
    };

    struct Word {
        std::vector<Phoneme> phones;
        std::vector<Note> notes;

        double duration() const;
    };

    struct Parameter {
        std::string tag;
        SampleCurve sample_curve;
        size_t retake_start = 0;
        size_t retake_end = 0;
    };

    struct Segment {
        int64_t context = -1;
        std::vector<Word> words;
        std::map<std::string, Parameter> parameters;
        SpeakerMixCurve speakers;

        size_t phoneCount() const;
        size_t noteCount() const;
    };

    inline size_t Segment::phoneCount() const {
        size_t phoneCount = 0;
        for (const auto &word : words) {
            phoneCount += word.phones.size();
        }
        return phoneCount;
    }

    inline size_t Segment::noteCount() const {
        size_t noteCount = 0;
        for (const auto &word : words) {
            noteCount += word.notes.size();
        }
        return noteCount;
    }

    inline double Word::duration() const {
        double totalDuration = 0.0;
        for (const auto &note : notes) {
            totalDuration += note.duration;
        }
        return totalDuration;
    }
}

#endif // PROJECT_H