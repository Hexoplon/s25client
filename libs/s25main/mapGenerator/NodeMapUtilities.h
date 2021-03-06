// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "RttrForeachPt.h"
#include "world/NodeMapBase.h"
#include <cassert>
#include <cmath>
#include <functional>

namespace rttr { namespace mapGenerator {

    template<typename T_Value>
    struct ValueRange
    {
        ValueRange(T_Value minimum, T_Value maximum) : minimum(minimum), maximum(maximum)
        {
            assert(maximum >= minimum);
        }

        const T_Value minimum;
        const T_Value maximum;

        /**
         * Computes the difference between maximum and minimum value.
         *
         * @returns the difference between maximum and minimum.
         */
        T_Value GetDifference() const { return maximum - minimum; }
    };

    /**
     * Maps the specified value of a range between the minimum and maximum to the corresponding index of an array
     * of the specified size. It's important to note, that this function make sure that following ratios are equal:
     * v / (max - min) = MapRangeToIndex(v, min, max, size) / size
     *
     * @param value value to map to the new range
     * @param range minimum and maximum value
     * @param size size of the container to find an index for
     *
     * @returns an index for a container which corresponds to the specified value within the specified range.
     */
    template<typename T_Value>
    unsigned MapValueToIndex(T_Value value, const ValueRange<T_Value>& range, size_t size)
    {
        T_Value difference = range.GetDifference();

        if(difference)
        {
            double slope = 1. * (size - 1) / difference;
            return static_cast<unsigned>(round(slope * (value - range.minimum)));
        }

        return 0u;
    }

    /**
     * Sets all specified points to the specified value.
     *
     * @param values reference to the node map
     * @param points points to update
     * @param value new value applied to the points
     */
    template<typename T_Value, class T_Container>
    void SetValues(NodeMapBase<T_Value>& values, const T_Container& points, const T_Value& value)
    {
        for(const MapPoint& point : points)
        {
            values[point] = value;
        }
    }

    /**
     * Finds the maximum value of the map.
     *
     * @param values reference to the node map to search for the maximum
     * @param area container of map points to compute maximum value for
     *
     * @returns the maximum value of the map.
     */
    template<typename T_Value, class T_Container>
    const T_Value& GetMaximum(const NodeMapBase<T_Value>& values, const T_Container& area)
    {
        auto compare = [&values](const MapPoint& rhs, const MapPoint& lhs) { return values[rhs] < values[lhs]; };
        auto maximum = std::max_element(area.begin(), area.end(), compare);
        return values[*maximum];
    }

    /**
     * Computes the range of values covered by the map.
     *
     * @param values reference to the node map to compute the range for
     * @param area container with map points to compute the range for
     *
     * @returns range of values covered by the map.
     */
    template<typename T_Value, class T_Container>
    ValueRange<T_Value> GetRange(const NodeMapBase<T_Value>& values, const T_Container& area)
    {
        auto compare = [&values](const MapPoint& rhs, const MapPoint& lhs) { return values[rhs] < values[lhs]; };
        auto range = std::minmax_element(area.begin(), area.end(), compare);
        return ValueRange<T_Value>(values[*range.first], values[*range.second]);
    }

    /**
     * Finds the maximum value of the map.
     *
     * @param values reference to the node map to search for the maximum
     *
     * @returns the maximum value of the map.
     */
    template<typename T_Value>
    T_Value GetMaximum(const NodeMapBase<T_Value>& values)
    {
        std::function<T_Value(const std::vector<T_Value>&)> calcMax = [](const std::vector<T_Value>& nodes) {
            return *std::max_element(nodes.begin(), nodes.end());
        };
        return values.Map(calcMax);
    }

    /**
     * Finds the point with the maximum value on the map.
     *
     * @param values reference to the node map to search for the maximum
     *
     * @returns the point which contains the maximum value.
     */
    template<typename T_Value>
    MapPoint GetMaximumPoint(const NodeMapBase<T_Value>& values)
    {
        std::function<MapPoint(const std::vector<T_Value>&)> calcMax = [&values](const std::vector<T_Value>& nodes) {
            auto maximum = std::max_element(nodes.begin(), nodes.end());
            auto index = std::distance(nodes.begin(), maximum);
            return MapPoint(static_cast<unsigned>(index % values.GetWidth()),
                            static_cast<unsigned>(index / values.GetWidth()));
        };
        return values.Map(calcMax);
    }

    /**
     * Computes the range of values covered by the map.
     *
     * @param values reference to the node map to search for the maximum
     *
     * @returns range of values covered by the map.
     */
    template<typename T_Value>
    ValueRange<T_Value> GetRange(const NodeMapBase<T_Value>& values)
    {
        std::function<ValueRange<T_Value>(const std::vector<T_Value>&)> calcRange =
          [](const std::vector<T_Value>& nodes) {
              auto range = std::minmax_element(nodes.begin(), nodes.end());
              return ValueRange<T_Value>(*range.first, *range.second);
          };
        return values.Map(calcRange);
    }

    /**
     * Selects all points fulfilling the specified predicate.
     */
    std::vector<MapPoint> SelectPoints(const std::function<bool(const MapPoint&)>& predicate, const MapExtent& size);

}} // namespace rttr::mapGenerator
