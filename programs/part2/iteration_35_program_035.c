```cpp
// coverage-ptree-static_assert.cc
// Compile with: g++ -std=c++20 -O1 -fdump-tree-all -fdump-lang-all coverage-ptree-static_assert.cc

#include <type_traits>

namespace {

// Feature 1: Template with variadic parameters for ARGUMENT_PACK_SELECT
template<typename T, typename... Args>
struct Checker {
    // Feature 2: static_assert with dependent condition
    static constexpr void verify() {
        static_assert(sizeof(T) > 0, "Type must have non-zero size");
        static_assert(sizeof...(Args) < 5, "Too many template arguments");
        
        // Feature 3: Use sizeof... in static_assert condition
        static_assert(sizeof...(Args) == sizeof...(Args), "Sizeof pack consistency");
        
        // Feature 4: static_assert with constexpr function
        constexpr bool is_integral = std::is_integral_v<T>;
        static_assert(!is_integral || sizeof(T) <= 8, 
                     "Integral types must be 8 bytes or less");
    }
    
    // Feature 5: Lambda expression inside template
    auto get_lambda() {
        return [this](T val) -> bool {
            // Feature 6: noexcept expression (can generate DEFERRED_NOEXCEPT)
            static_assert(noexcept(val + val), "Operation should be noexcept");
            return val > T{};
        };
    }
    
    // Feature 7: Method with requires clause (C++20)
    template<typename U>
    requires (sizeof(U) == sizeof(T))
    void check_type() {
        static_assert(std::is_same_v<T, U> || sizeof(T) == sizeof(U),
                     "Types must be same or same size");
    }
};

// Feature 8: Trait expression usage
template<typename T>
concept HasSize = requires(T t) {
    { sizeof(t) } -> std::same_as<size_t>;
};

// Feature 9: Class with static_assert in member declaration
template<bool B>
struct AssertHolder {
    static_assert(B, "Template parameter must be true");
    
    // Feature 10: Multiple static_asserts with different conditions
    static_assert(sizeof(int) == 4, "int must be 4 bytes on this platform");
    
    void method() {
        // Feature 11: static_assert in method body
        static_assert(std::is_void_v<void>, "Sanity check");
    }
};

// Feature 12: Function template with static_assert
template<typename... Ts>
constexpr size_t count_types() {
    static_assert(sizeof...(Ts) > 0, "Must have at least one type");
    return sizeof...(Ts);
}

// Feature 13: Variable template with static_assert
template<typename T>
constexpr bool always_true = []{
    static_assert(std::is_object_v<T>, "T must be object type");
    return true;
}();

// Explicit instantiations to force code generation
template struct Checker<int, char, double>;
template struct Checker<double, float>;
template struct AssertHolder<true>;

// Feature 14: Different static_assert forms
struct TestStruct {
    // C++17 static_assert without message
    static_assert(std::is_standard_layout_v<TestStruct>);
    
    int value;
    
    // Feature 15: static_assert with complex condition
    static_assert(noexcept(TestStruct{}.value) && 
                  std::is_trivial_v<TestStruct>);
};

// Feature 16: Namespace with static_assert
namespace Inner {
    template<typename T>
    void inner_function() {
        static_assert(std::is_copy_constructible_v<T>, 
                     "Type must be copy constructible");
    }
}

} // anonymous namespace

// Main function to ensure everything is referenced
int main() {
    // Instantiate and use templates
    Checker<int, char, double>::verify();
    
    Checker<int, char> checker;
    auto lambda = checker.get_lambda();
    bool result = lambda(42);
    
    // Use concepts/traits
    static_assert(HasSize<int>, "int has size");
    
    // Call variadic function template
    constexpr auto count = count_types<int, double, char>();
    static_assert(count == 3, "Should count 3 types");
    
    // Use variable template
    static_assert(always_true<int>, "Should be true");
    
    // Instantiate AssertHolder
    AssertHolder<true> holder;
    holder.method();
    
    // Call inner namespace function
    Inner::inner_function<double>();
    
    return result ? 0 : 1;
}

// Feature 17: Global static_assert
static_assert(__cplusplus >= 201703L, "Requires C++17 or later");

// Feature 18: static_assert depending on compiler builtins
static_assert(__has_cpp_attribute(deprecated) == 201309L, 
             "deprecated attribute should be available");
```
