
#if !defined(UVL_VIEWS_BINARY_ADDITIONVIEW_HPP)
#define UVL_VIEWS_BINARY_ADDITIONVIEW_HPP

#include "detail/CoeffWiseTraits.hpp"
#include "uvl/views/MappedViewBase.hpp"
#include "uvl/concepts/ViewDerived.hpp"

namespace uvl::views {
namespace binary {
template <concepts::ViewDerived A, concepts::ViewDerived B>
class AdditionView;

}
template <typename A, typename B>
struct detail::ViewTraits<binary::AdditionView<A, B>>
//: public binary::detail::CoeffWiseTraits<A, B> {
//: public detail::ViewTraits<A> {
{
    using Base = detail::ViewTraits<A>;
    using extents_type = typename Base::extents_type;
    using value_type = typename Base::value_type;
    using mapping_type = typename Base::mapping_type;
};

namespace binary {
template <concepts::ViewDerived A, concepts::ViewDerived B>
class AdditionView   : public MappedViewBase<AdditionView<A, B>>
{
   public:
    using self_type = AdditionView<A, B>;
    AdditionView(const A& a, const B& b) : m_lhs(a), m_rhs(b) {}
    using traits = uvl::views::detail::ViewTraits<self_type>;
     using Base = MappedViewBase<self_type>;
     using Base::extent;
     using Base::extents;

    // using value_type = traits::value_type;
    //  using extents_type = traits::extents_type;
    //  using extents_traits = uvl::detail::ExtentsTraits<extents_type>;

    // const mapping_type& mapping() const { return derived().mapping(); }
    // const extents_type& extents() const { return derived().extents(); }
     template <typename... Args>
     auto operator()(Args&&... idxs) const {
        return m_lhs(idxs...) +
               m_rhs(idxs...);
    }

   private:
    const A& m_lhs;
    const B& m_rhs;
};  // namespace binarytemplate<typenameA,typenameB>class AdditionView

template <concepts::ViewDerived A, concepts::ViewDerived B>
AdditionView(const A& a, const B& b) -> AdditionView<A, B>;
}  // namespace binary
}  // namespace uvl::views
#endif
