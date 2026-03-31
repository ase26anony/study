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

/* Function pointer type */
typedef void (*func_ptr_t)(int*);

/* Main test function with register pressure */
__attribute__((noinline))
int test_caller_save(int *data, int n, func_ptr_t fp) {
    /* Use explicit register variables to pressure specific registers */
#ifdef __x86_64__
    register int64_t r10_val asm("r10") = data[0];
    register int64_t r11_val asm("r11") = data[1];
    register int64_t r8_val asm("r8") = data[2];
    register int64_t r9_val asm("r9") = data[3];
#else
    register int32_t eax_val asm("eax") = data[0];
    register int32_t ecx_val asm("ecx") = data[1];
    register int32_t edx_val asm("edx") = data[2];
#endif
    
    volatile int keep_alive = 0; /* Prevent optimizations */
    int sum = 0;
    
    /* Complex loop with multiple live values across calls */
    for (int i = 0; i < n; i++) {
        /* Create many live values in registers */
        int val1 = data[i] + i;
        int val2 = data[i + 1] * 2;
        int val3 = data[i + 2] - 3;
        int val4 = data[i + 3] ^ 0xFF;
        
        /* Mix of operations keeping values live */
#ifdef __x86_64__
        r10_val += val1;
        r11_val ^= val2;
        r8_val *= val3 + 1;
        r9_val -= val4;
        
        /* Force values to stay in registers with inline asm */
        asm volatile("" : "+r"(r10_val), "+r"(r11_val), 
                          "+r"(r8_val), "+r"(r9_val));
#else
        eax_val += val1;
        ecx_val ^= val2;
        edx_val *= val3 + 1;
        
        asm volatile("" : "+r"(eax_val), "+r"(ecx_val), "+r"(edx_val));
#endif
        
        /* Function call that clobbers registers */
        fp(&data[i]);
        
        /* Use the register values after call - forces save/restore */
#ifdef __x86_64__
        sum += r10_val + r11_val + r8_val + r9_val;
        /* More operations to increase register pressure */
        r10_val = (r10_val * 3) >> 1;
        r11_val = (r11_val + i) ^ r8_val;
        r8_val = r9_val - r10_val;
        r9_val = r11_val * r8_val;
#else
        sum += eax_val + ecx_val + edx_val;
        eax_val = (eax_val * 3) >> 1;
        ecx_val = (ecx_val + i) ^ edx_val;
        edx_val = eax_val - ecx_val;
#endif
        
        /* Another call with different pattern */
        if (i % 3 == 0) {
            external_func1(&keep_alive);
        } else if (i % 3 == 1) {
            external_func2(&keep_alive);
        } else {
            external_func3(&keep_alive);
        }
        
        /* Nested loop to create complex control flow */
        for (int j = 0; j < 3; j++) {
            int temp = data[i + j] + sum;
            /* More register operations */
#ifdef __x86_64__
            r10_val += temp;
            r11_val ^= j;
#else
            eax_val += temp;
            ecx_val ^= j;
#endif
            /* Small call in nested loop */
            if (j == 1) {
                external_func1(&temp);
            }
        }
    }
    
    /* Final computation using register values */
#ifdef __x86_64__
    return sum + r10_val + r11_val + r8_val + r9_val + keep_alive;
#else
    return sum + eax_val + ecx_val + edx_val + keep_alive;
#endif
}

/* Alternate test with indirect calls */
__attribute__((noinline))
int test_indirect_calls(int *data, int n) {
    func_ptr_t funcs[] = {external_func1, external_func2, external_func3};
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple live values */
        int a = data[i];
        int b = data[i + 1];
        int c = data[i + 2];
        int d = data[i + 3];
        
        /* Computations that should use call-clobbered registers */
        a = a * b + c;
        b = b ^ d - a;
        c = c * 3 + b;
        d = d << 2 ^ c;
        
        /* Indirect call - compiler doesn't know which exactly */
        funcs[i % 3](&data[i]);
        
        /* Use values after call */
        result += a + b + c + d;
        
        /* More operations and another call */
        a = result * 2;
        external_func2(&a);
        result += a;
    }
    
    return result;
}

int main() {
    const int SIZE = 256;
    int *data = malloc(SIZE * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 3 + 7;
    }
    
    printf("Testing caller-save insertion...\n");
    
    /* Test 1: Direct function pointer call */
    int result1 = test_caller_save(data, 100, external_func1);
    printf("Result 1: %d\n", result1);
    
    /* Test 2: Indirect calls */
    int result2 = test_indirect_calls(data, 100);
    printf("Result 2: %d\n", result2);
    
    /* Test 3: Mix of different call patterns */
    func_ptr_t fp = external_func2;
    int result3 = 0;
    for (int i = 0; i < 50; i++) {
        /* Create register pressure */
        register int r1 asm("eax") = data[i];
        register int r2 asm("ecx") = data[i + 50];
        register int r3 asm("edx") = data[i + 100];
        
        for (int j = 0; j < 5; j++) {
            r1 = r1 * r2 + r3;
            r2 = r2 ^ r1 - j;
            r3 = r3 + r2 * 2;
            
            /* Call with live register values */
            fp(&data[i + j]);
            
            /* Force save/restore */
            asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3));
            
            result3 += r1 + r2 + r3;
        }
        
        /* Switch function pointer occasionally */
        if (i % 7 == 0) fp = external_func1;
        else if (i % 7 == 3) fp = external_func3;
        else fp = external_func2;
    }
    printf("Result 3: %d\n", result3);
    
    free(data);
    return 0;
}
