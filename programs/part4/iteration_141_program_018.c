/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#else
#define CLOBBER_LIST "eax", "ecx", "edx"
#endif

/* Force noinline to ensure actual calls */
__attribute__((noinline)) void external_func1(int *p) {
    *p += 1;
    /* Clobber call-clobbered registers via asm */
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func2(long *p) {
    *p *= 2;
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func3(unsigned *p) {
    *p ^= 0xAAAA;
    asm volatile("" : : : CLOBBER_LIST);
}

/* Complex calculation forcing register usage */
__attribute__((noinline, optimize("O0"))) 
int complex_calc(int a, int b, int c, int d, int e) {
    /* Prevent optimization */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    volatile int v5 = e;
    
    /* Force register usage with inline asm */
    int r1, r2, r3, r4, r5;
    asm volatile("" : "=r"(r1) : "0"(v1));
    asm volatile("" : "=r"(r2) : "0"(v2));
    asm volatile("" : "=r"(r3) : "0"(v3));
    asm volatile("" : "=r"(r4) : "0"(v4));
    asm volatile("" : "=r"(r5) : "0"(v5));
    
    return r1 + r2 * r3 - r4 / (r5 + 1);
}

int main() {
    /* Large array to create register pressure */
    int data[256];
    long accum[8] = {0};
    
    /* Initialize with pattern */
    for (int i = 0; i < 256; i++) {
        data[i] = i * 3 + 7;
    }
    
    /* Function pointer to force indirect calls */
    void (*func_ptr)(void*);
    int use_func1 = 1;
    
    /* Nested loops with calls - creates complex live ranges */
    for (int outer = 0; outer < 10; outer++) {
        /* Use explicit register variables to pressure specific registers */
        register long r10 asm("r10") = outer * 100;
        register long r11 asm("r11") = outer * 200;
        
        for (int inner = 0; inner < 32; inner++) {
            /* Multiple live values in registers before call */
            int idx = (outer * 32 + inner) % 256;
            
            /* Force values into call-clobbered registers */
            int val1 = data[idx];
            int val2 = data[(idx + 1) % 256];
            int val3 = data[(idx + 2) % 256];
            int val4 = data[(idx + 3) % 256];
            int val5 = data[(idx + 4) % 256];
            
            /* Complex calculation keeping values live */
            int calc_result = complex_calc(val1, val2, val3, val4, val5);
            
            /* Mix of call-clobbered and call-saved register usage */
            long temp1 = r10 + calc_result;
            long temp2 = r11 * calc_result;
            
            /* Volatile to prevent reordering */
            volatile long save1 = temp1;
            volatile long save2 = temp2;
            
            /* Switch function pointer to inhibit optimizations */
            if (use_func1) {
                func_ptr = (void(*)(void*))external_func1;
                use_func1 = 0;
            } else {
                func_ptr = (void(*)(void*))external_func2;
                use_func1 = 1;
            }
            
            /* Call via function pointer - inhibits optimizations */
            func_ptr(&save1);
            
            /* Use values after call - forces save/restore */
            accum[0] += save1;
            accum[1] += save2;
            
            /* More calculations with live values across another call */
            int val6 = data[(idx + 5) % 256];
            int val7 = data[(idx + 6) % 256];
            
            /* Force register usage with inline asm */
            register int reg_val asm("eax") = val6;
            register int reg_val2 asm("ecx") = val7;
            
            asm volatile("" : "+r"(reg_val), "+r"(reg_val2));
            
            /* Another call with different register pressure */
            external_func3(&data[idx]);
            
            /* Use register values after call */
            accum[2] += reg_val;
            accum[3] += reg_val2;
            
            /* Update register variables */
            r10 += val1;
            r11 -= val2;
            
            /* Force spill/reload with volatile */
            volatile int barrier = idx;
            asm volatile("" : : "r"(barrier));
        }
        
        /* Periodic call with many live values */
        if (outer % 3 == 0) {
            /* Create many live values */
            long sum = 0;
            for (int i = 0; i < 8; i++) {
                sum += accum[i];
            }
            
            /* Call with value in register */
            external_func2(&sum);
            
            /* Use result */
            accum[7] = sum;
        }
    }
    
    /* Final calculation and output */
    long final_result = 0;
    for (int i = 0; i < 8; i++) {
        final_result += accum[i];
    }
    
    printf("Result: %ld\n", final_result);
    
    /* Verify with simple calculation */
    long verify = 0;
    for (int i = 0; i < 256; i++) {
        verify += data[i];
    }
    printf("Data sum: %ld\n", verify);
    
    return 0;
}
