/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
static void high_pressure_loop(float *restrict a, float *restrict b, 
                               float *restrict c, int size) {
    int i;
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8;
    float t9, t10, t11, t12, t13, t14, t15, t16;
    
    for (i = 0; i < size; i += 8) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i] + vol_float1;
        t2 = a[i+1] * b[i+1] - vol_float2;
        t3 = a[i+2] / (b[i+2] + 1.0f);
        t4 = a[i+3] * b[i+3] * vol_float1;
        
        /* Group 2: More independent operations */
        t5 = sqrtf(fabsf(a[i+4])) + t1;
        t6 = sqrtf(fabsf(a[i+5])) - t2;
        t7 = sqrtf(fabsf(a[i+6])) * t3;
        t8 = sqrtf(fabsf(a[i+7])) / t4;
        
        /* Group 3: Mixed operations creating dependencies */
        t9 = t1 + t5 * vol_var1;
        t10 = t2 - t6 / (vol_var2 + 1);
        t11 = t3 * t7 + vol_float1;
        t12 = t4 / t8 - vol_float2;
        
        /* Group 4: Final computations with artificial delays */
        t13 = t9 * 1.1f;
        t14 = t10 * 1.2f;
        t15 = t11 * 1.3f;
        t16 = t12 * 1.4f;
        
        /* Store results with memory barriers */
        c[i] = t13;
        asm volatile("" ::: "memory");
        c[i+1] = t14;
        asm volatile("" ::: "memory");
        c[i+2] = t15;
        c[i+3] = t16;
        c[i+4] = t13 + t14;
        c[i+5] = t15 - t16;
        c[i+6] = t13 * t16;
        c[i+7] = t14 / t15;
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
static float mixed_dependency_compute(float x, float y, int n) {
    float result = 0.0f;
    float temp1, temp2, temp3, temp4;
    int i;
    
    /* Long latency operation to create delays */
    temp1 = x / y;  /* Floating point divide has high latency */
    
    /* Inline assembly to clobber registers and force spills */
    asm volatile (
        "mov %0, %%eax\n\t"
        "mov %1, %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "+r" (vol_var1)
        : "r" (vol_var2)
        : "%eax", "%ebx", "cc"
    );
    
    /* Independent chains with different priorities */
    for (i = 0; i < n; i++) {
        /* Chain A: Critical path */
        temp2 = temp1 * x + i;
        
        /* Chain B: Less critical independent operations */
        temp3 = y * i - temp1;
        
        /* Chain C: Memory intensive */
        temp4 = vol_float1 * temp2 + vol_float2 * temp3;
        
        /* Conditional to create control flow variations */
        if (temp4 > 100.0f) {
            result += temp2 * 0.5f;
        } else {
            result += temp3 * 2.0f;
        }
        
        /* Artificial dependency on volatile */
        temp1 += vol_var1 * 0.01f;
    }
    
    /* More register pressure at the end */
    asm volatile (
        "mov %0, %%ecx\n\t"
        "mov %1, %%edx\n\t"
        "imul %%edx, %%ecx\n\t"
        "mov %%ecx, %0\n\t"
        : "+r" (vol_var2)
        : "r" (i)
        : "%ecx", "%edx", "cc"
    );
    
    return result;
}

/* Function with many independent instructions for candidate selection */
__attribute__((noinline))
static void independent_instruction_block(int *restrict out, 
                                         const int *restrict in, int n) {
    int i;
    /* Many temporary variables to create scheduling candidates */
    int r0, r1, r2, r3, r4, r5, r6, r7;
    int r8, r9, r10, r11, r12, r13, r14, r15;
    
    for (i = 0; i < n; i += 16) {
        /* Block of independent integer operations */
        r0 = in[i] + vol_var1;
        r1 = in[i+1] - vol_var2;
        r2 = in[i+2] * 3;
        r3 = in[i+3] / 2;
        r4 = in[i+4] & 0xFF;
        r5 = in[i+5] | 0x55;
        r6 = in[i+6] ^ r0;
        r7 = in[i+7] << 2;
        
        /* Second block - independent from first */
        r8 = in[i+8] + r1;
        r9 = in[i+9] - r2;
        r10 = in[i+10] * r3;
        r11 = in[i+11] / (r4 + 1);
        r12 = in[i+12] & r5;
        r13 = in[i+13] | r6;
        r14 = in[i+14] ^ r7;
        r15 = in[i+15] << 1;
        
        /* Store results - creates dependencies for scheduling */
        out[i] = r0 + r8;
        out[i+1] = r1 + r9;
        out[i+2] = r2 + r10;
        out[i+3] = r3 + r11;
        out[i+4] = r4 + r12;
        out[i+5] = r5 + r13;
        out[i+6] = r6 + r14;
        out[i+7] = r7 + r15;
        out[i+8] = r8 * 2;
        out[i+9] = r9 * 3;
        out[i+10] = r10 / 2;
        out[i+11] = r11 / 3;
        out[i+12] = r12 & 0x0F;
        out[i+13] = r13 | 0xF0;
        out[i+14] = r14 ^ 0xAA;
        out[i+15] = r15 << 2;
    }
}

int main(void) {
    float *array_a, *array_b, *array_c;
    int *int_array_in, *int_array_out;
    float total = 0.0f;
    int i;
    
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    array_a = (float*)malloc(ARRAY_SIZE * sizeof(float));
    array_b = (float*)malloc(ARRAY_SIZE * sizeof(float));
    array_c = (float*)malloc(ARRAY_SIZE * sizeof(float));
    int_array_in = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int_array_out = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!array_a || !array_b || !array_c || !int_array_in || !int_array_out) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    for (i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (float)rand() / RAND_MAX * 100.0f;
        array_b[i] = (float)rand() / RAND_MAX * 100.0f + 0.1f;
        int_array_in[i] = rand() % 1000;
    }
    
    /* Perform computations to trigger scheduling analysis */
    for (i = 0; i < ITERATIONS; i++) {
        /* Call high pressure function - creates register pressure */
        high_pressure_loop(array_a, array_b, array_c, ARRAY_SIZE);
        
        /* Call mixed dependency function - creates delays and priority variations */
        total += mixed_dependency_compute(array_a[i % ARRAY_SIZE], 
                                         array_b[i % ARRAY_SIZE], 10);
        
        /* Call independent instruction block - creates many scheduling candidates */
        independent_instruction_block(int_array_out, int_array_in, ARRAY_SIZE);
        
        /* Modify volatile variables to affect dependencies */
        if (i % 1000 == 0) {
            vol_var1 = (vol_var1 * 3 + 7) % 100;
            vol_var2 = (vol_var2 * 5 + 11) % 100;
            vol_float1 = sinf((float)i) * 10.0f;
            vol_float2 = cosf((float)i) * 10.0f;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += array_c[i] + int_array_out[i];
    }
    checksum += total;
    
    printf("Result checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(int_array_in);
    free(int_array_out);
    
    return 0;
}
