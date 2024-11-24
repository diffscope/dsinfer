#ifndef ARRAY_UTIL_H
#define ARRAY_UTIL_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <sstream>

namespace dsinfer::dsinterp {
    enum InterpolationMethod {
		InterpolateLinear,
		InterpolateCubicSpline,
		InterpolateNearestNeighbor,
		InterpolateNearestNeighborUp
	};

	/**
	 * @brief Performs array interpolation for given sample points and function values.
	 *
	 * @param samplePoints           The sample points to interpolate. (ascending, no duplicate)
	 * @param referencePoints        The reference points of the function values. (ascending, no duplicate)
	 * @param referenceValues        The function values corresponding to the reference points.
	 * @param interpolationMethod    The optional interpolation method.
	 *                               Can be InterpolateLinear (default), InterpolateCubicSpline,
	 *                               InterpolateNearestNeighbor, InterpolateNearestNeighborUp.
	 * @param leftFillValue          The optional fill value for elements in samplePoints
	 *                               less than referencePoints[0]. Defaults to NaN.
	 * @param rightFillValue         The optional fill value for elements in samplePoints
	 *                               greater than referencePoints[n-1]. Defaults to NaN.
	 * @return                       A vector of interpolated values corresponding to the
	 *                               sample points samplePoints.
	 *
	 * The interpolate function performs array interpolation using the specified interpolation method.
	 * It takes the sample points samplePoints, the corresponding reference points
	 * referencePoints, and the function values referenceValues, and returns a vector
	 * of interpolated values corresponding to the sample points samplePoints. The
	 * interpolation is performed based on the provided reference points referencePoints
	 * and function values referenceValues.
	 *
	 * If referencePoints has only one element, the interpolation reduces to assigning
	 * the corresponding reference value to all elements of samplePoints. If referencePoints
	 * has more than one element, the function iterates over each element of samplePoints
	 * and performs interpolation using linear interpolation between the neighboring
	 * points in referencePoints and referenceValues.
	 *
	 * The optional leftFillValue and rightFillValue arguments specify fill values for
	 * elements in samplePoints that fall outside the range of referencePoints. If
	 * leftFillValue or rightFillValue is not provided, NaN is used as the fill value.
	 * If an element in samplePoints is NaN, the corresponding element in the interpolated
	 * vector is also set to NaN.
	 */
	template<class T>
	inline std::vector<T> interpolate(
			const std::vector<T> &samplePoints,
			const std::vector<T> &referencePoints,
			const std::vector<T> &referenceValues,
			InterpolationMethod interpolationMethod = InterpolateLinear,
			T leftFillValue = std::nan(""),
			T rightFillValue = std::nan(""));

	template<class T>
	inline std::vector<T> arange(T start, T stop, T step);

	template<class T>
	std::vector<T> splitString(const std::string &str);

	/**
	 * @brief Performs linear interpolation between two points.
	 *
	 * @param x0 The x-coordinate of the first point.
	 * @param y0 The y-coordinate of the first point.
	 * @param x1 The x-coordinate of the second point.
	 * @param y1 The y-coordinate of the second point.
	 * @param samplePoint The x-coordinate at which to interpolate.
	 * @return The interpolated y-coordinate.
	 */
	template<class T>
	inline T interpolatePointLinear(T x0, T y0, T x1, T y1, T x);

	/**
	 * @brief Performs cubic spline interpolation between two points.
	 *
	 * @param x0 The x-coordinate of the first point.
	 * @param y0 The y-coordinate of the first point.
	 * @param x1 The x-coordinate of the second point.
	 * @param y1 The y-coordinate of the second point.
	 * @param samplePoint The x-coordinate at which to interpolate.
	 * @return The interpolated y-coordinate.
	 */
	template<class T>
	inline T interpolatePointCubicSpline(T x0, T y0, T x1, T y1, T x);

	template<class T>
	inline T interpolateNearestNeighbor(T x0, T y0, T x1, T y1, T x);

	template<class T>
	inline T interpolateNearestNeighborUp(T x0, T y0, T x1, T y1, T x);

	/* IMPLEMENTATION BELOW */

	template<class T>
	T interpolatePointLinear(T x0, T y0, T x1, T y1, T x) {
		return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
	}

	template<class T>
	T interpolatePointCubicSpline(T x0, T y0, T x1, T y1, T x) {
		auto h = x1 - x0;
		auto t = (x - x0) / h;
		auto t2 = t * t;
		auto t3 = t2 * t;

		auto a = 2 * t3 - 3 * t2 + 1;
		auto b = t3 - 2 * t2 + t;
		auto c = -2 * t3 + 3 * t2;
		auto d = t3 - t2;

		return a * y0 + b * h * (y1 - y0) + c * y1 + d * h * (y1 - y0);
	}

	template<class T>
	T interpolateNearestNeighbor(T x0, T y0, T x1, T y1, T x) {
		return ((x - x0) > (x1 - x)) ? y1 : y0;
	}

	template<class T>
	T interpolateNearestNeighborUp(T x0, T y0, T x1, T y1, T x) {
		return ((x - x0) >= (x1 - x)) ? y1 : y0;
	}

	template<class T>
	std::vector<T> interpolate(
			const std::vector<T> &samplePoints,
			const std::vector<T> &referencePoints,
			const std::vector<T> &referenceValues,
			InterpolationMethod interpolationMethod,
			T leftFillValue,
			T rightFillValue) {

		std::vector<T> interpolatedValues;
		interpolatedValues.reserve(samplePoints.size());

		for (const auto &samplePoint: samplePoints) {
			if (samplePoint < referencePoints.front() || samplePoint > referencePoints.back()) {
				interpolatedValues.push_back(samplePoint < referencePoints.front() ? leftFillValue : rightFillValue);
			} else {
				size_t index = 0;
				while (referencePoints[index] < samplePoint) {
					++index;
				}

				if (referencePoints[index] == samplePoint) {
					interpolatedValues.push_back(referenceValues[index]);
				} else {
					auto x0 = referencePoints[index - 1];
					auto x1 = referencePoints[index];
					auto y0 = referenceValues[index - 1];
					auto y1 = referenceValues[index];
					T interpolatedValue;
					switch (interpolationMethod) {
						case InterpolateLinear:
							interpolatedValue = interpolatePointLinear(x0, y0, x1, y1, samplePoint);
							break;
						case InterpolateCubicSpline:
							interpolatedValue = interpolatePointCubicSpline(x0, y0, x1, y1, samplePoint);
							break;
						case InterpolateNearestNeighbor:
							interpolatedValue = interpolateNearestNeighbor(x0, y0, x1, y1, samplePoint);
							break;
						case InterpolateNearestNeighborUp:
							interpolatedValue = interpolateNearestNeighborUp(x0, y0, x1, y1, samplePoint);
							break;
						default:
							interpolatedValue = interpolatePointLinear(x0, y0, x1, y1, samplePoint);
							break;
					}
					interpolatedValues.push_back(interpolatedValue);
				}
			}
		}

		return interpolatedValues;
	}

	template<class T>
	std::vector<T> arange(T start, T stop, T step) {
		if ((stop < start) && (step > 0)) {
			return {};
		}
		auto size = static_cast<size_t>(std::ceil((stop - start) / step));
		if (size == 0) {
			return {};
		}

		std::vector<T> result;
		result.reserve(size);

		for (size_t i = 0; i < size; ++i) {
			result.push_back(start + i * step);
		}

		return result;
	}

	template<class T>
	std::vector<T> splitString(const std::string &str) {
		std::istringstream iss(str);
		std::vector<T> tokens;
		T token;

		while (iss >> token) {
			tokens.push_back(token);
		}

		return tokens;
	}
}

#endif