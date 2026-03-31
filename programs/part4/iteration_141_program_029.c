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
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func2(int *p) {
    *p *= 2;
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func3(int *p) {
    *p -= 3;
    asm volatile("" : : : CLOBBER_LIST);
}

/* Complex calculation that uses many registers */
__attribute__((noinline)) int complex_calc(int a, int b, int c, int d, int e, int f) {
    /* Use volatile to prevent optimization */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    volatile int v5 = e;
    volatile int v6 = f;
    
    /* Force register usage with inline asm */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3), "+r"(v4), "+r"(v5), "+r"(v6));
    
    return v1 + v2 * v3 - v4 / (v5 + 1) + v6;
}

int main(void) {
    int data[256];
    int sum = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        data[i] = i * 3 + 1;
    }
    
    /* Function pointer array to force indirect calls */
    void (*func_array[3])(int *) = {
        external_func1,
        external_func2,
        external_func3
    };
    
    /* Use explicit register variables to create pressure */
#ifdef __x86_64__
    register long r10 asm("r10");
    register long r11 asm("r11");
    register long r12 asm("r12");  /* Call-saved register */
    register long r13 asm("r13");  /* Call-saved register */
    r10 = 100;
    r11 = 200;
    r12 = 300;  /* Will need to be preserved across calls */
    r13 = 400;
#endif
    
    /* Nested loops with calls - creates complex live ranges */
    for (int outer = 0; outer < 10; outer++) {
        int temp_sum = 0;
        
        /* Inner loop with register-intensive calculations and calls */
        for (int i = 0; i < 100; i++) {
            /* Load values into registers that will be live across calls */
            int idx1 = (i * 7) % 256;
            int idx2 = (i * 13) % 256;
            int idx3 = (i * 19) % 256;
            
            /* Force values into registers before call */
            int val1 = data[idx1] + outer;
            int val2 = data[idx2] * 2;
            int val3 = data[idx3] - outer;
            
            /* Use inline asm to prevent reordering/optimization */
            asm volatile("" : "+r"(val1), "+r"(val2), "+r"(val3));
            
            /* Complex calculation that uses many registers */
            int complex_result = complex_calc(val1, val2, val3, 
                                            data[(i * 5) % 256],
                                            data[(i * 11) % 256],
                                            data[(i * 17) % 256]);
            
            /* Call external function - forces caller-save for live registers */
            func_array[i % 3](&complex_result);
            
            /* Use the result after call - requires restored values */
            temp_sum += complex_result;
            
            /* More register operations to create pressure */
            val1 = (val1 * 3) / (val2 + 1);
            val2 = (val2 + val3) * 2;
            val3 = val1 ^ val2;
            
            /* Another call with different live values */
            if (i % 5 == 0) {
                external_func2(&val3);
            }
            
            /* Store results back - creates more register pressure */
            data[idx1] = val1;
            data[idx2] = val2;
            data[idx3] = val3;
            
            /* Use explicit register variables in calculations */
#ifdef __x86_64__
            r10 = r10 + val1;
            r11 = r11 ^ val2;
            /* Call-saved registers used across calls */
            r12 = r12 + complex_result;
            r13 = r13 - val3;
            
            /* Force spill/reload of call-saved registers */
            if (i % 7 == 0) {
                external_func3((int *)&r12);
            }
#endif
        }
        
        sum += temp_sum;
        
        /* Additional calls with register pressure */
        for (int j = 0; j < 5; j++) {
            int tmp = data[j * 10] + sum;
            external_func1(&tmp);
            sum += tmp;
            
            /* Mix of operations to keep registers busy */
            for (int k = 0; k < 3; k++) {
                int a = data[j * 20 + k];
                int b = data[j * 20 + k + 1];
                int c = a * b + k;
                external_func2(&c);
                data[j * 20 + k] = c;
            }
        }
    }
    
#ifdef __x86_64__
    /* Use the register variables at the end */
    sum += (int)(r10 + r11 + r12 + r13);
#endif
    
    printf("Result: %d\n", sum);
    
    /* Verify computation */
    int verify = 0;
    for (int i = 0; i < 256; i++) {
        verify += data[i];
    }
    printf("Array sum: %d\n", verify);
    
    return 0;
}
