#if !defined(UVL_VIEWS_UNARY_SCALARPOWERVIEW_HPP)
#define UVL_VIEWS_UNARY_SCALARPOWERVIEW_HPP

#include <cmath>

#include "ScalarOperationView.hpp"

namespace uvl::views::unary {
namespace detail {

template <typename T>
struct pow {
    constexpr auto operator()(const T& a, const T& b) const {
        return std::pow<T>(a, b);
    };
};
}  // namespace detail

template <concepts::ViewDerived B, typename A>
using ScalarPowerView = ScalarOperationView<B, detail::pow<A>, A, true>;

}  // namespace uvl::views::unary

#endif
