Here's a C++ program designed to trigger the specific uncovered lines in `targhooks.cc`:

```cpp
// Compile with: g++ -O3 -flto -fwhole-program -fvisibility=hidden -fvisibility-inlines-hidden -std=c++17

#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>
#include <cstdlib>

// ==================== SECTION 1: Lambda expressions with captures ====================
// These generate closure types and operator() with hidden visibility
#pragma GCC visibility push(hidden)
auto create_lambdas() {
    int capture1 = 42;
    double capture2 = 3.14;
    
    // Lambda with multiple captures - generates hidden closure type
    auto lambda1 = [capture1, capture2](int x) __attribute__((nothrow)) {
        return capture1 + capture2 + x;
    };
    
    // Generic lambda - generates templated operator()
    auto lambda2 = [capture1](auto&& arg) noexcept {
        return capture1 + arg;
    };
    
    return std::make_tuple(lambda1, lambda2);
}
#pragma GCC visibility pop

// ==================== SECTION 2: Extern volatile symbols ====================
// Force generation of TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL, TREE_THIS_VOLATILE
extern volatile int external_volatile_counter __attribute__((weak));
extern volatile double external_volatile_data __attribute__((weak, used));

// Static symbol with complex initialization that forces emission
static volatile int __attribute__((used, externally_visible, retain)) 
hidden_static_volatile = __builtin_constant_p(42) ? 42 : 0;

// ==================== SECTION 3: Custom container for range-based for ====================
#pragma GCC visibility push(hidden)
template<typename T>
class HiddenContainer {
    T data[10];
public:
    HiddenContainer() {
        for (int i = 0; i < 10; ++i) data[i] = T(i);
    }
    
    // Hidden begin/end functions
    T* begin() noexcept { return data; }
    T* end() noexcept { return data + 10; }
    
    const T* begin() const noexcept { return data; }
    const T* end() const noexcept { return data + 10; }
};
#pragma GCC visibility pop

// ==================== SECTION 4: Complex template metaprogramming ====================
// Recursive template for compile-time computation
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N - 1>::value;
    static void force_odr_use() __attribute__((used, visibility("hidden"))) {
        // Force ODR-use with volatile
        volatile int dummy = value;
        (void)dummy;
    }
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
    static void force_odr_use() __attribute__((used, visibility("hidden"))) {
        volatile int dummy = value;
        (void)dummy;
    }
};

// Variable template with specializations
template<typename T>
constexpr T constant = T(3.14159);

template<>
constexpr double constant<double> = 3.141592653589793;

template<>
constexpr int constant<int> = 42;

// Type-generating constexpr function
template<typename T>
constexpr auto generate_type() {
    if constexpr (std::is_integral_v<T>) {
        return std::integral_constant<int, 1>{};
    } else {
        return std::integral_constant<int, 0>{};
    }
}

// ==================== SECTION 5: Structured bindings ====================
#pragma GCC visibility push(hidden)
auto get_hidden_tuple() {
    // Force generation of hidden decomposition declarations
    return std::make_tuple(
        constant<int>,
        constant<double>,
        Factorial<5>::value
    );
}
#pragma GCC visibility pop

// ==================== SECTION 6: noexcept and typeid operators ====================
class PolymorphicBase {
public:
    virtual ~PolymorphicBase() = default;
    virtual void foo() noexcept {}
};

class DerivedHidden : public PolymorphicBase {
public:
    void foo() noexcept override {}
};

// ==================== SECTION 7: Main function integrating all patterns ====================
int main() {
    // 1. Use lambdas with captures
    auto [lambda1, lambda2] = create_lambdas();
    int lambda_result = lambda1(10) + lambda2(5.5);
    
    // 2. Reference extern volatile symbols in inline assembly
    // This prevents optimization and forces TREE_USED
    asm volatile (
        "/* Reference external volatile symbols */"
        : 
        : "r" (external_volatile_counter), "r" (external_volatile_data)
        : "memory"
    );
    
    // Use the static volatile with inline assembly
    asm volatile (
        "addl $1, %0"
        : "+m" (hidden_static_volatile)
        :
        : "cc"
    );
    
    // 3. Range-based for over custom container
    HiddenContainer<int> hidden_container;
    int sum = 0;
    for (auto x : hidden_container) {
        sum += x;
    }
    
    // 4. Use structured bindings
    auto [a, b, c] = get_hidden_tuple();
    
    // 5. Force template instantiation and ODR-use
    Factorial<7>::force_odr_use();
    Factorial<8>::force_odr_use();
    
    // Use variable templates
    volatile auto vt1 = constant<int>;
    volatile auto vt2 = constant<double>;
    (void)vt1; (void)vt2;
    
    // 6. Use noexcept and typeid
    DerivedHidden dh;
    PolymorphicBase* pb = &dh;
    pb->foo();
    
    // Force typeid usage which may generate internal symbols
    if (typeid(*pb) == typeid(DerivedHidden)) {
        // Do nothing, just force the comparison
    }
    
    // 7. Complex constexpr evaluation
    constexpr auto gen1 = generate_type<int>();
    constexpr auto gen2 = generate_type<double>();
    static_assert(gen1.value == 1, "Compile-time check");
    static_assert(gen2.value == 0, "Compile-time check");
    
    // 8. Additional volatile access pattern
    volatile int* volatile_ptr = &hidden_static_volatile;
    asm volatile ("" : "+r" (volatile_ptr) : : "memory");
    
    // 9. Use all results to prevent optimization
    std::cout << "Results: " 
              << lambda_result << " "
              << sum << " "
              << a << " "
              << b << " "
              << c << " "
              << hidden_static_volatile << std::endl;
    
    // 10. Force instantiation of multiple template depths
    // This creates many internal artificial declarations
    struct ForceInstantiation {
        template<int I>
        struct Inner {
            static constexpr int value = I * 2;
            static void use() __attribute__((used, visibility("hidden"))) {
                volatile int v = value;
                (void)v;
            }
        };
    };
    
    ForceInstantiation::Inner<1>::use();
    ForceInstantiation::Inner<2>::use();
    ForceInstantiation::Inner<3>::use();
    ForceInstantiation::Inner<4>::use();
    ForceInstantiation::Inner<5>::use();
    
    return 0;
}

// ==================== SECTION 8: Additional translation unit patterns ====================
// These would typically be in separate headers/files to force ODR-use across TUs

// Hidden visibility inline function
#pragma GCC visibility push(hidden)
inline __attribute__((always_inline, nothrow)) 
int hidden_inline_func(int x) {
    return x * constant<int>;
}
#pragma GCC visibility pop

// Template instantiation in hidden section
#pragma GCC visibility push(hidden)
template<typename T>
class HiddenTemplate {
    T value;
public:
    HiddenTemplate(T v) noexcept : value(v) {}
    T get() const noexcept { return value; }
    
    // Force generation of multiple methods
    T add(T other) const noexcept { return value + other; }
    T mul(T other) const noexcept { return value * other; }
};

// Instantiate with different types
template class HiddenTemplate<int>;
template class HiddenTemplate<double>;
#pragma GCC visibility pop

// Weak symbol definition (could be overridden externally)
volatile int external_volatile_counter = 0;
volatile double external_volatile_data = 1.0;
```

This program combines multiple techniques to trigger the specific GCC internal hooks:

1. **Lambda expressions** with captures generate closure types with `operator()` that should be marked with hidden visibility and nothrow attributes.

2. **Extern volatile symbols** with weak linkage force `TREE_STATIC`, `TREE_PUBLIC`, `DECL_EXTERNAL`, and `TREE_THIS_VOLATILE` flags.

3. **Structured bindings** on tuples create hidden decomposition declarations.

4. **Range-based for loops** over custom containers require hidden `begin`/`end` calls.

5. **Complex template metaprogramming** with recursive specializations forces generation of numerous internal symbols.

6. **`noexcept` functions and `typeid` operators** may generate internal lookup symbols.

7. **Inline assembly with volatile** prevents optimization and ensures symbols are marked as `TREE_USED`.

8. **Hidden visibility sections** with `#pragma GCC visibility push(hidden)` force `DECL_VISIBILITY_SPECIFIED` and `DECL_VISIBILITY(t) = VISIBILITY_HIDDEN`.

9. **`__attribute__((used, externally_visible, retain))`** ensures static symbols are emitted and marked as used.

10. **Multiple template instantiations** at different depths create artificial declarations for metaprogramming.

The compilation options recommended will maximize the generation of internal symbols and trigger the middle-end hooks that set these specific tree flags.
