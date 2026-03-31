Here's a C++ program designed to trigger the uncovered lines in `targhooks.cc` by creating scenarios where the compiler generates artificial declarations with specific visibility and linkage properties:

```cpp
// Compile with: g++ -O3 -fopenmp -fsanitize=address -march=native -ftree-vectorize -fno-omit-frame-pointer

#include <cstdlib>
#include <cmath>
#include <cstring>

// Vector type for target-specific built-ins
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

// Function with hidden visibility using built-ins
__attribute__((visibility("hidden"), noinline))
static int hidden_builtin_func(int x, int y) {
    // Complex expression with multiple built-ins
    int a = __builtin_abs(x);
    int b = __builtin_expect(y > 0, 1);
    int c = __builtin_clz(a | 1);
    
    // Unpredictable control flow
    if (__builtin_unpredictable((a ^ b) & 1)) {
        return __builtin_popcount(a) + c;
    } else {
        return __builtin_ctz(b | 1) * a;
    }
}

// Weak function with internal visibility using vector built-ins
__attribute__((weak, visibility("internal")))
void vector_operations(v4si* data, int n) {
    v4si pattern = {0, 1, 2, 3};
    
    for (int i = 0; i < n; i++) {
        // Vector shuffle operation
        data[i] = __builtin_shuffle(data[i], pattern);
        
        // Assume aligned pointer for optimization
        v4si* aligned_ptr = (v4si*)__builtin_assume_aligned(data + i, 16);
        
        // Vector arithmetic
        *aligned_ptr = *aligned_ptr + pattern;
    }
}

// Hot function with default visibility using complex built-ins
__attribute__((hot, visibility("default"), noinline))
int hot_function_with_builtins(int* arr, int size) {
    int sum = 0;
    
    // Loop with builtin_expect for optimization
    for (int i = 0; i < size; i++) {
        if (__builtin_expect(arr[i] != 0, 1)) {
            // Use assume aligned for pointer
            int* ptr = (int*)__builtin_assume_aligned(arr + i, sizeof(int));
            
            // Complex builtin usage
            sum += __builtin_abs(*ptr) * __builtin_clz(*ptr | 1);
            
            // Builtin for unreachable code elimination
            if (*ptr < 0 && __builtin_unpredictable(false)) {
                __builtin_unreachable();
            }
        }
    }
    
    // Trap in unreachable path
    if (sum < 0) {
        __builtin_trap();
    }
    
    return sum;
}

// Constructor function that uses built-ins
__attribute__((constructor, visibility("hidden")))
void init_constructor() {
    // Use builtins in constructor
    volatile int x = __builtin_bswap32(0x12345678);
    (void)x;
}

// Destructor function
__attribute__((destructor, visibility("hidden")))
void cleanup_destructor() {
    // Use sync builtin
    __builtin_ia32_sfence();
}

// OpenMP helper function with artificial declaration potential
__attribute__((visibility("hidden")))
static void omp_helper(int* data, int start, int end) {
    for (int i = start; i < end; i++) {
        data[i] = hidden_builtin_func(data[i], i);
    }
}

// Main function with various constructs
int main() {
    const int N = 1024;
    int* data = (int*)aligned_alloc(16, N * sizeof(int));
    
    if (!data) return 1;
    
    // Initialize data
    for (int i = 0; i < N; i++) {
        data[i] = i - N/2;
    }
    
    // 1. Call function with hidden visibility and built-ins
    int result1 = hidden_builtin_func(data[0], data[1]);
    
    // 2. Use vector operations
    v4si vec_data[N/4];
    for (int i = 0; i < N/4; i++) {
        vec_data[i] = {i, i+1, i+2, i+3};
    }
    vector_operations(vec_data, N/4);
    
    // 3. Call hot function with built-ins
    int result2 = hot_function_with_builtins(data, N);
    
    // 4. OpenMP parallel region (triggers artificial declarations)
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        // Use builtin within OpenMP region
        data[i] = __builtin_abs(data[i]) + __builtin_popcount(i);
        
        // AddressSanitizer check (if enabled)
        if (i >= 0 && i < N) {
            data[i] += result1;
        }
    }
    
    // 5. Additional vector operations with shuffle
    v4sf float_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf converted = __builtin_convertvector(vec_data[0], v4sf);
    converted = converted + float_vec;
    
    // 6. Complex control flow with builtins
    volatile int control = 0;
    for (int i = 0; i < 100; i++) {
        if (__builtin_unpredictable((i & 3) == 0)) {
            control += __builtin_clz(i | 1);
        } else {
            control -= __builtin_ctz(i | 1);
        }
        
        // Potential unreachable path
        if (__builtin_expect(control > 1000, 0)) {
            __builtin_unreachable();
        }
    }
    
    // 7. Call OpenMP helper
    omp_helper(data, 0, N/2);
    
    // Final computation using results
    int final_result = result1 + result2 + control + data[N-1];
    
    free(data);
    
    return final_result == 0 ? 0 : 1;
}
```

This program combines multiple techniques to trigger the target code in `targhooks.cc`:

1. **Artificial/Built-in Declarations**: Uses numerous GCC built-ins (`__builtin_abs`, `__builtin_expect`, `__builtin_clz`, `__builtin_shuffle`, etc.) in complex expressions that force the compiler to create internal artificial tree nodes.

2. **Visibility Attributes**: Functions with `visibility("hidden")`, `visibility("internal")`, and `visibility("default")` attributes combined with artificial declaration contexts.

3. **OpenMP Pragmas**: `#pragma omp parallel for` triggers compiler-generated helper functions and data structures.

4. **Constructor/Destructor Attributes**: Functions marked with `__attribute__((constructor))` and `__attribute__((destructor))` involve compiler-generated initialization code.

5. **Vector Extensions**: Uses GCC vector types and vector-specific built-ins (`__builtin_shuffle`, `__builtin_convertvector`).

6. **Complex Control Flow**: Uses `__builtin_unpredictable`, `__builtin_unreachable()`, and `__builtin_trap()` in conditional blocks.

7. **Sanitizer Compatibility**: Includes bounds checking that works with `-fsanitize=address`.

8. **Hot/Cold Attributes**: Uses `__attribute__((hot))` to influence optimization decisions.

The program should be compiled with the recommended flags to maximize the chances of triggering the uncovered lines. The combination of optimization levels, OpenMP, sanitizers, and vectorization creates multiple pathways for the compiler to generate artificial declarations with the specific flags set in the target code block.
