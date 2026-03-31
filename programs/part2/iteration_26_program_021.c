/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug output in GCC's sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fsel-sched-debug sel-sched-trigger.c -o sel-sched-trigger -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#pragma GCC optimize ("O2")
#pragma GCC optimize ("unroll-loops")

/* Mixed data structure to create complex memory access patterns */
struct MixedData {
    int id;
    double value;
    char tag;
    float weight;
    long long timestamp;
};

/* Helper function with varying arguments to create call instructions */
static double __attribute__((noinline)) compute_transform(double x, int scale, char mode) {
    if (mode == 's') {
        return sin(x) * scale;
    } else if (mode == 'c') {
        return cos(x) / (scale + 1);
    } else {
        return pow(x, 1.5) + scale;
    }
}

/* Function with complex loop-carried dependencies */
__attribute__((optimize("O3"))) 
static long long process_nested_loops(struct MixedData* data, int n) {
    long long total = 0;
    
    /* Outer loop with varying trip counts for inner loop */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with data-dependent iteration count */
        for (int j = 0; j < i; ++j) {
            /* Complex data-dependent operations */
            data[i].value += data[j].value * 0.5;
            data[i].weight = (float)(data[j].id % 256) / 255.0f;
            
            /* Conditional move operations */
            total += (data[i].id > data[j].id) ? data[i].timestamp : data[j].timestamp;
            
            /* Inline assembly with clobbers to force scheduler constraints */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : : : "rax", "rbx", "memory"
            );
        }
        
        /* Function call within loop */
        data[i].value = compute_transform(data[i].value, i % 10, 's');
    }
    
    return total;
}

/* SIMD processing function */
__attribute__((target("sse2")))
static void process_simd(int* src, int* dst, int len) {
    /* Process with SIMD intrinsics */
    for (int i = 0; i < len; i += 4) {
        __m128i a = _mm_loadu_si128((__m128i*)(src + i));
        __m128i b = _mm_add_epi32(a, a);
        __m128i c = _mm_mullo_epi16(b, _mm_set1_epi32(0x00010001));
        _mm_storeu_si128((__m128i*)(dst + i), c);
        
        /* Mix with scalar operations */
        dst[i] ^= i;
        dst[i+1] += src[i+1] >> 3;
    }
    
    /* Tail processing */
    for (int i = (len / 4) * 4; i < len; ++i) {
        dst[i] = src[i] * 2 + (i % 7);
    }
}

/* Function with computed goto for complex control flow */
static int computed_goto_processor(int* values, int n, int mode) {
    static void* jump_table[] = {
        &&case_0, &&case_1, &&case_2, &&case_3,
        &&case_4, &&case_5, &&case_default
    };
    
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        int idx = values[i] % 7;
        if (idx < 0) idx = 6;  /* Default case index */
        
        /* Computed goto */
        goto *jump_table[idx];
        
    case_0:
        result += values[i] * 2;
        continue;
    case_1:
        result += values[i] >> 1;
        continue;
    case_2:
        result ^= values[i];
        continue;
    case_3:
        result = (result > 1000) ? result - values[i] : result + values[i];
        continue;
    case_4:
        result *= (values[i] % 5) + 1;
        continue;
    case_5:
        result = (result & 0xFF) | (values[i] << 8);
        continue;
    case_default:
        result = ~result;
        continue;
    }
    
    return result;
}

/* Matrix-like processing with non-contiguous access */
static double matrix_style_process(double** matrix, int rows, int cols) {
    double sum = 0.0;
    
    #pragma GCC unroll 2
    for (int i = 0; i < rows; ++i) {
        /* Strided access pattern */
        for (int j = 0; j < cols; j += 2) {
            matrix[i][j] = matrix[i][j] * 1.5 + sin(matrix[i][j]);
            
            /* Data-dependent conditional */
            if (j > 0 && (i * j) % 7 == 0) {
                matrix[i][j] += matrix[i-1][j-1] * 0.3;
            }
            
            sum += matrix[i][j];
        }
    }
    
    return sum;
}

/* Switch statement with mixed dense/sparse cases */
static int switch_processor(int value) {
    int result = 0;
    
    switch (value) {
        /* Dense range */
        case 0:  result = 1; break;
        case 1:  result = 3; break;
        case 2:  result = 7; break;
        case 3:  result = 15; break;
        case 4:  result = 31; break;
        
        /* Small gap */
        case 10: result = 63; break;
        case 11: result = 127; break;
        
        /* Large sparse range */
        case 100: result = 255; break;
        case 200: result = 511; break;
        case 300: result = 1023; break;
        
        default:
            /* Complex default computation */
            result = (value & 1) ? value * 2 : value / 2;
            result ^= 0xABCD;
            break;
    }
    
    /* Additional computation after switch */
    result = (result > 0) ? result : -result;
    
    /* Another inline assembly with different clobbers */
    asm volatile (
        "mov %%rax, %%rbx\n\t"
        : : : "rax", "rbx", "cc"
    );
    
    return result;
}

/* Main benchmark function */
__attribute__((noinline))
static unsigned long long run_benchmark() {
    const int DATA_SIZE = 256;
    const int MATRIX_SIZE = 32;
    
    /* Initialize data structures */
    struct MixedData* data = (struct MixedData*)malloc(DATA_SIZE * sizeof(struct MixedData));
    int* int_array1 = (int*)malloc(DATA_SIZE * sizeof(int));
    int* int_array2 = (int*)malloc(DATA_SIZE * sizeof(int));
    double** matrix = (double**)malloc(MATRIX_SIZE * sizeof(double*));
    
    for (int i = 0; i < DATA_SIZE; ++i) {
        data[i].id = i;
        data[i].value = (i * 1.5) / (i + 1);
        data[i].tag = 'A' + (i % 26);
        data[i].weight = (float)(i % 100) / 99.0f;
        data[i].timestamp = 1000 + i * 7;
        
        int_array1[i] = (i * 3) ^ 0x55;
        int_array2[i] = 0;
    }
    
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        matrix[i] = (double*)malloc(MATRIX_SIZE * sizeof(double));
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            matrix[i][j] = (i * MATRIX_SIZE + j) * 0.1;
        }
    }
    
    unsigned long long checksum = 0;
    
    /* Execute various computation patterns */
    checksum += process_nested_loops(data, DATA_SIZE / 4);
    
    process_simd(int_array1, int_array2, DATA_SIZE);
    for (int i = 0; i < DATA_SIZE; ++i) {
        checksum += int_array2[i];
    }
    
    checksum += computed_goto_processor(int_array1, DATA_SIZE, 1);
    
    double matrix_sum = matrix_style_process(matrix, MATRIX_SIZE / 2, MATRIX_SIZE / 2);
    checksum += (unsigned long long)(matrix_sum * 1000);
    
    for (int i = 0; i < DATA_SIZE; i += 8) {
        checksum += switch_processor(int_array1[i] % 400);
    }
    
    /* Additional mixed computation */
    for (int i = 0; i < DATA_SIZE; ++i) {
        /* Complex expression with multiple operations */
        double temp = data[i].value;
        temp = (temp > 0.5) ? temp * 0.8 : temp * 1.2;
        temp = sin(temp) + cos(temp * 0.3);
        
        /* Pointer arithmetic with casting */
        char* byte_ptr = (char*)&data[i];
        for (int b = 0; b < 4; ++b) {
            checksum += byte_ptr[b];
        }
        
        /* Memory barrier */
        asm volatile ("" ::: "memory");
    }
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        free(matrix[i]);
    }
    free(matrix);
    free(data);
    free(int_array1);
    free(int_array2);
    
    return checksum;
}

int main() {
    printf("Starting selective scheduler trigger program...\n");
    
    /* Run multiple iterations to increase scheduling opportunities */
    unsigned long long final_checksum = 0;
    for (int iter = 0; iter < 3; ++iter) {
        final_checksum ^= run_benchmark();
        
        /* Progress indicator that won't be optimized away */
        printf("Iteration %d complete\n", iter);
        fflush(stdout);
    }
    
    printf("Final checksum: %llu\n", final_checksum);
    printf("Program completed successfully.\n");
    
    return 0;
}
