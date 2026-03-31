Here's a C++ program designed to trigger the uncovered selective scheduler debug output:

```cpp
// Compile with: g++ -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug -msse4.2 -march=haswell -o sel_sched_test sel_sched_test.cpp -lm
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <emmintrin.h>
#include <smmintrin.h>
#include <xmmintrin.h>

// Structure with mixed data types to create complex memory access patterns
struct MixedData {
    int a;
    double b;
    char c;
    float d;
    long e;
};

// Function with optimization attribute to ensure selective scheduling
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static double helper_func(int x, double y) {
    // Complex operations to generate varied RTL
    double result = sin(y) * cos(x * 0.1);
    result += (x % 7) * 0.25;
    
    // Inline assembly with clobbers to force scheduler constraints
    asm volatile (
        "nop\n\t"
        "nop\n\t"
        : : : "rax", "rcx", "memory"
    );
    
    return result;
}

// Function with SIMD operations
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
void process_simd(int* src, int* dst, int size) {
    #pragma GCC unroll 4
    for (int i = 0; i < size; i += 4) {
        // Load unaligned data
        __m128i data = _mm_loadu_si128((__m128i*)(src + i));
        
        // Multiple SIMD operations
        __m128i doubled = _mm_add_epi32(data, data);
        __m128i shifted = _mm_slli_epi32(doubled, 1);
        __m128i result = _mm_add_epi32(doubled, shifted);
        
        // Conditional operation using blend
        __m128i mask = _mm_cmpgt_epi32(data, _mm_set1_epi32(100));
        result = _mm_blendv_epi8(result, data, mask);
        
        _mm_storeu_si128((__m128i*)(dst + i), result);
    }
}

// Function with nested loops and loop-carried dependencies
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double nested_loop_computation(int N) {
    double sum = 0.0;
    
    // Outer loop with varying trip count for inner loop
    for (int i = 1; i < N; ++i) {
        // Inner loop with data-dependent trip count
        for (int j = 0; j < i; ++j) {
            // Complex dependency chain
            double val = sin(i * 0.01) * cos(j * 0.01);
            
            // Ternary operator for conditional move
            val = (val > 0.5) ? val * 2.0 : val / 2.0;
            
            // Function call within inner loop
            val += helper_func(j, val);
            
            sum += val;
        }
        
        // Non-contiguous memory access
        if (i % 3 == 0) {
            sum *= 1.01;
        }
    }
    
    return sum;
}

// Function with computed goto for complex control flow
__attribute__((optimize("O2")))
int computed_goto_test(int x) {
    static void* jump_table[] = {
        &&label_0, &&label_1, &&label_2, &&label_3,
        &&label_4, &&label_5, &&label_6, &&label_7
    };
    
    int result = 0;
    int idx = x % 8;
    
    // Indirect branch
    goto *jump_table[idx];
    
label_0:
    result = x * 2;
    goto end;
label_1:
    result = x + 100;
    goto end;
label_2:
    result = x / 3;
    goto end;
label_3:
    result = x ^ 0xFF;
    goto end;
label_4:
    result = x << 2;
    goto end;
label_5:
    result = x >> 1;
    goto end;
label_6:
    result = ~x;
    goto end;
label_7:
    result = x % 17;
    goto end;
    
end:
    return result;
}

// Function with switch statement testing different patterns
__attribute__((optimize("O3", "funroll-loops")))
double switch_pattern(int* data, int size) {
    double total = 0.0;
    
    #pragma GCC unroll 2
    for (int i = 0; i < size; ++i) {
        // Dense switch cases
        switch (data[i] % 10) {
            case 0:
                total += sin(data[i] * 0.1);
                break;
            case 1:
                total += cos(data[i] * 0.2);
                break;
            case 2:
                total += sqrt(fabs(data[i]));
                break;
            case 3:
                total += pow(1.5, data[i] % 5);
                break;
            case 4:
                total += log(fabs(data[i]) + 1.0);
                break;
            case 5:
                total += tan(data[i] * 0.05);
                break;
            case 6:
                total += exp((data[i] % 3) * 0.3);
                break;
            case 7:
                total += atan(data[i] * 0.01);
                break;
            case 8:
                total += asin((data[i] % 100) / 100.0);
                break;
            case 9:
                total += acos((data[i] % 100) / 100.0);
                break;
        }
        
        // Sparse switch case
        switch (data[i]) {
            case 1000:
                total *= 1.1;
                break;
            case 2000:
                total *= 0.9;
                break;
            case 3000:
                total += 50.0;
                break;
            case 4000:
                total -= 25.0;
                break;
            default:
                // More inline assembly
                asm volatile (
                    "nop\n\t"
                    : : : "r12", "r13", "r14", "r15"
                );
                break;
        }
    }
    
    return total;
}

// Main benchmark function
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
int main() {
    std::srand(std::time(nullptr));
    
    const int N = 1000;
    const int ARRAY_SIZE = 1024;
    
    // Initialize arrays with pattern
    int* src_array = new int[ARRAY_SIZE];
    int* dst_array = new int[ARRAY_SIZE];
    MixedData* mixed_array = new MixedData[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        src_array[i] = std::rand() % 1000;
        mixed_array[i].a = std::rand() % 100;
        mixed_array[i].b = (std::rand() % 1000) / 10.0;
        mixed_array[i].c = std::rand() % 128;
        mixed_array[i].d = (std::rand() % 1000) / 100.0f;
        mixed_array[i].e = std::rand() % 10000;
    }
    
    double total_sum = 0.0;
    
    // Run various computations to trigger scheduler
    total_sum += nested_loop_computation(N);
    
    process_simd(src_array, dst_array, ARRAY_SIZE);
    
    // Process mixed data structure with non-contiguous access
    #pragma GCC unroll 2
    for (int i = 0; i < ARRAY_SIZE; i += 2) {
        mixed_array[i].b = mixed_array[i + 1].a * 0.5;
        mixed_array[i].d = sin(mixed_array[i].b) * cos(mixed_array[i + 1].d);
        
        // More inline assembly
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : : : "xmm0", "xmm1", "xmm2", "xmm3"
        );
    }
    
    total_sum += switch_pattern(src_array, ARRAY_SIZE);
    
    // Use computed goto
    int goto_result = 0;
    for (int i = 0; i < 100; ++i) {
        goto_result ^= computed_goto_test(src_array[i % ARRAY_SIZE]);
    }
    total_sum += goto_result;
    
    // Complex pointer arithmetic
    double* double_ptr = reinterpret_cast<double*>(mixed_array);
    for (int i = 0; i < ARRAY_SIZE * sizeof(MixedData) / sizeof(double); ++i) {
        double_ptr[i] = double_ptr[i] * 0.99 + sin(i * 0.01);
    }
    
    // Final checksum to prevent dead code elimination
    std::cout << "Result checksum: " << total_sum << std::endl;
    std::cout << "Goto result: " << goto_result << std::endl;
    
    // Verify SIMD results
    int simd_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        simd_sum += dst_array[i];
    }
    std::cout << "SIMD sum: " << simd_sum << std::endl;
    
    delete[] src_array;
    delete[] dst_array;
    delete[] mixed_array;
    
    return 0;
}
```

This program combines all the required elements:

1. **Selective Scheduling Activation**: Uses `-fsel-sched-pipelining` and `-fsel-sched-pipelining-outer-loops` flags with appropriate loop structures.

2. **Loop-Carried Dependencies**: `nested_loop_computation` has inner loops with trip counts dependent on outer loop indices.

3. **Mixed Data Types**: `MixedData` struct with non-contiguous memory access patterns.

4. **Inline Assembly**: Multiple `asm volatile` statements with different clobber lists to constrain register allocation.

5. **SIMD Intrinsics**: SSE/AVX operations in `process_simd` with conditional blends.

6. **Computed Goto**: Indirect branches via jump table in `computed_goto_test`.

7. **Switch Statements**: Both dense (0-9) and sparse (1000-4000) switch cases.

8. **Function Calls**: Library calls (`sin`, `cos`, `pow`) and custom helper functions within loops.

9. **Pragmas**: `#pragma GCC unroll` to create larger basic blocks.

10. **Optimization Attributes**: `__attribute__((optimize(...)))` to control optimization levels per function.

The program creates complex scheduling scenarios that should trigger the selective scheduler's debug output, including calls to `dump_insn_rtx_1` for many different instruction types.
