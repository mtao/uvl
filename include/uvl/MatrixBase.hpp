#if !defined(UVL_MATRIXBASE_HPP)
#define UVL_MATRIXBASE_HPP

#include "ArrayBase.hpp"
#include "uvl/detail/convert_extents.hpp"
#include "uvl/types.hpp"
//
#include "concepts/MatrixBaseDerived.hpp"
#include "concepts/MatrixViewDerived.hpp"
#include "concepts/VectorBaseDerived.hpp"
//
#include "uvl/detail/declare_operations.hpp"
#include "uvl/views/binary/ArithmeticViews.hpp"
#include "views/binary/MatrixProductView.hpp"
#include "views/binary/MatrixVectorProductView.hpp"
#include "views/reductions/CoefficientSum.hpp"
#include "views/reductions/Any.hpp"
#include "views/reductions/All.hpp"
#include "views/reductions/Trace.hpp"
//#include "views/reductions/Determinant.hpp"
#include "views/unary/CastView.hpp"
#include "views/unary/DiagonalView.hpp"
#include "views/unary/IdentityView.hpp"
#include "views/unary/ScalarArithmeticViews.hpp"
#include "views/unary/SliceView.hpp"
#include "views/unary/SwizzleView.hpp"
#include "views/unary/detail/operation_implementations.hpp"

namespace uvl {
template <concepts::ViewDerived View>
class ArrayBase;

template <concepts::MatrixViewDerived View>
class MatrixBase {
   public:
    MatrixBase() = default;

    using view_type = View;
    using value_type = View::value_type;
    using extents_type = View::extents_type;
    auto eval() const { return Matrix(*this); }
    template <typename... Args>
    MatrixBase(Args&&... v) : m_view(std::forward<Args>(v)...) {}

    MatrixBase(View&& v) : m_view(v) {}
    MatrixBase(const View& v) : m_view(v) {}
    MatrixBase& operator=(concepts::ViewDerived auto const& v) { m_view = v; return *this;}
    MatrixBase& operator=(concepts::MatrixBaseDerived auto const& v) { m_view = v.view(); return *this; }

    MatrixBase(MatrixBase&& v) = default;
    MatrixBase(const MatrixBase& v) = default;

    template <concepts::MatrixViewDerived Other>
    MatrixBase(const Other& other)
        requires(view_type::is_writable)
        : m_view(detail::convert_extents<extents_type>(other.extents())) {
        m_view.assign(other);
    }
    template <concepts::MatrixViewDerived Other>
    MatrixBase& operator=(const Other& other)
        requires(view_type::is_writable)
    {
        m_view.assign(other);
        return *this;
    }
    template <concepts::MatrixBaseDerived Other>
    MatrixBase(const Other& other)
        requires(view_type::is_writable)
        : MatrixBase(other.view()) {}

    template <concepts::MatrixBaseDerived Other>
    MatrixBase& operator=(const Other& other)
        requires(view_type::is_writable)
    {
        return operator=(other.view());
    }

    MatrixBase& operator*=(const value_type& other)
        requires(view_type::is_writable)
    {
        return *this = other * *this;
    }
    MatrixBase& operator/=(const value_type& other)
        requires(view_type::is_writable)
    {
        return *this = *this / other;
    }
    template <concepts::MatrixBaseDerived Other>
    MatrixBase& operator+=(const Other& other)
        requires(view_type::is_writable)
    {
        return *this = *this + other;
    }
    template <concepts::MatrixBaseDerived Other>
    MatrixBase& operator-=(const Other& other)
        requires(view_type::is_writable)
    {
        return *this = *this - other;
    }

    auto as_array() const {
        return ArrayBase<views::unary::IdentityView<View>>(view());
    }

    template <typename T>
    auto cast() const {
        return MatrixBase<views::unary::CastView<T, view_type>>(view());
    }

    template <rank_type... ranks>
    auto swizzle() const {
        return MatrixBase<views::unary::SwizzleView<view_type, ranks...>>(
            views::unary::SwizzleView<view_type, ranks...>(view()));
    }
    auto transpose() const { return swizzle<1, 0>(); }

    template <typename... Slices>
    auto slice(Slices&&... slices) const {
        using view_type = views::unary::SliceView<view_type, true, Slices...>;

        constexpr static rank_type rank = view_type::extents_type::rank();
        if constexpr (rank == 1) {
            return VectorBase<view_type>(
                view_type(view(), std::forward<Slices>(slices)...));
        } else if constexpr (rank == 2) {
            return MatrixBase<view_type>(
                view_type(view(), std::forward<Slices>(slices)...));
        }
    }
    template <typename... Slices>
    auto slice() const {
        using view_type = views::unary::SliceView<view_type, true, Slices...>;
        constexpr static rank_type rank = view_type::extents_type::rank();
        if constexpr (rank == 1) {
            return VectorBase<view_type>(view_type(view(), Slices{}...));
        } else if constexpr (rank == 2) {
            return MatrixBase<view_type>(view_type(view(), Slices{}...));
        }
    }

    template <typename... Slices>
    auto slice(Slices&&... slices) {
        using view_type = views::unary::SliceView<view_type, false, Slices...>;

        constexpr static rank_type rank = view_type::extents_type::rank();
        if constexpr (rank == 1) {
            return VectorBase<view_type>(
                view_type(view(), std::forward<Slices>(slices)...));
        } else if constexpr (rank == 2) {
            return MatrixBase<view_type>(
                view_type(view(), std::forward<Slices>(slices)...));
        }
    }
    template <typename... Slices>
    auto slice() {
        using view_type = views::unary::SliceView<view_type, false, Slices...>;
        constexpr static rank_type rank = view_type::extents_type::rank();
        if constexpr (rank == 1) {
            return VectorBase<view_type>(view_type(view(), Slices{}...));
        } else if constexpr (rank == 2) {
            return MatrixBase<view_type>(view_type(view(), Slices{}...));
        }
    }

    auto diagonal() const {
        return VectorBase<views::unary::DiagonalView<view_type, true>>(view());
    }
    auto diagonal() {
        return VectorBase<views::unary::DiagonalView<view_type, false>>(view());
    }

    value_type trace() const {
        return views::reductions::Trace(view())();
    }

    template <typename Slice>
    auto row() {
        return slice<Slice, full_extent_t>();
    }
    template <typename Slice>
    auto col() {
        return slice<full_extent_t, Slice>();
    }

    template <typename Slice>
    auto row() const {
        return slice<Slice, full_extent_t>();
    }
    template <typename Slice>
    auto col() const {
        return slice<full_extent_t, Slice>();
    }

    template <typename Slice>
    auto row(Slice&& s) {
        return slice<Slice, full_extent_t>(std::forward<Slice>(s),
                                           full_extent_t{});
    }
    template <typename Slice>
    auto col(Slice&& s) {
        return slice<full_extent_t, Slice>(full_extent_t{},
                                           std::forward<Slice>(s));
    }

    template <typename Slice>
    auto row(Slice&& s) const {
        return slice<Slice, full_extent_t>(full_extent_t{},
                                           std::forward<Slice>(s));
    }
    template <typename Slice>
    auto col(Slice&& s) const {
        return slice<full_extent_t, Slice>(std::forward<Slice>(s),
                                           full_extent_t{});
    }


    /*
    template <index_type... Indices,
              typename mapping_type = std::experimental::layout_left>
    auto reshape(const uvl::extents<Indices...>& e,
                 mapping_type map = {}) const {
        if constexpr (sizeof...(Indices) == 1 &&
                      concepts::MappedViewDerived<view_type>) {
            return nullptr;
        } else if constexpr (extents_type::rank == 1) {
        }
        // return MatrixBase<views::unary::CastView<T, view_type>>(view());
    }
    */

    template <typename... Args>
    value_type operator()(Args&&... idxs) const

    {
        return view()(std::forward<Args>(idxs)...);
    }

    const View& view() const { return m_view; }
    View& view() { return m_view; }
    const extents_type& extents() const { return view().extents(); }
    constexpr index_type extent(rank_type i) const { return m_view.extent(i); }

   private:
    View m_view;
};

template <concepts::MatrixViewDerived View>
MatrixBase(View&& view) -> MatrixBase<View>;
template <concepts::MatrixViewDerived View>
MatrixBase(const View& view) -> MatrixBase<View>;

UNARY_DECLARATION(MatrixBase, LogicalNot, operator!)
UNARY_DECLARATION(MatrixBase, BitNot, operator~)
UNARY_DECLARATION(MatrixBase, Negate, operator-)

SCALAR_BINARY_DECLARATION(MatrixBase, Divides, operator/)

BINARY_DECLARATION(MatrixBase, Plus, operator+)
BINARY_DECLARATION(MatrixBase, Minus, operator-)
//BINARY_DECLARATION(MatrixBase, EqualsTo, operator==)
//BINARY_DECLARATION(MatrixBase, NotEqualsTo, operator!=)
//BINARY_DECLARATION(MatrixBase, Greater, operator>)
//BINARY_DECLARATION(MatrixBase, Less, operator<)
//BINARY_DECLARATION(MatrixBase, GreaterEqual, operator>=)
//BINARY_DECLARATION(MatrixBase, LessEqual, operator<=)
//BINARY_DECLARATION(MatrixBase, LogicalAnd, operator&&)
//BINARY_DECLARATION(MatrixBase, LogicalOr, operator||)
//BINARY_DECLARATION(MatrixBase, BitAnd, operator&)
//BINARY_DECLARATION(MatrixBase, BitOr, operator|)
//BINARY_DECLARATION(MatrixBase, BitXor, operator^)
//
template <concepts::MatrixBaseDerived View1, concepts::MatrixBaseDerived View2>
bool operator==(View1 const& lhs, View2 const& rhs) {
    return (lhs.as_array() == rhs.as_array()).all();
}

template <concepts::MatrixBaseDerived View1, concepts::MatrixBaseDerived View2>
bool operator!=(View1 const& lhs, View2 const& rhs) {
    return (lhs.as_array() != rhs.as_array()).any();
}

template <concepts::MatrixBaseDerived View1, concepts::MatrixBaseDerived View2>
auto operator*(View1 const& lhs, View2 const& rhs) {
    return MatrixBase<views::binary::MatrixProductView<
        typename View1::view_type, typename View2::view_type>>(lhs.view(),
                                                               rhs.view());
}

template <concepts::MatrixBaseDerived View>
auto operator*(View const& lhs, typename View::value_type const& rhs) {
    return MatrixBase<views::unary::ScalarMultipliesView<
        typename View::value_type, View, true>>(lhs.view(), rhs);
}
template <concepts::MatrixBaseDerived View>
auto operator*(typename View::value_type const& lhs, View const& rhs) {
    return MatrixBase<views::unary::ScalarMultipliesView<
        typename View::value_type, View, false>>(lhs, rhs.view());
}

template <concepts::MatrixBaseDerived View1, concepts::VectorBaseDerived View2>
auto operator*(View1 const& lhs, View2 const& rhs) {
    return VectorBase<views::binary::MatrixVectorProductView<
        typename View1::view_type, typename View2::view_type>>(lhs.view(),
                                                               rhs.view());
}

}  // namespace uvl

// namespace uvl::views {

// template <typename ValueType, index_type Rows, index_type Cols>
// struct detail::ViewTraits<Matrix<ValueType, Rows, Cols>>
//     : public detail::ViewTraits<
//           uvl::storage::PlainObjectStorage<ValueType, extents<Rows, Cols>>>
//           {};
// }  // namespace uvl::views
// template <concepts::MatrixViewDerived View>
// struct detail::ViewTraits<MatrixBase<view_type>> : public
// detail::ViewTraits<View>
// {
// };
#endif
