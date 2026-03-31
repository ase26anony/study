Here's a C++ program designed to trigger the uncovered selective scheduler debug output:

```cpp
// Compile with: g++ -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug -msse4.2 -o sel_sched_test sel_sched_test.cpp -lm
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <emmintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>

// Mixed data structure for non-contiguous access
struct MixedData {
    int a;
    double b;
    char c;
    float d;
};

// Helper function with attribute to ensure optimization
__attribute__((optimize("O2"))) 
static inline double helper_func(int x, double y) {
    return sin(x * y) + cos(x - y);
}

// Function with complex loop patterns
__attribute__((optimize("O2", "fsel-sched-pipelining")))
void nested_loops_with_deps(MixedData* arr, int N) {
    // Loop-carried dependencies with varying trip counts
    for (int i = 1; i < N; ++i) {
        // Inner loop depends on outer index
        for (int j = 0; j < i; ++j) {
            // Data-dependent operations with mixed types
            arr[i].b = arr[j].a * 0.5 + helper_func(i, j);
            arr[i].d = static_cast<float>(arr[j].a % 7);
            
            // Conditional move operations
            arr[i].a = (i > j * 2) ? arr[j].a + i : arr[j].a - j;
            
            // Inline assembly with clobbers to force scheduling constraints
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : : : "rax", "rbx", "rcx", "memory"
            );
        }
    }
}

// SIMD intensive function
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
void simd_processing(int* src, int* dst, int len) {
    #pragma GCC unroll 4
    for (int i = 0; i < len - 3; i += 4) {
        // SSE intrinsics - varied operations
        __m128i v1 = _mm_loadu_si128((__m128i*)(src + i));
        __m128i v2 = _mm_add_epi32(v1, v1);
        __m128i v3 = _mm_mullo_epi32(v2, _mm_set1_epi32(3));
        __m128i v4 = _mm_slli_epi32(v3, 2);
        
        // Conditional SIMD operation simulation
        __m128i mask = _mm_cmpgt_epi32(v1, _mm_set1_epi32(100));
        __m128i result = _mm_blendv_epi8(v4, v2, mask);
        
        _mm_storeu_si128((__m128i*)(dst + i), result);
        
        // More inline assembly
        asm volatile ("mfence" ::: "memory");
    }
}

// Function with computed goto and switch
__attribute__((optimize("O2")))
int computed_goto_test(int x) {
    static const void* jtable[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    
    int idx = x % 5;
    int result = 0;
    
    // Computed goto
    goto *jtable[idx];
    
case0:
    result = x * 2;
    goto end;
case1:
    result = x + x;
    goto end;
case2:
    result = x | 0xFF;
    goto end;
case3:
    result = x ^ 0xAAAA;
    goto end;
default_case:
    result = ~x;
    goto end;
    
end:
    return result;
}

// Complex switch with sparse cases
__attribute__((optimize("O2")))
int sparse_switch(int val) {
    switch(val) {
        case 1: return val * 10;
        case 2: return val + 100;
        case 10: return val / 2;
        case 100: return val - 50;
        case 1000: return val | 0xFFFF;
        case 10000: return val ^ 0xAAAA;
        default: return val * 3;
    }
}

// Matrix-like operations with pointer arithmetic
__attribute__((optimize("O3", "funroll-loops")))
void matrix_style_ops(double** matrix, int size) {
    for (int i = 0; i < size; ++i) {
        #pragma GCC unroll 2
        for (int j = 0; j < size; ++j) {
            // Complex addressing with mixed operations
            double* row = matrix[i];
            double val = row[j];
            
            // Library function calls in loop
            val = pow(fabs(val), 1.5);
            val = sin(val) * cos(val);
            
            // Conditional operation
            row[j] = (val > 0) ? val : -val;
            
            // Another inline assembly
            asm volatile ("pause" ::: "memory");
        }
    }
}

// Main benchmark function
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
int main() {
    const int N = 100;
    const int ARR_SIZE = 1000;
    
    // Initialize with pattern
    MixedData* mixed_arr = new MixedData[ARR_SIZE];
    for (int i = 0; i < ARR_SIZE; ++i) {
        mixed_arr[i].a = i * 3;
        mixed_arr[i].b = i * 0.1;
        mixed_arr[i].c = i % 128;
        mixed_arr[i].d = i * 0.01f;
    }
    
    int* src_arr = new int[ARR_SIZE];
    int* dst_arr = new int[ARR_SIZE];
    for (int i = 0; i < ARR_SIZE; ++i) {
        src_arr[i] = i * 7 % 255;
    }
    
    // Create matrix
    double** matrix = new double*[N];
    for (int i = 0; i < N; ++i) {
        matrix[i] = new double[N];
        for (int j = 0; j < N; ++j) {
            matrix[i][j] = (i + j) * 0.1;
        }
    }
    
    int checksum = 0;
    
    // Execute all patterns to trigger scheduler
    nested_loops_with_deps(mixed_arr, 50);
    
    simd_processing(src_arr, dst_arr, ARR_SIZE);
    
    for (int i = 0; i < 100; ++i) {
        checksum ^= computed_goto_test(i);
        checksum += sparse_switch(i % 10001);
    }
    
    matrix_style_ops(matrix, N);
    
    // Additional complex loop with mixed operations
    for (int i = 0; i < ARR_SIZE; i += 2) {
        mixed_arr[i].b = mixed_arr[i + 1].a * 0.5;
        
        // More conditional moves
        double temp = (i % 3 == 0) ? mixed_arr[i].b : mixed_arr[i].d;
        mixed_arr[i].a = static_cast<int>(temp * 100);
        
        // Function call with varying arguments
        mixed_arr[i].d = static_cast<float>(helper_func(i, mixed_arr[i].b));
    }
    
    // Final checksum to prevent dead code elimination
    for (int i = 0; i < ARR_SIZE; ++i) {
        checksum += mixed_arr[i].a;
        checksum ^= dst_arr[i];
    }
    
    std::cout << "Checksum: " << checksum << std::endl;
    
    // Cleanup
    delete[] mixed_arr;
    delete[] src_arr;
    delete[] dst_arr;
    for (int i = 0; i < N; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;
    
    return 0;
}
```

This program combines all the required elements:

1. **Loop-carried dependencies**: `nested_loops_with_deps` has inner loops whose trip counts depend on outer loop indices.

2. **Mixed data types and non-contiguous access**: `MixedData` struct with different types accessed in non-linear patterns.

3. **Inline assembly with clobbers**: Multiple `asm volatile` statements with register and memory clobbers.

4. **Vector intrinsics**: SSE4.2 operations in `simd_processing`.

5. **Function calls in loops**: Calls to `helper_func`, `sin`, `cos`, `pow`, `fabs`.

6. **Conditional moves**: Ternary operators (`? :`) used extensively.

7. **Computed goto**: Jump table implementation in `computed_goto_test`.

8. **Loop unrolling pragmas**: `#pragma GCC unroll` directives.

9. **Sparse switch**: `sparse_switch` with widely spaced case values.

10. **Optimization attributes**: `__attribute__((optimize(...)))` on key functions.

11. **Matrix operations**: Double-nested loops with complex addressing.

The program creates varied computational patterns that should force the selective scheduler to make complex decisions and generate debug output, potentially triggering the uncovered `dump_insn_rtx_1` calls. The final checksum ensures no dead code elimination.
