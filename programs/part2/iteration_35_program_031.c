```cpp
// ptree_coverage.cc
// Compile with: g++ -std=c++20 -O0 -fdump-lang-all -c ptree_coverage.cc

#include <type_traits>

namespace {
    // Custom type to use in templates
    struct MyType {
        int value;
        constexpr MyType(int v) : value(v) {}
    };

    // Helper constexpr function for static_assert condition
    template<typename T>
    constexpr bool is_large_enough() {
        return sizeof(T) >= sizeof(int);
    }

    // Variadic template with multiple features
    template<typename T, typename... Args>
    requires (sizeof...(Args) > 0) // C++20 requires-clause
    class Checker {
    public:
        // Static member function with static_assert
        static constexpr void verify() {
            // Static assert with non-trivial dependent condition
            static_assert(is_large_enough<T>(), "Type T is too small");
            
            // Another static_assert using sizeof...
            static_assert(sizeof...(Args) < 5, "Too many template arguments");
            
            // Static assert with trait expression
            static_assert(std::is_copy_constructible_v<T>, 
                         "T must be copy constructible");
        }
        
        // Method containing lambda
        void process(T& obj) {
            // Lambda expression with capture
            auto lambda = [&obj](auto&&... args) {
                return (obj.value + ... + args);
            };
            
            // Use lambda to avoid unused warning
            if constexpr (sizeof...(Args) > 0) {
                auto result = lambda(Args{}...);
                (void)result;
            }
        }
        
        // noexcept expression that might generate DEFERRED_NOEXCEPT
        auto get_noexcept() noexcept(noexcept(T())) {
            return T();
        }
    };

    // Explicit instantiations to force code generation
    template class Checker<int, double, char>;
    template class Checker<MyType, float, bool, short>;
    template class Checker<double, int, char, long, float>;
    
    // Function template with static_assert in different context
    template<typename... Ts>
    void variadic_check() {
        // Static assert using argument pack
        static_assert(sizeof...(Ts) != 0, "Need at least one type");
        
        // Lambda with generic lambda parameter
        auto pack_lambda = []<typename U>(U u) {
            return sizeof(U);
        };
        
        // Use lambda
        if constexpr (sizeof...(Ts) > 0) {
            auto size = pack_lambda(typename std::tuple_element<0, 
                              std::tuple<Ts...>>::type{});
            (void)size;
        }
    }
    
    // Instantiate the variadic template
    template void variadic_check<int, double>();
    template void variadic_check<char>();
}

// Another namespace for additional complexity
namespace Nested {
    template<typename T>
    class Base {
    public:
        virtual ~Base() = default;
        
        // Pure virtual with noexcept
        virtual void execute() noexcept = 0;
    };
    
    template<typename T>
    class Derived : public Base<T> {
    public:
        void execute() noexcept override {
            // Static assert in derived class
            static_assert(std::is_default_constructible_v<T>,
                         "T must be default constructible");
        }
        
        // Method with trailing return type and noexcept
        auto complex_method() noexcept(noexcept(T())) -> decltype(T()) {
            static_assert(sizeof(T) > 0, "Type must have size");
            return T();
        }
    };
    
    // Instantiate
    template class Derived<int>;
}

// Main function to ensure everything is referenced
int main() {
    // Instantiate and use Checker
    Checker<int, double, char>::verify();
    
    MyType mt{42};
    Checker<MyType, float, bool, short> checker;
    checker.process(mt);
    
    // Call variadic function
    variadic_check<int, double>();
    
    // Use nested template
    Nested::Derived<int> derived;
    derived.execute();
    
    return 0;
}

// Additional static_assert at namespace scope
static_assert(sizeof(long) >= 4, "long must be at least 4 bytes");

// Template with concept (C++20)
template<typename T>
concept HasValue = requires(T t) {
    { t.value } -> std::convertible_to<int>;
};

template<HasValue T>
void concept_check(T t) {
    static_assert(sizeof(T) > 0, "Type with value");
}

// Force instantiation
struct WithValue { int value; };
template void concept_check<WithValue>(WithValue);
```
