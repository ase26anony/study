```cpp
// File: static_assert_coverage.cc
// Compile with: g++ -std=c++20 -O2 -fdump-tree-all -fdump-ipa-all -fdump-rtl-all -c static_assert_coverage.cc

// Macro to generate static_assert with unique locations
#define ASSERT_SIZE(T, msg) static_assert(sizeof(T) == sizeof(T), #T ": " msg)
#define ASSERT_ALIGN(T, a) static_assert(alignof(T) == a, "Alignment check for " #T)

// Namespace scope static_asserts (different lines)
namespace ns1 {
    ASSERT_SIZE(int, "int size consistent");
    ASSERT_ALIGN(int, 4);
}

namespace ns2 {
    template<typename T>
    struct TypeChecker {
        // Class scope static_assert
        static_assert(sizeof(T) >= 1, "Type must be complete");
        
        // Template-dependent static_assert
        static constexpr bool always_true = true;
        static_assert(always_true, "Template-dependent assertion");
    };
    
    // Instantiate to force instantiation
    template struct TypeChecker<int>;
}

// Lambda expressions (LAMBDA_EXPR in switch)
auto lambda1 = []() {
    // Local scope static_assert inside lambda
    static_assert(sizeof(int) == 4, "int size in lambda");
    return 42;
};

// Complex template metaprogramming with static_assert
template<typename... Ts>
struct PackChecker {
    // Fold expression in static_assert
    static_assert((sizeof(Ts) + ...) > 0, "Total size must be positive");
    
    template<typename U>
    static constexpr bool check() {
        // static_assert in constexpr function template
        static_assert(sizeof(U) <= 8, "Size limit in constexpr function");
        return true;
    }
};

// Partial specialization with static_assert
template<typename T>
struct Specialized {
    static_assert(sizeof(T) > 0, "General case");
};

template<>
struct Specialized<void> {
    // Different line for specialization
    static_assert(1 == 1, "void specialization");
};

// C++20 concepts with static_assert
template<typename T>
concept HasSizeFour = sizeof(T) == 4;

template<typename T>
requires HasSizeFour<T>
struct Constrained {
    static_assert(HasSizeFour<T>, "Concept-satisfied assertion");
};

// Mix with TRAIT_EXPR nodes (noexcept, is_same, etc.)
template<typename T>
void trait_example() noexcept(sizeof(T) == 4) {
    static_assert(noexcept(trait_example<T>()), "Noexcept check");
}

// Function with attribute to potentially trigger dumps
__attribute__((optimize("O0")))
void mixed_optimization() {
    // Local static_assert
    static_assert(alignof(long) >= 4, "long alignment");
    
    // Lambda with static_assert
    auto lambda2 = [](auto x) {
        static_assert(sizeof(decltype(x)) > 0, "Generic lambda assertion");
        return x;
    };
    
    lambda2(3.14);
}

// SFINAE with static_assert
template<typename T, typename = std::enable_if_t<sizeof(T) == 4>>
struct SFINAE_Example {
    static_assert(std::is_same_v<T, int> || std::is_same_v<T, float>, 
                  "Only int or float allowed");
};

// Main function that uses everything
int main() {
    // Force instantiation and usage
    ns2::TypeChecker<double> tc;
    PackChecker<int, float, char> pc;
    Specialized<int> s1;
    Specialized<void> s2;
    
    if constexpr (HasSizeFour<int>) {
        Constrained<int> c;
    }
    
    trait_example<int>();
    mixed_optimization();
    
    // Use lambda to prevent optimization
    auto result = lambda1() + []() {
        static_assert(sizeof(char) == 1, "char size in nested lambda");
        return 1;
    }();
    
    return result - 43; // Returns 0
}

// Additional static_asserts at file scope
ASSERT_SIZE(double, "double size");
static_assert(__cplusplus >= 201703L, "C++17 or later required");

// Template with argument pack selection (ARGUMENT_PACK_SELECT)
template<typename... Ts>
struct LastType {
    using type = typename std::tuple_element_t<sizeof...(Ts) - 1, std::tuple<Ts...>>;
    static_assert(sizeof...(Ts) > 0, "Pack must not be empty");
};

// Instantiate with various types
template struct LastType<int, float, double>;
```
