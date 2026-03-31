Here's a C++ program designed to trigger the uncovered lines in `targhooks.cc` by creating artificial declarations with hidden visibility:

```cpp
// Compile with: g++ -O3 -fopenmp -fsanitize=address -march=native -ftree-vectorize -fno-omit-frame-pointer

#include <cstdlib>
#include <cmath>

// 1. Artificial built-in declarations with visibility attributes
__attribute__((visibility("hidden"), always_inline)) 
static inline int hidden_builtin_abs(int x) {
    // Complex expression with built-in that may create artificial nodes
    return __builtin_abs(x) * __builtin_expect((x != 0), 1);
}

__attribute__((visibility("internal"), weak))
int weak_hidden_func(int* ptr) {
    // Use alignment built-in that may generate artificial declarations
    int* aligned_ptr = (int*)__builtin_assume_aligned(ptr, 16);
    return aligned_ptr ? *aligned_ptr : 0;
}

// 2. Function with constructor attribute - may generate initialization code
__attribute__((constructor, visibility("hidden")))
static void hidden_constructor() {
    volatile int x = 42;
    // Use trap/unreachable in conditional that gets optimized
    if (__builtin_expect(x == 0, 0)) {
        __builtin_trap();
    }
}

// 3. Hot function with complex control flow
__attribute__((hot, visibility("default")))
static int hot_function_with_builtins(int n) {
    int sum = 0;
    // Loop with unpredictable termination
    for (int i = 0; __builtin_unpredictable(i < n); ++i) {
        sum += hidden_builtin_abs(i - n/2);
        
        // Use unreachable in optimized-away branch
        if (__builtin_expect(sum > 1000000, 0)) {
            __builtin_unreachable();
        }
    }
    return sum;
}

// 4. Vector types and operations
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

__attribute__((visibility("hidden")))
static v4si vector_operation(v4si a, v4si b) {
    // Use vector built-ins
    v4si result = a + b;
    // Shuffle may require artificial declarations
    result = __builtin_shuffle(result, result, (v4si){3, 2, 1, 0});
    return result;
}

// 5. OpenMP helper function - often generates artificial declarations
__attribute__((visibility("hidden")))
static void omp_helper(int* data, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        data[i] = hidden_builtin_abs(data[i]);
    }
}

// 6. Destructor with sanitizer interaction
__attribute__((destructor, visibility("hidden")))
static void hidden_destructor() {
    // Array access that sanitizers might instrument
    volatile int small_array[4] = {0};
    // This may trigger ASan instrumentation
    int val = small_array[__builtin_abs(-2) % 4];
    (void)val; // Suppress unused warning
}

// Main function that exercises all patterns
int main() {
    // Initialize some data
    const int N = 1000;
    int* data = new int[N];
    
    // Fill with alternating positive/negative values
    for (int i = 0; i < N; ++i) {
        data[i] = (i % 2 == 0) ? i : -i;
    }
    
    // 1. Use function with hidden visibility and built-ins
    for (int i = 0; i < 10; ++i) {
        data[i] = hidden_builtin_abs(data[i]);
    }
    
    // 2. Call weak function (may not exist, but declaration is processed)
    weak_hidden_func(data);
    
    // 3. Hot function with complex control flow
    int result = hot_function_with_builtins(100);
    
    // 4. Vector operations
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = vector_operation(vec_a, vec_b);
    
    // Store vector result to prevent optimization
    for (int i = 0; i < 4; ++i) {
        data[i] += vec_result[i];
    }
    
    // 5. OpenMP parallel region
    omp_helper(data, N);
    
    // 6. Additional built-in usage
    volatile int* ptr = data;
    if (__builtin_expect(ptr != nullptr, 1)) {
        // Use alignment hint
        int* aligned = (int*)__builtin_assume_aligned(ptr, sizeof(void*));
        result += aligned[0];
    }
    
    // 7. More control flow with built-ins
    int limit = 50;
    while (__builtin_unpredictable(limit-- > 0)) {
        result += __builtin_popcount(result);
        
        // Conditional that may be optimized away
        if (__builtin_expect(result > 1000000, 0)) {
            __builtin_unreachable();
        }
    }
    
    // Use result to prevent optimization
    if (result > 1000) {
        // Potential sanitizer check
        data[N-1] = result % 256;
    }
    
    delete[] data;
    
    return result % 256;
}
```

**Key elements that target the uncovered lines:**

1. **Multiple artificial declarations**: Functions with `__attribute__((constructor))`, `__attribute__((destructor))`, `always_inline`, and `weak` attributes often cause GCC to generate artificial tree nodes.

2. **Visibility attributes**: Explicit use of `visibility("hidden")`, `visibility("internal")`, and `visibility("default")` combined with other attributes.

3. **Built-in functions in complex contexts**: `__builtin_abs`, `__builtin_expect`, `__builtin_unpredictable`, `__builtin_assume_aligned`, `__builtin_trap`, `__builtin_unreachable`, `__builtin_shuffle`, and `__builtin_popcount` used in ways that may require compiler-generated helper code.

4. **OpenMP pragma**: The `#pragma omp parallel for` often causes GCC to generate artificial declarations for parallel regions.

5. **Vector types**: Use of GCC vector extensions which may require artificial type declarations.

6. **Sanitizer interaction**: Array access patterns that could trigger AddressSanitizer instrumentation when compiled with `-fsanitize=address`.

7. **Complex control flow**: Loops with `__builtin_unpredictable` and conditionals with `__builtin_expect` that may trigger optimization hooks.

The combination of these patterns increases the likelihood that GCC's internal machinery will create artificial tree nodes and set the flags specified in the uncovered lines, particularly when compiled with the recommended optimization and sanitizer flags.
