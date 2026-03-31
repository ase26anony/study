/* reload_coverage.c - Test program to trigger various reload types in GCC */
#include <stdint.h>
#include <stdlib.h>

/* Force frame pointer usage */
#pragma GCC optimize ("no-omit-frame-pointer")

/* Complex structure to force address computations */
struct NestedData {
    int values[8];
    struct NestedData *next;
    volatile int flags;
    double coords[3];
};

/* Global arrays to create complex addressing */
volatile int global_array[256][16];
static double static_matrix[32][32];
struct NestedData data_pool[64];

/* ========== TEST 1: Complex Array Addressing ========== */
/* Targets: RELOAD_FOR_INPUT, RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
static inline __attribute__((always_inline)) 
int test_complex_addressing(int idx1, int idx2, int idx3, int idx4) {
    /* Many local variables to consume registers */
    volatile int v1 = idx1 * 2;
    volatile int v2 = idx2 + 7;
    volatile int v3 = idx3 - 5;
    volatile int v4 = idx4 * 3;
    int v5 = v1 + v2;
    int v6 = v3 * v4;
    int v7 = v5 - v6;
    int v8 = v2 * v3;
    int v9 = v4 + v1;
    int v10 = v7 * v8;
    int v11 = v9 - v10;
    int v12 = v5 * v6;
    int v13 = v7 + v8;
    int v14 = v9 * v10;
    int v15 = v11 + v12;
    int v16 = v13 - v14;
    
    /* Complex multi-level array indexing - forces address reloads */
    int result = 0;
    
    /* This creates RELOAD_FOR_INPUT_ADDRESS: address computation needs register */
    result += global_array[v1 + v2][v3 * 2];
    
    /* More complex: nested addressing with multiple computations */
    result += global_array[(v4 + v5) * 2][(v6 + v7) % 16];
    
    /* Even more complex: address with three levels of computation */
    int temp_idx1 = (v8 * v9 + v10) % 256;
    int temp_idx2 = (v11 * v12 - v13) % 16;
    result += global_array[temp_idx1][temp_idx2];
    
    /* Structure access with pointer arithmetic */
    struct NestedData *ptr = &data_pool[(v14 + v15) % 64];
    result += ptr->values[(v16 * 3) % 8];
    
    /* Multi-dimensional array with volatile */
    result += static_matrix[v1 % 32][v2 % 32];
    
    return result;
}

/* ========== TEST 2: Inline Assembly with Multiple Outputs ========== */
/* Targets: RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS, RELOAD_FOR_OPERAND_ADDRESS */
static __attribute__((noinline))
int test_asm_reloads(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Many local variables to create register pressure */
    int x1 = a * b + c;
    int x2 = d * e - f;
    int x3 = g * h + a;
    int x4 = b * c - d;
    int x5 = e * f + g;
    int x6 = h * a - b;
    int x7 = c * d + e;
    int x8 = f * g - h;
    
    volatile int mem1, mem2, mem3;
    int output1, output2, output3;
    
    /* Complex memory operand in inline asm - forces address reloads */
    asm volatile (
        /* Multiple outputs to different memory locations with complex addresses */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out2]\n\t"
        "subl %[in3], %[out3]\n\t"
        : [out1] "=m" (global_array[x1 % 256][x2 % 16]),  /* RELOAD_FOR_OUTPUT_ADDRESS */
          [out2] "=m" (global_array[(x3 + x4) % 256][(x5 * 2) % 16]), /* Complex address */
          [out3] "=m" (mem1)  /* Simple memory */
        : [in1] "r" (x1),
          [in2] "r" (x2),
          [in3] "r" (x3)
        : "memory", "cc"
    );
    
    /* More asm with input memory operands that need address computation */
    int temp_array[8];
    for (int i = 0; i < 8; i++) temp_array[i] = i * x4;
    
    asm volatile (
        "movl (%[addr1]), %%eax\n\t"
        "addl (%[addr2]), %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=r" (output1)
        : [addr1] "r" (&temp_array[x5 % 8]),  /* RELOAD_FOR_OPERAND_ADDRESS */
          [addr2] "r" (&temp_array[x6 % 8])   /* Another address reload */
        : "%eax", "memory"
    );
    
    /* Pointer chasing with complex address computation */
    struct NestedData *current = &data_pool[0];
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += current->values[i];
        /* Force address computation for next pointer */
        current = current->next ? current->next : &data_pool[(i * x7) % 64];
    }
    
    return output1 + sum + mem1;
}

/* ========== TEST 3: Mixed Operand Types and Volatile ========== */
/* Targets: RELOAD_OTHER, RELOAD_FOR_OTHER_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
static __attribute__((noinline))
int test_mixed_operands(double d1, double d2, double d3) {
    /* Mix different types to use different register classes */
    volatile float f1 = d1 * 2.0f;
    volatile float f2 = d2 / 3.0f;
    volatile float f3 = d3 + 1.0f;
    
    int i1 = (int)f1 * 100;
    int i2 = (int)f2 * 200;
    int i3 = (int)f3 * 300;
    
    /* Complex expression with many intermediate values */
    double arr[4] = {d1, d2, d3, d1 + d2};
    volatile double *volatile ptr_arr[4];
    
    for (int i = 0; i < 4; i++) {
        ptr_arr[i] = &arr[i];
    }
    
    /* Multiple volatile accesses force spills */
    double sum = 0.0;
    sum += *ptr_arr[0] * f1;
    sum += *ptr_arr[1] * f2;
    sum += *ptr_arr[2] * f3;
    sum += *ptr_arr[3] * (f1 + f2);
    
    /* Use __builtin_expect to create data dependencies */
    if (__builtin_expect(i1 > i2, 0)) {
        sum += static_matrix[i1 % 32][i2 % 32];
    }
    
    if (__builtin_expect(i3 < i1, 1)) {
        sum += static_matrix[i3 % 32][i1 % 32];
    }
    
    /* Complex pointer expression that needs OPADDR_ADDR reload */
    double ***triple_ptr = (double***)&ptr_arr;
    double indirect = ***triple_ptr;
    
    return (int)(sum + indirect) + i1 + i2 + i3;
}

/* ========== TEST 4: Loop Unrolling with Register Pressure ========== */
#pragma GCC unroll 4
static __attribute__((noinline))
int test_unrolled_loops(int iterations) {
    int results[8] = {0};
    volatile int counters[8] = {0};
    
    /* Unrolled loop creates many similar but distinct operations */
    for (int i = 0; i < iterations; i++) {
        /* Each iteration uses different local variables */
        int a = i * 3;
        int b = i * 5 + 1;
        int c = i * 7 - 2;
        int d = i * 11 + 3;
        int e = i * 13 - 4;
        int f = i * 17 + 5;
        int g = i * 19 - 6;
        int h = i * 23 + 7;
        
        /* Complex addressing in loop body */
        results[0] += global_array[a % 256][b % 16];
        results[1] += global_array[(c + d) % 256][(e * 2) % 16];
        results[2] += global_array[(f * g) % 256][(h / 2) % 16];
        results[3] += data_pool[i % 64].values[(a + b) % 8];
        
        /* Force spills with volatile */
        counters[0] = a;
        counters[1] = b;
        counters[2] = c;
        counters[3] = d;
        counters[4] = e;
        counters[5] = f;
        counters[6] = g;
        counters[7] = h;
    }
    
    int total = 0;
    for (int i = 0; i < 8; i++) {
        total += results[i] + counters[i];
    }
    
    return total;
}

/* ========== MAIN DRIVER ========== */
int main(int argc, char **argv) {
    /* Initialize data structures */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            data_pool[i].values[j] = i * 8 + j;
        }
        data_pool[i].next = &data_pool[(i + 1) % 64];
        data_pool[i].flags = i;
        for (int j = 0; j < 3; j++) {
            data_pool[i].coords[j] = i * 0.1 + j * 0.01;
        }
    }
    
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 16; j++) {
            global_array[i][j] = i * 16 + j;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            static_matrix[i][j] = i * 0.5 + j * 0.25;
        }
    }
    
    /* Call all test functions to trigger different reload patterns */
    int result = 0;
    
    /* Test 1: Complex addressing */
    for (int i = 0; i < 100; i++) {
        result += test_complex_addressing(i, i+1, i+2, i+3);
    }
    
    /* Test 2: Inline assembly */
    result += test_asm_reloads(1, 2, 3, 4, 5, 6, 7, 8);
    result += test_asm_reloads(9, 10, 11, 12, 13, 14, 15, 16);
    
    /* Test 3: Mixed operands */
    result += test_mixed_operands(1.0, 2.0, 3.0);
    result += test_mixed_operands(4.0, 5.0, 6.0);
    
    /* Test 4: Unrolled loops */
    result += test_unrolled_loops(50);
    result += test_unrolled_loops(25);
    
    /* Ensure result is used */
    volatile int final_result = result;
    
    return final_result % 256;
}
