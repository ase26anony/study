Here's a C++ program designed to trigger the uncovered selective scheduler debug output:

```cpp
// Compile with: g++ -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug -msse4.2 -march=haswell coverage_program.cpp -o coverage_program
// Or for maximum scheduling: g++ -O3 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -funroll-loops -msse4.2 -march=haswell coverage_program.cpp -o coverage_program

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <emmintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>

// Mixed data type structure to create complex memory access patterns
struct MixedData {
    int a;
    double b;
    char c;
    float d;
    long e;
};

// Helper function with attribute to ensure optimization
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static double helper_func(int x, double y) {
    return sin(x * y) + cos(x - y);
}

// Function with complex loop-carried dependencies
__attribute__((optimize("O3")))
void nested_loops_dependency(MixedData* arr, int N, double* result) {
    double sum = 0.0;
    
    // Outer loop with varying trip count for inner loop
    for (int i = 1; i < N; ++i) {
        // Inner loop with data-dependent trip count
        for (int j = 0; j < i; ++j) {
            // Complex data-dependent operations
            arr[j].b = arr[i].a * 0.5 + helper_func(j, arr[j].b);
            
            // Conditional move/select operations
            double temp = (j % 2 == 0) ? arr[j].b * 2.0 : arr[j].b / 2.0;
            
            // Mix of operations
            arr[j].d = static_cast<float>(temp * sin(arr[j].b));
            
            // Inline assembly with clobbers to force scheduler work
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : : : "rax", "rbx", "rcx", "memory"
            );
            
            sum += arr[j].d;
        }
        
        // Function call within loop
        arr[i].b = pow(arr[i].b, 1.5) + helper_func(i, sum);
    }
    
    *result = sum;
}

// SIMD intensive function with vector intrinsics
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
void simd_processing(float* src, float* dst, int size) {
    #pragma GCC unroll 4
    for (int i = 0; i < size; i += 4) {
        // Load unaligned data
        __m128 vec = _mm_loadu_ps(&src[i]);
        
        // SIMD operations
        __m128 squared = _mm_mul_ps(vec, vec);
        __m128 sqrt_val = _mm_sqrt_ps(squared);
        __m128 result = _mm_add_ps(vec, sqrt_val);
        
        // Store result
        _mm_storeu_ps(&dst[i], result);
        
        // More complex SIMD with blending
        if (i > 0) {
            __m128 prev = _mm_loadu_ps(&dst[i-4]);
            __m128 blended = _mm_blendv_ps(result, prev, _mm_set1_ps(0.5f));
            _mm_storeu_ps(&dst[i], blended);
        }
    }
}

// Function with computed goto and jump table
__attribute__((optimize("O2")))
int computed_goto_pattern(int* data, int size) {
    static void* jumptable[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&case_6, &&case_7
    };
    
    int result = 0;
    
    for (int i = 0; i < size; ++i) {
        int idx = data[i] & 0x7;
        
        // Computed goto
        goto *jumptable[idx];
        
    case_0:
        result += data[i] * 2;
        continue;
    case_1:
        result += data[i] / 2;
        continue;
    case_2:
        result ^= data[i];
        continue;
    case_3:
        result |= data[i];
        continue;
    case_4:
        result &= data[i];
        continue;
    case_5:
        result = result << (data[i] & 0x3);
        continue;
    case_6:
        result = result >> (data[i] & 0x3);
        continue;
    case_7:
        result = ~result;
        continue;
    }
    
    return result;
}

// Switch statement with mixed dense/sparse cases
__attribute__((optimize("O3")))
int switch_pattern(int value) {
    int result = 0;
    
    // Switch with varying case density
    switch (value) {
        case 0:  // Dense region
        case 1:
        case 2:
        case 3:
            result = value * 10;
            break;
        case 4:
        case 5:
            result = value * 20;
            break;
        case 100:  // Sparse jump
            result = 1000;
            break;
        case 200:
            result = 2000;
            break;
        case 1000:  // Very sparse
            result = 10000;
            break;
        default:
            result = value * 2;
            break;
    }
    
    // Nested switch to create more complexity
    switch (result % 5) {
        case 0: result += 1; break;
        case 1: result += 2; break;
        case 2: result += 3; break;
        case 3: result += 4; break;
        case 4: result += 5; break;
    }
    
    return result;
}

// Matrix-like operations with non-contiguous access
void matrix_style_ops(MixedData* matrix, int rows, int cols) {
    #pragma GCC unroll 2
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int idx = i * cols + j;
            
            // Strided access pattern
            if (j % 2 == 0) {
                matrix[idx].b = matrix[idx].a * 0.25;
            } else {
                matrix[idx].b = matrix[idx].a * 0.75;
            }
            
            // Cross-element dependency
            if (i > 0 && j > 0) {
                int prev_idx = (i-1) * cols + (j-1);
                matrix[idx].d = matrix[prev_idx].d * 0.9f;
            }
            
            // Complex floating point ops
            matrix[idx].d = sinf(matrix[idx].d) * cosf(matrix[idx].b);
        }
    }
}

// Main benchmark function
int main() {
    const int N = 512;
    const int MATRIX_SIZE = 64;
    
    // Initialize with some pattern
    MixedData* data = new MixedData[N];
    float* src_array = new float[N];
    float* dst_array = new float[N];
    int* int_data = new int[N];
    MixedData* matrix = new MixedData[MATRIX_SIZE * MATRIX_SIZE];
    
    srand(time(nullptr));
    
    // Initialize arrays
    for (int i = 0; i < N; ++i) {
        data[i].a = rand() % 100;
        data[i].b = (rand() % 100) * 0.1;
        data[i].c = 'A' + (rand() % 26);
        data[i].d = (rand() % 100) * 0.01f;
        data[i].e = rand() % 1000;
        
        src_array[i] = (rand() % 100) * 0.01f;
        int_data[i] = rand() % 100;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i) {
        matrix[i].a = rand() % 100;
        matrix[i].d = (rand() % 100) * 0.01f;
    }
    
    double nested_result = 0.0;
    int goto_result = 0;
    int switch_sum = 0;
    
    // Execute all patterns to create scheduling pressure
    
    // 1. Nested loops with dependencies
    nested_loops_dependency(data, N, &nested_result);
    
    // 2. SIMD processing
    simd_processing(src_array, dst_array, N);
    
    // 3. Computed goto pattern
    goto_result = computed_goto_pattern(int_data, N);
    
    // 4. Multiple switch patterns
    for (int i = 0; i < N; ++i) {
        switch_sum += switch_pattern(int_data[i]);
    }
    
    // 5. Matrix operations
    matrix_style_ops(matrix, MATRIX_SIZE, MATRIX_SIZE);
    
    // Combine results to prevent dead code elimination
    double final_result = nested_result + goto_result + switch_sum;
    
    // Add SIMD results
    for (int i = 0; i < N; ++i) {
        final_result += dst_array[i];
    }
    
    // Add matrix results
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; ++i) {
        final_result += matrix[i].d;
    }
    
    std::cout << "Final checksum: " << final_result << std::endl;
    
    // Cleanup
    delete[] data;
    delete[] src_array;
    delete[] dst_array;
    delete[] int_data;
    delete[] matrix;
    
    return 0;
}
```

This program combines all the required elements:

1. **Complex Loop Dependencies**: The `nested_loops_dependency` function creates loops where the inner loop's trip count depends on the outer loop index, creating challenging scheduling problems.

2. **Mixed Data Types and Memory Access**: The `MixedData` structure and its usage throughout create non-contiguous, varied memory access patterns.

3. **Inline Assembly with Clobbers**: Multiple `asm volatile` statements with register clobbers force the scheduler to work around fixed constraints.

4. **Vector Intrinsics**: SSE intrinsics (`_mm_loadu_ps`, `_mm_mul_ps`, `_mm_blendv_ps`) generate specific RTL patterns.

5. **Function Calls in Hot Paths**: Calls to `helper_func`, `sin`, `cos`, and `pow` within loops create call instructions with varying register usage.

6. **Conditional Operations**: Ternary operators and conditional logic encourage conditional move instructions.

7. **Computed Goto**: The `computed_goto_pattern` function uses labels-as-values for indirect branching.

8. **Loop Unrolling Pragmas**: `#pragma GCC unroll` directives create larger basic blocks.

9. **Switch Statements**: Both dense and sparse switch cases test different jump table implementations.

10. **Optimization Attributes**: `__attribute__((optimize(...)))` ensures specific optimization levels for key functions.

The program creates sustained computational pressure across multiple domains, forcing the selective scheduler to analyze and potentially log many instructions through `dump_insn_rtx_1`. The final checksum prevents dead code elimination while allowing verification of correct execution.
