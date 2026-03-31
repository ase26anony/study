#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create varied call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, 
            float fa, float fb, float fc, float fd) {
    volatile int result = a + b + c + d + e + f + g + h;
    result += (int)(fa + fb + fc + fd);
    /* Force register pressure with local computations */
    int t1 = result * 2;
    int t2 = t1 / 3;
    float ft1 = fa * fb;
    float ft2 = fc / fd;
    return result + t2 + (int)(ft1 + ft2);
}

/* Function with pointer arguments and mixed types */
void __attribute__((noinline)) 
process_data(int* arr, float* farr, size_t len, int modifier, 
             double d1, double d2) {
    volatile int acc = 0;
    volatile float facc = 0.0f;
    
    for (size_t i = 0; i < len && i < 8; i++) {
        arr[i] = arr[i] * modifier + (int)(d1 * d2);
        farr[i] = farr[i] * (float)modifier + (float)(d1 - d2);
        acc += arr[i];
        facc += farr[i];
    }
    
    /* Inline assembly to clobber registers */
    __asm__ volatile (
        "# Force clobbering\n"
        : 
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

/* Static function that might be inlined in some compilation modes */
static float __attribute__((noinline))
float_operations(float a, float b, float c, float d, float e, float f) {
    volatile float result = a * b + c * d - e / f;
    
    /* Create register pressure with intermediate values */
    float t1 = result * 2.0f;
    float t2 = t1 + 3.14f;
    float t3 = t2 * 0.5f;
    float t4 = t3 - 1.0f;
    float t5 = t4 * t4;
    
    return result + t5;
}

/* Function using alloca to affect frame pointer */
int* __attribute__((noinline))
create_local_array(int size, int seed) {
    /* alloca encourages frame pointer usage */
    int* local = (int*)alloca(size * sizeof(int));
    volatile int* ptr = local;
    
    for (int i = 0; i < size && i < 16; i++) {
        ptr[i] = seed + i * 3;
    }
    
    /* Return a pointer to stack memory (will be invalid, but creates register pressure) */
    return local;
}

/* Complex function with many live values across calls */
static long __attribute__((noinline))
complex_calculation(int a, int b, int c, int d, 
                    float fa, float fb, float fc,
                    double da, double db) {
    /* Many local variables to create register pressure */
    volatile int v1 = a * b;
    volatile int v2 = c + d;
    volatile int v3 = v1 - v2;
    volatile int v4 = v3 * 2;
    volatile int v5 = v4 / 3;
    
    volatile float fv1 = fa * fb;
    volatile float fv2 = fc + 1.0f;
    volatile float fv3 = fv1 / fv2;
    volatile float fv4 = fv3 * 3.14f;
    
    volatile double dv1 = da * db;
    volatile double dv2 = dv1 + 3.14159;
    volatile double dv3 = dv2 / 2.0;
    
    /* Mix computations */
    long result = (long)v5 + (long)fv4 + (long)dv3;
    
    /* Force register clobbering between computations */
    __asm__ volatile (
        "# Clobber integer registers\n"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
    );
    
    __asm__ volatile (
        "# Clobber floating point registers\n"
        : 
        : 
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    return result * 2 - 1;
}

int main(void) {
    /* Declare many local variables of mixed types */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    volatile int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15;
    volatile float fa = 1.1f, fb = 2.2f, fc = 3.3f, fd = 4.4f;
    volatile float fe = 5.5f, ff = 6.6f, fg = 7.7f, fh = 8.8f;
    volatile double da = 1.01, db = 2.02, dc = 3.03, dd = 4.04;
    volatile int* ptr1 = &a;
    volatile float* ptr2 = &fa;
    volatile long long_result = 0;
    
    /* Array to create more register/memory pressure */
    int arr[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    float farr[8] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f};
    
    /* Control flow to create basic blocks with calls inside */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* First basic block with function calls */
            int sum1 = compute_sum(a, b, c, d, e, f, g, h, fa, fb, fc, fd);
            
            /* Use inline assembly to clobber call-clobbered registers */
            __asm__ volatile (
                "# Force save/restore around this point\n"
                : 
                : 
                : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
            );
            
            /* More computations keeping values live */
            a = sum1 + i;
            b = a * 2 - j;
            fa = fb * fc + (float)sum1;
            
            /* Another call with different signature */
            process_data(arr, farr, 8, iteration + 1, da, db);
            
            /* More live computations */
            c = b + k;
            d = c / 2 + l;
            fb = fa * 1.5f - fd;
        } else {
            /* Different basic block path */
            float float_res = float_operations(fa, fb, fc, fd, fe, ff);
            
            /* Clobber different registers */
            __asm__ volatile (
                "# Clobber more registers\n"
                : 
                : 
                : "rbx", "r12", "r13", "r14", "r15",
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10"
            );
            
            /* Address-taking to affect frame pointer */
            int* local_ptr = create_local_array(8, iteration * 10);
            
            /* Use the pointer (even though it points to expired stack) */
            volatile int dummy = local_ptr != NULL ? 1 : 0;
            
            /* Complex calculation with many live values */
            long_result += complex_calculation(
                a, b, c, d, 
                fa, fb, fc,
                da + iteration, db - iteration
            );
            
            /* More computations */
            e = m + n;
            f = o * e;
            fc = fg * fh / (float)(e + 1);
        }
        
        /* Loop-carried dependencies to keep values live */
        a += iteration;
        b -= iteration;
        fa += (float)iteration * 0.1f;
        fb -= (float)iteration * 0.05f;
        
        /* Another call in the loop tail */
        int sum2 = compute_sum(i, j, k, l, m, n, o, a, fb, fc, fd, fe);
        
        /* Final register clobber */
        __asm__ volatile (
            "# Final clobber set\n"
            : 
            : 
            : "rax", "rdx", "rcx", "rbx", "rsi", "rdi",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
        );
        
        /* Update variables for next iteration */
        i = sum2 % 100;
        j = (sum2 / 100) % 50;
        da += 0.1;
        db -= 0.05;
    }
    
    /* Compute a checksum of all variables */
    long checksum = (long)a + b + c + d + e + f + g + h + i + j + k + l + m + n + o;
    checksum += (long)(fa * 100) + (long)(fb * 100) + (long)(fc * 100) + (long)(fd * 100);
    checksum += (long)(da * 1000) + (long)(db * 1000) + (long)(dc * 1000) + (long)(dd * 1000);
    checksum += long_result;
    
    /* Add array contents */
    for (int idx = 0; idx < 8; idx++) {
        checksum += arr[idx] + (long)(farr[idx] * 100);
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    return (checksum % 256) == 0 ? 0 : 1;
}
