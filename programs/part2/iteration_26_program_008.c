Here's a C++ program designed to trigger the uncovered selective scheduler debug output:

```cpp
// Compile with: g++ -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug -msse4.2 -march=haswell coverage_program.cpp -o coverage_program
// Or for maximum scheduling: g++ -O3 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -funroll-loops -msse4.2 -march=haswell coverage_program.cpp -o coverage_program

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <emmintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>

// Mixed data structure to create varied memory access patterns
struct MixedData {
    int a;
    double b;
    char c;
    float d;
    long e;
};

// Function with optimization attribute to ensure selective scheduling
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static double helper_func(int x, double y, float z) {
    // Complex arithmetic with branches
    if (x > 0) {
        return y * z + std::sin(y);
    } else {
        return y / (z + 1.0) + std::cos(y);
    }
}

// SIMD processing function
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
void process_simd(float* src, float* dst, int n) {
    for (int i = 0; i < n; i += 4) {
        // Load unaligned data
        __m128 vec = _mm_loadu_ps(src + i);
        
        // Perform SIMD operations
        __m128 squared = _mm_mul_ps(vec, vec);
        __m128 sqrt_val = _mm_sqrt_ps(squared);
        
        // Conditional operation using blend
        __m128 threshold = _mm_set1_ps(0.5f);
        __m128 mask = _mm_cmpgt_ps(vec, threshold);
        __m128 result = _mm_blendv_ps(sqrt_val, vec, mask);
        
        // Store result
        _mm_storeu_ps(dst + i, result);
        
        // Inline assembly with clobber to force scheduler work
        asm volatile ("nop" : : : "rax", "rbx", "memory");
    }
}

// Complex loop structure with varying trip counts
__attribute__((optimize("O2")))
long nested_loops_computation(int N) {
    long total = 0;
    
    // Outer loop with varying inner loop trip count
    for (int i = 1; i < N; ++i) {
        // Inner loop whose iterations depend on outer index
        for (int j = 0; j < i; ++j) {
            // Data-dependent operations with mixed types
            double temp = std::pow(i, 1.0 / (j + 1));
            float ftemp = static_cast<float>(temp);
            
            // Call helper function with varying arguments
            total += static_cast<long>(helper_func(j, temp, ftemp) * 1000);
            
            // Ternary operator for conditional move
            int cond_val = (i * j) % 7 == 0 ? i : j;
            total += cond_val;
        }
        
        // Additional computation with pointer arithmetic
        MixedData* data = new MixedData[i];
        for (int k = 0; k < i; ++k) {
            data[k].a = i + k;
            data[k].b = data[k].a * 0.5;
            data[k].c = static_cast<char>((i * k) % 256);
            data[k].d = static_cast<float>(std::sin(data[k].b));
            data[k].e = total + k;
            
            // Non-contiguous access pattern
            if (k % 2 == 0) {
                total += data[k].a + static_cast<long>(data[k].b);
            } else {
                total -= data[k].e;
            }
        }
        delete[] data;
    }
    
    return total;
}

// Function with computed goto for complex control flow
__attribute__((noinline))
int computed_goto_demo(int x) {
    static void* jump_table[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5, &&label6, &&label7
    };
    
    int result = 0;
    int idx = x % 8;
    
    // Indirect branch
    goto *jump_table[idx];
    
label0:
    result = x * 2;
    goto end;
label1:
    result = x + 100;
    goto end;
label2:
    result = x - 50;
    goto end;
label3:
    result = x / 3;
    goto end;
label4:
    result = x * x;
    goto end;
label5:
    result = std::abs(x);
    goto end;
label6:
    result = x << 2;
    goto end;
label7:
    result = x | 0xFF;
    goto end;
    
end:
    return result;
}

// Matrix multiplication with loop unrolling hint
#pragma GCC unroll 4
void matrix_multiply(const float A[4][4], const float B[4][4], float C[4][4]) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

// Switch statement with mixed dense/sparse cases
int complex_switch(int val) {
    int result = 0;
    
    switch (val) {
        // Dense range
        case 0:  result = val + 1; break;
        case 1:  result = val * 2; break;
        case 2:  result = val << 3; break;
        case 3:  result = val / 2; break;
        case 4:  result = val | 0x0F; break;
        
        // Sparse range
        case 10: result = val - 5; break;
        case 20: result = val + 15; break;
        case 50: result = val * 3; break;
        case 100: result = val >> 2; break;
        case 200: result = val & 0xFF; break;
        
        // Default with computation
        default: 
            result = std::abs(val);
            // More computation in default case
            for (int i = 0; i < (val % 10); ++i) {
                result += i * i;
            }
            break;
    }
    
    return result;
}

// Main computational kernel
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
long compute_kernel(int iterations) {
    long checksum = 0;
    
    // Initialize arrays for SIMD processing
    const int ARRAY_SIZE = 1024;
    float* src_array = new float[ARRAY_SIZE];
    float* dst_array = new float[ARRAY_SIZE];
    
    // Initialize with pattern
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        src_array[i] = std::sin(i * 0.1f) + std::cos(i * 0.05f);
    }
    
    // Perform multiple types of computations
    for (int iter = 0; iter < iterations; ++iter) {
        // 1. Nested loops with varying trip counts
        checksum ^= nested_loops_computation(50 + (iter % 10));
        
        // 2. SIMD processing
        process_simd(src_array, dst_array, ARRAY_SIZE);
        
        // Process SIMD results
        for (int i = 0; i < ARRAY_SIZE; i += 8) {
            __m128 v1 = _mm_loadu_ps(dst_array + i);
            __m128 v2 = _mm_loadu_ps(dst_array + i + 4);
            __m128 sum = _mm_add_ps(v1, v2);
            float temp[4];
            _mm_storeu_ps(temp, sum);
            checksum += static_cast<long>(temp[0] + temp[1] + temp[2] + temp[3]);
        }
        
        // 3. Computed goto pattern
        checksum += computed_goto_demo(iter);
        
        // 4. Matrix operations
        float A[4][4], B[4][4], C[4][4];
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                A[i][j] = (i + j + iter) * 0.1f;
                B[i][j] = (i * j - iter) * 0.2f;
            }
        }
        matrix_multiply(A, B, C);
        
        // Use matrix result
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                checksum += static_cast<long>(C[i][j] * 1000);
            }
        }
        
        // 5. Complex switch statements
        checksum += complex_switch(iter % 250);
        
        // 6. Mixed data structure processing
        MixedData mixed_arr[100];
        for (int i = 0; i < 100; i += 2) {
            mixed_arr[i].a = iter + i;
            mixed_arr[i].b = mixed_arr[i].a * 0.5;
            mixed_arr[i + 1].a = iter - i;
            mixed_arr[i + 1].b = mixed_arr[i + 1].a * 1.5;
            
            // Cross-structure computation
            mixed_arr[i].d = static_cast<float>(mixed_arr[i + 1].b);
            mixed_arr[i + 1].d = static_cast<float>(mixed_arr[i].b);
            
            checksum += static_cast<long>(mixed_arr[i].d * 100) + 
                       static_cast<long>(mixed_arr[i + 1].d * 100);
        }
        
        // Another inline assembly with different clobbers
        asm volatile ("nop; nop" : : : "rcx", "rdx", "memory");
    }
    
    delete[] src_array;
    delete[] dst_array;
    
    return checksum;
}

int main() {
    std::cout << "Starting selective scheduler coverage test..." << std::endl;
    
    // Set random seed for varied execution
    srand(42);
    
    // Perform computation with sufficient iterations
    long final_result = compute_kernel(100);
    
    // Additional computation to ensure all code paths are used
    final_result += complex_switch(rand() % 300);
    final_result += computed_goto_demo(rand() % 100);
    
    std::cout << "Computation checksum: " << final_result << std::endl;
    std::cout << "Test completed. Check compiler debug output for sel-sched-dump coverage." << std::endl;
    
    return 0;
}
```

This program combines all the required elements:

1. **Selective Scheduling Activation**: Uses `-fsel-sched-pipelining` and `-fsel-sched-pipelining-outer-loops` flags with complex loop structures that benefit from pipelining.

2. **Loop-Carried Dependencies**: `nested_loops_computation` has inner loops whose trip counts depend on outer loop indices.

3. **Mixed Data Types**: `MixedData` struct with different types and non-contiguous access patterns.

4. **Inline Assembly**: Multiple `asm volatile` statements with register clobbers.

5. **SIMD Intrinsics**: SSE/AVX operations using compiler intrinsics.

6. **Function Calls**: Calls to `helper_func`, `std::sin`, `std::cos`, `std::pow` within loops.

7. **Conditional Moves**: Ternary operators and `_mm_blendv_ps` for conditional selection.

8. **Computed Goto**: Indirect branches via jump tables.

9. **Loop Unrolling**: `#pragma GCC unroll` hint for matrix multiplication.

10. **Complex Switch**: Mixed dense and sparse case values.

11. **Optimization Attributes**: `__attribute__((optimize(...)))` on key functions.

The program creates sustained computational pressure across multiple basic blocks and loops, forcing the selective scheduler to analyze and potentially log many instructions via `dump_insn_rtx_1`. The final checksum ensures the compiler cannot dead-code eliminate the computations.
