```cpp
// ptree_coverage.cc
// Compile with: g++ -std=c++20 -O2 -fdump-tree-all -fdump-ipa-all -g3 ptree_coverage.cc -o ptree_coverage

// Macro to generate static_assert with unique line locations
#define ASSERT_SIZE(T, msg) static_assert(sizeof(T) >= 1, #T ": " msg)
#define ASSERT_ALIGN(T, a) static_assert(alignof(T) <= 64, #T " alignment: " #a)

// Namespace scope static_asserts (different lines)
namespace N1 {
    ASSERT_SIZE(int, "int size at namespace scope");
    ASSERT_ALIGN(int, 4);
}

// Class scope static_asserts
template<typename T>
struct Container {
    // Member static_assert dependent on template parameter
    static_assert(sizeof(T) > 0, "T must be complete in Container");
    
    // Another static_assert with different condition
    static_assert(alignof(T) > 0, "alignment positive");
    
    void method() {
        // Local static_assert inside method
        static_assert(sizeof(T) <= 256, "T too large in method");
    }
};

// Template with partial specialization and static_assert
template<typename T, typename U>
struct Pair {
    static_assert(sizeof(T) + sizeof(U) < 1024, "Pair total size limit");
};

template<typename T>
struct Pair<T, T> {
    static_assert(sizeof(T) * 2 < 512, "Specialized Pair size limit");
};

// Lambda expressions (LAMBDA_EXPR nodes)
auto lambda1 = []() {
    static_assert(sizeof(int) == 4, "int size in lambda");
    return 42;
};

// Constexpr function with static_assert
template<typename T>
constexpr bool check_type() {
    static_assert(sizeof(T) > 0, "in constexpr function");
    return true;
}

// SFINAE with static_assert
template<typename T, typename = std::enable_if_t<sizeof(T) == 4>>
struct SFINAECheck {
    static_assert(alignof(T) % 2 == 0, "even alignment required");
};

// C++20 concept with static_assert
template<typename T>
concept Sized = requires {
    { sizeof(T) } -> std::convertible_to<std::size_t>;
};

template<Sized T>
void concept_func(T) {
    static_assert(sizeof(T) < 100, "concept function size check");
}

// Function with attribute to trigger different optimization handling
__attribute__((optimize("O0")))
void mixed_optimization_func() {
    // Static assert in function with different optimization
    static_assert(sizeof(double) == 8, "double size in O0 function");
    
    // Lambda inside function
    auto inner_lambda = []() {
        static_assert(alignof(long) >= 4, "long alignment in nested lambda");
        return 3.14;
    };
}

// Template instantiation that forces evaluation
template struct Container<int>;
template struct Container<double>;
template struct Pair<int, char>;
template struct Pair<float, float>;

// Main function that uses everything
int main() {
    // Force template instantiation
    Container<short> c1;
    Container<long long> c2;
    
    // Use lambdas
    auto result1 = lambda1();
    auto lambda2 = [](auto x) {
        static_assert(sizeof(decltype(x)) > 0, "generic lambda");
        return x;
    };
    
    // Call constexpr function
    constexpr bool b1 = check_type<int>();
    constexpr bool b2 = check_type<char>();
    
    // Call function with mixed optimization
    mixed_optimization_func();
    
    // Use concepts if available
    #if __cpp_concepts >= 201907L
    concept_func(42);
    concept_func(3.14);
    #endif
    
    // Simple computation to avoid dead code elimination
    return lambda1() - 42;
}
```
