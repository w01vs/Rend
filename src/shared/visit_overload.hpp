#ifndef VISIT_OVERLOAD_HPP
#define VISIT_OVERLOAD_HPP

template <class... Ts> struct Overload : Ts... {
    using Ts::operator()...;
};
template <class... Ts> Overload(Ts...) -> Overload<Ts...>;

#endif // VISIT_OVERLOAD_HPP