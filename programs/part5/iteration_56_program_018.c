/* Test program to trigger early rematerialization virtual register creation */
/* Compile with: gcc -O2 -m32 -fno-optimize-sibling-calls -fdump-rtl-early-remat test.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Global arrays for address calculations */
static int global_array[1024];
static double global_double[512];
static char global_char[2048];

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int *data) {
    /* Large immediate constants that need rematerialization */
    const long long big_const_1 = 0x123456789ABCDEF0LL;
    const long long big_const_2 = 0xFEDCBA9876543210LL;
    const unsigned long large_addr = 0xDEADBEEF;
    
    /* Invariant pointers that will be used in loop */
    int *invariant_ptr1 = &global_array[0];
    double *invariant_ptr2 = &global_double[0];
    char *invariant_ptr3 = &global_char[0];
    
    /* Many local variables with overlapping live ranges */
    int sum = 0;
    int temp1, temp2, temp3, temp4, temp5;
    long long acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Complex loop with invariant usage */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places */
        temp1 = invariant_ptr1[i % 1024];
        temp2 = *(invariant_ptr1 + (i * 7) % 1024);
        
        /* Use large constants in calculations */
        acc1 = (big_const_1 * i) / 137;
        acc2 = (big_const_2 + i) ^ large_addr;
        
        /* More invariant usage */
        temp3 = (int)(invariant_ptr2[i % 512] * 100.0);
        temp4 = invariant_ptr3[i % 2048] * 2;
        
        /* Complex expression with many live values */
        temp5 = (temp1 * temp2 + temp3 - temp4) ^ (int)(acc1 & 0xFFFFFFFF);
        
        /* Use all accumulators */
        acc3 += acc1 - acc2 + temp5;
        
        /* More operations to extend live ranges */
        sum += temp5 * (i & 0xFF);
        sum ^= (int)(acc2 >> 32);
        
        /* Conditional to create control flow complexity */
        if (i % 3 == 0) {
            acc1 += big_const_1 >> (i % 16);
            temp2 = invariant_ptr1[(i + 1) % 1024];
        } else if (i % 3 == 1) {
            acc2 ^= large_addr * i;
            temp3 = (int)(invariant_ptr2[(i + 2) % 512]);
        }
        
        /* Use data array with address calculation */
        data[i] = sum + temp1 + temp2 + temp3 + temp4 + (int)acc3;
    }
    
    /* Final computation using all variables */
    return sum + (int)(acc1 >> 32) + (int)(acc2 >> 32) + (int)acc3;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b, int c) {
    /* Register variables to force specific allocation */
    register int r1 asm("eax") = a;
    register int r2 asm("ebx") = b;
    register int r3 asm("ecx") = c;
    register int r4 asm("edx");
    register int r5 asm("esi");
    register int r6 asm("edi");
    
    int result1, result2, result3;
    
    /* Multi-output inline assembly with many clobbers */
    asm volatile (
        "movl %[r1], %%eax\n\t"
        "movl %[r2], %%ebx\n\t"
        "movl %[r3], %%ecx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%ebx, %[out2]\n\t"
        "leal (%%ecx, %%eax, 2), %[out3]"
        : [out1] "=&r" (result1), 
          [out2] "=&r" (result2),
          [out3] "=&r" (result3)
        : [r1] "r" (r1), [r2] "r" (r2), [r3] "r" (r3)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use results in complex expressions */
    r4 = result1 * 0x12345678;
    r5 = result2 ^ 0x9ABCDEF0;
    r6 = result3 + 0x55555555;
    
    /* More inline assembly using specific registers */
    asm volatile (
        "rdtsc\n\t"
        "movl %%eax, %0\n\t"
        "movl %%edx, %1"
        : "=r" (r4), "=r" (r5)
        :
        : "eax", "edx", "cc"
    );
    
    /* Complex chain of operations */
    for (int i = 0; i < 100; i++) {
        r1 = (r1 * 1103515245 + 12345) & 0x7FFFFFFF;
        r2 = (r2 ^ r4) + r5;
        r3 = (r3 * r6) | r1;
        
        /* Use global array with address calculation */
        global_array[i % 1024] = r1 + r2 + r3;
    }
    
    return r1 + r2 + r3 + r4 + r5 + r6 + result1 + result2 + result3;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int func_complex_control(int seed, int *output) {
    /* Many local variables with overlapping lives */
    int a = seed * 3;
    int b = seed + 0x7FFFFFFF;
    int c = seed ^ 0x12345678;
    int d = seed * 0x55555555;
    int e = seed - 0x33333333;
    int f = seed | 0xAAAAAAAA;
    int g = seed & 0xCCCCCCCC;
    int h = seed / 7;
    
    /* Labels for computed goto */
    void *labels[] = { &&label0, &&label1, &&label2, &&label3, 
                      &&label4, &&label5, &&label6, &&label7 };
    
    int sum = 0;
    
    /* Nested loops with switch inside */
    for (int i = 0; i < 1000; i++) {
        /* Update all variables to keep them live */
        a = (a * 13 + 7) & 0xFFF;
        b = (b ^ i) + 1;
        c = c * 3 - 5;
        d = d + global_array[i % 1024];
        e = e ^ global_char[i % 2048];
        f = f | (i * 2);
        g = g & ~i;
        h = h + (i % 19);
        
        /* Complex switch with many cases */
        switch (i % 8) {
            case 0:
                sum += a * b;
                /* Use computed goto */
                goto *labels[i % 8];
            label0:
                a += c;
                break;
            case 1:
                sum += c - d;
                goto *labels[1];
            label1:
                b ^= e;
                break;
            case 2:
                sum += e * f;
                goto *labels[2];
            label2:
                c += g;
                break;
            case 3:
                sum += g / (h + 1);
                goto *labels[3];
            label3:
                d |= f;
                break;
            case 4:
                sum += a ^ b ^ c;
                goto *labels[4];
            label4:
                e = e * 2 + 1;
                break;
            case 5:
                sum += d + e + f;
                goto *labels[5];
            label5:
                f = f ^ g ^ h;
                break;
            case 6:
                sum += g * h;
                goto *labels[6];
            label6:
                h = h + a + b;
                break;
            case 7:
                sum += a + b + c + d;
                goto *labels[7];
            label7:
                a = b = c = i;
                break;
        }
        
        /* More operations extending live ranges */
        output[i % 256] = a + b + c + d + e + f + g + h + sum;
        
        /* Inner loop with more variables */
        for (int j = 0; j < 10; j++) {
            int temp = (a * j + b * (j + 1) + c * (j + 2)) & 0xFF;
            global_double[(i + j) % 512] = (double)temp / 256.0;
            sum += temp;
        }
    }
    
    /* Final computation using all variables */
    return sum + a + b + c + d + e + f + g + h;
}

/* Function D: Mixed patterns for maximum pressure */
__attribute__((noinline, noclone))
int func_mixed_patterns(double *data, int size) {
    /* Register variables mixed with regular ones */
    register double r1 asm("xmm0");
    register double r2 asm("xmm1");
    register int r3 asm("eax");
    
    /* Many local doubles causing FP register pressure */
    double d1 = 3.141592653589793;
    double d2 = 2.718281828459045;
    double d3 = 1.414213562373095;
    double d4 = 1.618033988749895;
    double d5 = 0.577215664901532;
    double d6 = 0.301029995663981;
    double d7 = 0.693147180559945;
    double d8 = 0.434294481903252;
    
    int int1 = 0x12345678;
    int int2 = 0x9ABCDEF0;
    long long ll1 = 0xFEDCBA9876543210LL;
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < size; i++) {
        /* FP operations */
        d1 = d1 * d2 + d3;
        d2 = d2 / d4 - d5;
        d3 = d3 + d6 * d7;
        d4 = d4 - d8 / d1;
        
        /* Integer operations */
        int1 = (int1 * 1103515245 + 12345) & 0x7FFFFFFF;
        int2 = int2 ^ (i * 0x55555555);
        ll1 = ll1 + (long long)int1 * int2;
        
        /* Use register variables */
        r1 = d1 + d2;
        r2 = d3 * d4;
        r3 = int1 + int2;
        
        /* Inline assembly accessing specific registers */
        asm volatile (
            "movsd %[r1], %%xmm0\n\t"
            "movsd %[r2], %%xmm1\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "movsd %%xmm0, %[out]"
            : [out] "=m" (data[i])
            : [r1] "m" (r1), [r2] "m" (r2)
            : "xmm0", "xmm1", "memory"
        );
        
        /* More operations to extend live ranges */
        d5 = d5 + (double)int1 / 256.0;
        d6 = d6 * (double)int2 / 65536.0;
        d7 = d7 - (double)(ll1 & 0xFFFF) / 32768.0;
        d8 = d8 + (double)i / 1024.0;
        
        /* Conditional with many live values */
        if (i % 4 == 0) {
            global_array[i % 1024] = (int)(d1 * 1000.0);
        } else if (i % 4 == 1) {
            global_char[i % 2048] = (char)(d2 * 256.0);
        } else if (i % 4 == 2) {
            global_double[i % 512] = d3 + d4;
        }
    }
    
    /* Final mix of all types */
    return (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8) + r3 + (int)(ll1 >> 32);
}

/* Main function to drive everything */
int main(int argc, char **argv) {
    /* Initialize global arrays */
    for (int i = 0; i < 1024; i++) {
        global_array[i] = i * 3 + 7;
    }
    for (int i = 0; i < 512; i++) {
        global_double[i] = (double)i / 512.0;
    }
    for (int i = 0; i < 2048; i++) {
        global_char[i] = (char)(i & 0xFF);
    }
    
    /* Local array for outputs */
    int output[256];
    double fp_data[100];
    
    /* Call all test functions with appropriate arguments */
    int result1 = func_loop_invariants(1000, output);
    int result2 = func_asm_clobber(0x11111111, 0x22222222, 0x33333333);
    int result3 = func_complex_control(0x44444444, output);
    int result4 = func_mixed_patterns(fp_data, 100);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + result3 + result4;
    
    /* Use results to affect control flow */
    if (final_result > 0) {
        for (int i = 0; i < 256; i++) {
            output[i] += final_result;
        }
    }
    
    /* Return final result */
    return final_result & 0xFF;
}
