```cpp
// ptree_coverage.cc
// Compile with: g++ -std=c++20 -O1 -fdump-tree-all -fdump-lang-all ptree_coverage.cc -o ptree_coverage

#include <type_traits>

// Feature 1: Custom struct for template testing
struct MyStruct {
    int x;
    double y;
};

// Feature 2: constexpr function for static_assert condition
template<typename T>
constexpr bool always_true() {
    return true;
}

// Feature 3: Template with variadic parameters and requires clause (C++20)
template<typename T, typename... Args>
requires (sizeof...(Args) > 0) // Creates DEFERRED_NOEXCEPT/TRAIT_EXPR nodes
class Checker {
public:
    // Feature 4: Static assert with dependent condition using sizeof...
    static void verify() {
        static_assert(sizeof(T) >= sizeof(int), "Type size check failed");
        static_assert(sizeof...(Args) <= 5, "Too many template arguments");
        static_assert(always_true<T>(), "Constexpr function check");
        
        // Feature 5: Lambda expression inside template
        auto lambda = [](T& value) -> bool {
            return sizeof(value) > 0;
        };
        
        T dummy{};
        lambda(dummy);
    }
    
    // Feature 6: Method with noexcept expression
    template<typename U>
    auto test_noexcept(U&& val) noexcept(noexcept(val + val)) {
        return val + val;
    }
    
    // Feature 7: Static assert in a nested context
    struct Nested {
        static constexpr bool check() {
            static_assert(std::is_arithmetic_v<T>, "Must be arithmetic");
            return true;
        }
    };
};

// Feature 8: Multiple explicit instantiations
template class Checker<int, double, char>;
template class Checker<MyStruct, int, float, bool>;
template class Checker<double, int, short, long, char, bool>;

// Feature 9: Function template with static_assert
template<typename T>
void process_value(T value) {
    static_assert(std::is_copy_constructible_v<T>, "Must be copyable");
    
    // Another lambda with capture
    auto printer = [value]() {
        static_assert(sizeof(value) > 0, "Size check in lambda");
        return sizeof(value);
    };
    
    printer();
}

// Feature 10: Class with static_assert in inline method
struct Container {
    template<typename U>
    static void validate() {
        static_assert(std::is_default_constructible_v<U>, 
                     "Must be default constructible");
    }
};

// Feature 11: Anonymous namespace for "multi-file" illusion
namespace {
    template<typename... Ts>
    struct HiddenChecker {
        static constexpr bool value = (sizeof(Ts) + ...) > 0;
        
        static void check() {
            static_assert(value, "Pack size check");
            static_assert((std::is_trivial_v<Ts> && ...), "All must be trivial");
        }
    };
    
    // Instantiate in anonymous namespace
    HiddenChecker<int, double>::check();
}

// Feature 12: __attribute__((used)) to prevent optimization
template<typename T>
__attribute__((used)) void unused_but_present() {
    static_assert(alignof(T) <= 16, "Alignment check");
}

// Main function to ensure everything is referenced
int main() {
    // Instantiate and use templates
    Checker<int, double, char>::verify();
    Checker<MyStruct, int, float, bool>::verify();
    
    process_value(42);
    process_value(3.14);
    
    Container::validate<int>();
    
    // Use the noexcept method
    Checker<int, double> checker;
    checker.test_noexcept(10);
    
    // Force instantiation of attribute-marked function
    unused_but_present<int>();
    
    return 0;
}

// Feature 13: Global static_assert (different AST context)
static_assert(__cplusplus >= 201703L, "Requires C++17 or later");

// Feature 14: Static_assert with trait expression
template<typename T>
struct TypeTester {
    static_assert(std::is_class_v<T> || std::is_arithmetic_v<T>, 
                 "Must be class or arithmetic");
};

// Explicit instantiation
template struct TypeTester<MyStruct>;
```
