//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

/// @file egolib/Math/Translate_Ray.hpp
/// @brief Translation of rays.
/// @author Michael Heilmann

#pragma once

#include "egolib/Math/Ray.hpp"

namespace Ego {
namespace Math {

template <typename _EuclideanSpaceType>
struct Translate<Ray<_EuclideanSpaceType>> {
    typedef Ray<_EuclideanSpaceType> X;
    typedef typename _EuclideanSpaceType::VectorType T;
    X operator()(const X& x, const T& t) const {
        return X(x.getOrigin() + t, x.getDirection());
    }
};

template <typename _EuclideanSpaceType>
Ray<_EuclideanSpaceType> translate(const Ray<_EuclideanSpaceType>& x, const typename _EuclideanSpaceType::VectorType& t) {
    Translate<Ray<_EuclideanSpaceType>> f;
    return f(x, t);
}

} // namespace Math
} // namespace Ego
