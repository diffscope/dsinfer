#include "sample_curve.h"

#include <cmath>
#include <algorithm>

#include "array_util.h"

namespace dsinfer::dsinterp {

    std::vector<double>
    SampleCurve::resample(double targetTimestep, int64_t targetLength) const {
        if (samples.empty() || targetLength == 0) {
            return {};
        }
        if (samples.size() == 1) {
            std::vector<double> result(targetLength, samples[0]);
            return result;
        }
        if (timestep == 0 || targetTimestep == 0) {
            return {};
        }
        if (targetLength == 1) {
            return { samples[0] };
        }
        // Find the time duration of input samples in seconds.
        auto tMax = static_cast<double>(samples.size() - 1) * timestep;

        // Construct target time axis for interpolation.
        auto targetTimeAxis = arange(0.0, tMax, targetTimestep);

        // Construct input time axis (for interpolation).
        auto inputTimeAxis = arange(0.0, static_cast<double>(samples.size()), 1.0);
        std::transform(inputTimeAxis.begin(), inputTimeAxis.end(), inputTimeAxis.begin(),
                       [this](double value) { return value * timestep; });

        // Interpolate sample curve to target time axis
        auto targetSamples = interpolate(targetTimeAxis, inputTimeAxis, samples);

        // Resize the interpolated curve vector to target length
        auto actualLength = static_cast<int64_t>(targetSamples.size());

        if (actualLength > targetLength) {
            // Truncate vector to target length
            targetSamples.resize(targetLength);
        } else if (actualLength < targetLength) {
            // Expand vector to target length, filling last value
            auto lastValue = targetSamples.back();
            targetSamples.resize(targetLength, lastValue);
        }
        return targetSamples;
    }

    SampleCurve::SampleCurve() : samples(), timestep(0.0) {}

    SampleCurve::SampleCurve(double fillValue, int64_t targetLength, double targetTimestep)
            : samples(targetLength, fillValue), timestep(targetTimestep) {}

    SampleCurve::SampleCurve(const std::vector<double> &samples, double timestep)
            : samples(samples), timestep(timestep) {}

    SampleCurve::SampleCurve(std::vector<double> &&samples, double timestep)
            : samples(samples), timestep(timestep) {}

    SpeakerMixCurve SpeakerMixCurve::resample(double targetTimestep, int64_t targetLength) const {
        SpeakerMixCurve smc;
        smc.spk.reserve(spk.size());
        for (const auto &s : spk) {
            smc.spk[s.first] = {std::move(s.second.resample(targetTimestep, targetLength)), targetTimestep};
        }
        return smc;
    }

    void SpeakerMixCurve::resampleInPlace(double targetTimestep, int64_t targetLength) {
        for (const auto &s : spk) {
            spk[s.first] = {std::move(s.second.resample(targetTimestep, targetLength)), targetTimestep};
        }
    }

    SpeakerMixCurve SpeakerMixCurve::fromStaticMix(const std::unordered_map<std::string, double> &spk,
                                                   int64_t targetLength, double targetTimestep) {
        SpeakerMixCurve smc;
        smc.spk.reserve(spk.size());
        for (const auto &s : spk) {
            smc.spk[s.first] = SampleCurve(s.second, targetLength, targetTimestep);
        }
        return smc;
    }

}