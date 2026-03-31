```cpp
// File: static_assert_coverage.cc
// Compile with: g++ -std=c++20 -O2 -fdump-tree-all -fdump-ipa-all -fdump-rtl-all -g3 -c static_assert_coverage.cc

// Macro to generate static_assert with preserved location
#define ASSERT_SIZE(T) static_assert(sizeof(T) == sizeof(T), #T " size consistent")
#define ASSERT_ALIGN(T) static_assert(alignof(T) <= sizeof(T), #T " alignment")

// Namespace scope static_asserts
namespace ns1 {
    ASSERT_SIZE(int);
    ASSERT_ALIGN(int);
    static_assert(true, "always true");
}

// Class scope static_asserts
template<typename T>
struct Container {
    static_assert(sizeof(T) > 0, "T must be complete");
    
    template<typename U>
    struct Inner {
        static_assert(sizeof(U) >= sizeof(T), "U not smaller than T");
    };
    
    // Lambda expression (LAMBDA_EXPR in switch)
    auto get_lambda() const {
        return [this](T x) { return x; };
    }
};

// Partial specialization with static_assert
template<typename T>
struct Container<T*> {
    static_assert(sizeof(void*) == sizeof(T*), "pointer size mismatch");
};

// Function local static_asserts
template<typename... Ts>
constexpr auto sum_sizes() {
    static_assert(sizeof...(Ts) > 0, "need at least one type");
    return (sizeof(Ts) + ...);
}

// Constexpr function with static_assert
template<typename T>
constexpr T check_type() {
    static_assert(!std::is_void_v<T>, "cannot use void");
    return T{};
}

// SFINAE with static_assert
template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void process(T val) {
    static_assert(sizeof(T) <= 8, "integral too large");
}

// C++20 concept with static_assert
template<typename T>
concept Sized = requires {
    { sizeof(T) } -> std::convertible_to<std::size_t>;
};

template<Sized T>
void use_sized(T&&) {
    static_assert(Sized<T>, "must be sized");
}

// Complex template metaprogramming
template<int N>
struct Factorial {
    static_assert(N >= 0, "negative factorial");
    static constexpr int value = N * Factorial<N-1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

// Mix with other constructs that invoke tree printing
auto global_lambda = [](auto x) -> decltype(x) {
    static_assert(std::is_copy_constructible_v<decltype(x)>, "must be copyable");
    return x;
};

// Template with argument pack
template<typename... Ts>
struct Pack {
    static_assert(sizeof...(Ts) < 10, "too many types");
    
    template<typename U>
    using first = std::tuple_element_t<0, std::tuple<Ts...>>;
    
    static_assert(sizeof(first<int>) == sizeof(int), "first type check");
};

// Main function that instantiates everything
int main() {
    // Instantiate templates with static_asserts
    Container<int> c1;
    Container<double*> c2;
    Container<float>::Inner<int> inner;
    
    // Use constexpr functions
    constexpr auto size_sum = sum_sizes<int, double, char>();
    static_assert(size_sum > 0, "positive sum");
    
    // Check factorial with static_assert
    static_assert(Factorial<5>::value == 120, "factorial correct");
    
    // Use SFINAE function
    process(42);
    
    // Use concept-constrained function
    use_sized(3.14);
    
    // Instantiate pack
    Pack<int, double, char> pack;
    
    // Use lambdas
    auto lambda = c1.get_lambda();
    auto result = global_lambda(100);
    
    // More local static_asserts
    static_assert(noexcept(main()), "main is noexcept");
    static_assert(__cplusplus >= 201703L, "C++17 or later");
    
    // Mix optimization levels via attribute
    [[gnu::optimize("O0")]] 
    int unused() {
        static_assert(true, "in O0 function");
        return 0;
    }
    
    return unused();
}

// Additional namespace with more static_asserts
namespace ns2 {
    template<typename T>
    void function() {
        static_assert(std::is_default_constructible_v<T>, "default constructible");
    }
    
    // Instantiate in global scope
    template void function<int>();
}

// Ensure no dead code elimination
volatile int dummy = Factorial<3>::value;
```
