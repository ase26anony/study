/* test_sched_context.c - Complex scheduling test for GCC Haifa scheduler */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* Force inlining to create larger basic blocks */
#define ALWAYS_INLINE __attribute__((always_inline))

/* Vector types for wide operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Volatile function to prevent optimization */
static volatile int global_counter = 0;

/* ========== TEST FUNCTION 1: Mixed Integer/FP with Dependency Chains ========== */
ALWAYS_INLINE static float complex_math_chain(int a, int b, float c, float d) {
    /* Long dependency chain mixing int and FP */
    int t1 = a * b + 7;
    float t2 = c * d + 3.14159f;
    int t3 = t1 >> 2;
    float t4 = t2 / (float)t3;
    int t5 = t3 ^ 0x55AA55AA;
    float t6 = t4 * sinf(t4);
    int t7 = t5 * global_counter;
    float t8 = t6 + cosf(t6);
    return t8 * (float)t7;
}

/* ========== TEST FUNCTION 2: Wide Basic Block with Unrolled Loop ========== */
static double matrix_accumulate(double* restrict A, double* restrict B, int n) {
    double sum = 0.0;
    
    /* Unrolled loop creates wide basic block */
    for (int i = 0; i < n; i += 4) {
        /* Independent chains that can be scheduled in parallel */
        double a1 = A[i] * B[i];
        double a2 = A[i+1] * B[i+1];
        double a3 = A[i+2] * B[i+2];
        double a4 = A[i+3] * B[i+3];
        
        /* Dependent operations */
        double s1 = a1 + a2;
        double s2 = a3 + a4;
        double s3 = s1 * s2;
        double s4 = sqrt(fabs(s3));
        
        /* Mix with integer operations */
        int idx = i & 0xFF;
        double scale = 1.0 + (idx * 0.01);
        sum += s4 * scale;
        
        /* Memory aliasing prevention */
        A[i] = A[i] * 0.99;
        B[i+1] = B[i+1] * 1.01;
    }
    return sum;
}

/* ========== TEST FUNCTION 3: Vector Operations with SIMD ========== */
ALWAYS_INLINE static v4sf vector_ops(v4sf a, v4sf b, v4sf c) {
    /* SIMD operations that may use target-specific scheduling */
    v4sf r1 = a + b;
    v4sf r2 = a * c;
    v4sf r3 = r1 - b;
    v4sf r4 = r2 / (v4sf){2.0f, 2.0f, 2.0f, 2.0f};
    v4sf r5 = r3 * r4;
    v4sf r6 = __builtin_ia32_sqrtps(r5);  /* x86-specific intrinsic */
    return r6;
}

/* ========== TEST FUNCTION 4: Conditional Speculative Scheduling ========== */
static int speculative_loop(int* data, int n) {
    int total = 0;
    
    /* Loop with condition that may cause speculative scheduling */
    for (int i = 0; i < n; i++) {
        int val = data[i];
        
        /* Complex conditional chain */
        if (val > 100) {
            val = val * 3 + 7;
            val = val >> 2;
            val = val ^ 0x12345678;
        } else if (val < -50) {
            val = val / 2;
            val = val * val;
            val = val - 1000;
        } else {
            val = val + global_counter;
            val = val * 2;
        }
        
        /* Mixed FP operation in integer loop */
        float fval = (float)val * 1.5f;
        val = (int)fval;
        
        total += val;
        
        /* Function call with side effect */
        global_counter++;
    }
    
    return total;
}

/* ========== TEST FUNCTION 5: Large Ready List Creation ========== */
static double wide_ready_list(int iterations) {
    /* Many independent variables to fill ready list */
    double a = 1.0, b = 2.0, c = 3.0, d = 4.0, e = 5.0;
    double f = 6.0, g = 7.0, h = 8.0, i = 9.0, j = 10.0;
    double k = 11.0, l = 12.0, m = 13.0, n = 14.0, o = 15.0;
    double p = 16.0, q = 17.0, r = 18.0, s = 19.0, t = 20.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Many independent operations that can be reordered */
        a = b * c + d;
        e = f / g - h;
        i = j * k * l;
        m = n + o - p;
        q = r * s / t;
        
        /* Create some dependencies */
        double tmp1 = a + e;
        double tmp2 = i + m;
        double tmp3 = q * tmp1;
        double result = tmp3 / tmp2;
        
        /* Use result to prevent elimination */
        a = result * 0.5;
    }
    
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t;
}

/* ========== TEST FUNCTION 6: Inline Assembly Scheduling Barrier ========== */
static int asm_scheduling_test(int x, int y) {
    int result;
    
    /* Inline assembly creates scheduling barriers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "imull $7, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (x), "r" (y)
        : "%eax"
    );
    
    /* Operations around assembly */
    int t1 = result * 3;
    float t2 = (float)t1 * 1.234f;
    
    asm volatile (
        "cpuid\n\t"  /* Serializing instruction */
        : : : "%eax", "%ebx", "%ecx", "%edx"
    );
    
    int t3 = (int)t2 + global_counter;
    return t3;
}

/* ========== MAIN DRIVER ========== */
int main(int argc, char** argv) {
    clock_t start = clock();
    double total_result = 0.0;
    
    /* Seed for reproducible results */
    srand(42);
    
    /* Test 1: Mixed operations */
    printf("Test 1: Mixed integer/FP chains\n");
    for (int i = 0; i < 1000; i++) {
        float r = complex_math_chain(i, i+1, i*0.1f, i*0.2f);
        total_result += r;
    }
    
    /* Test 2: Matrix-style wide blocks */
    printf("Test 2: Wide basic blocks\n");
    double A[256], B[256];
    for (int i = 0; i < 256; i++) {
        A[i] = (double)rand() / RAND_MAX;
        B[i] = (double)rand() / RAND_MAX;
    }
    total_result += matrix_accumulate(A, B, 256);
    
    /* Test 3: Vector operations */
    printf("Test 3: Vector/SIMD operations\n");
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    
    for (int i = 0; i < 500; i++) {
        v4sf r = vector_ops(vec1, vec2, vec3);
        /* Extract and accumulate */
        float arr[4];
        memcpy(arr, &r, sizeof(arr));
        total_result += arr[0] + arr[1] + arr[2] + arr[3];
    }
    
    /* Test 4: Speculative scheduling */
    printf("Test 4: Speculative loops\n");
    int data[500];
    for (int i = 0; i < 500; i++) {
        data[i] = rand() % 200 - 100;
    }
    total_result += (double)speculative_loop(data, 500);
    
    /* Test 5: Large ready list */
    printf("Test 5: Large ready list creation\n");
    total_result += wide_ready_list(100);
    
    /* Test 6: Assembly scheduling */
    printf("Test 6: Inline assembly barriers\n");
    for (int i = 0; i < 100; i++) {
        total_result += (double)asm_scheduling_test(i, i*2);
    }
    
    /* Additional complex block with switch statement */
    printf("Test 7: Switch with state saving\n");
    for (int i = 0; i < 200; i++) {
        double val = total_result * 0.01;
        switch (i % 5) {
            case 0: val = sin(val); break;
            case 1: val = cos(val); break;
            case 2: val = exp(val); break;
            case 3: val = log(fabs(val) + 1.0); break;
            case 4: val = val * val + 1.0; break;
        }
        total_result += val;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nFinal checksum: %f\n", total_result);
    printf("Elapsed time: %f seconds\n", elapsed);
    printf("Global counter: %d\n", global_counter);
    
    return (total_result > 0.0) ? 0 : 1;
}
