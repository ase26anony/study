/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that clobber registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#else
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi"
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

/* Main test function with intensive register usage */
__attribute__((noinline))
int test_caller_save(int *data, int size, func_ptr_t fp) {
    /* Use explicit register variables to pressure specific registers */
    #ifdef __x86_64__
    register int64_t r10_val asm("r10") = 0;
    register int64_t r11_val asm("r11") = 0;
    register int64_t r8_val asm("r8") = 0;
    register int64_t r9_val asm("r9") = 0;
    #else
    register int32_t eax_val asm("eax") = 0;
    register int32_t ecx_val asm("ecx") = 0;
    register int32_t edx_val asm("edx") = 0;
    #endif
    
    volatile int sum = 0;  /* Prevent optimization */
    int i, j;
    
    /* Nested loops to create complex live ranges */
    for (i = 0; i < size; i++) {
        /* Load values into registers - these must survive function calls */
        int val1 = data[i];
        int val2 = data[(i + 1) % size];
        int val3 = data[(i + 2) % size];
        
        /* Force values into call-clobbered registers with inline asm */
        asm volatile("" : "+r"(val1), "+r"(val2), "+r"(val3));
        
        /* Complex computation keeping values live */
        for (j = 0; j < 3; j++) {
            /* Mix of operations in registers */
            int temp = val1 + val2;
            val1 = val2 ^ val3;
            val2 = val3 * temp;
            val3 = temp - val1;
            
            /* Function call that clobbers registers */
            fp(&sum);
            
            /* Continue using the live values after call */
            val1 += (sum % 7);
            val2 -= (sum % 5);
            val3 *= (sum % 3) + 1;
            
            /* More register pressure with explicit register vars */
            #ifdef __x86_64__
            r10_val = val1 * val2;
            r11_val = val2 * val3;
            r8_val = r10_val + r11_val;
            r9_val = r8_val - val3;
            
            /* Use the values to prevent dead code elimination */
            asm volatile("" : : "r"(r10_val), "r"(r11_val), "r"(r8_val), "r"(r9_val));
            
            /* Another function call */
            if (j % 2) {
                external_func2(&sum);
            } else {
                external_func3(&sum);
            }
            
            /* Continue using register values */
            val1 = r9_val % 256;
            val2 = r8_val % 128;
            #else
            eax_val = val1 * val2;
            ecx_val = val2 * val3;
            edx_val = eax_val + ecx_val;
            
            asm volatile("" : : "r"(eax_val), "r"(ecx_val), "r"(edx_val));
            
            if (j % 2) {
                external_func2(&sum);
            } else {
                external_func3(&sum);
            }
            
            val1 = edx_val % 256;
            val2 = ecx_val % 128;
            #endif
        }
        
        /* Store results back */
        data[i] = val1 + val2 + val3;
    }
    
    return sum;
}

/* Another test with indirect calls */
__attribute__((noinline))
int test_indirect_calls(int *data, int size) {
    func_ptr_t funcs[3] = {external_func1, external_func2, external_func3};
    volatile int accumulator = 0;
    
    /* Use many local variables to increase register pressure */
    int a = data[0], b = data[1], c = data[2], d = data[3];
    int e = data[4], f = data[5], g = data[6], h = data[7];
    
    for (int i = 0; i < size * 2; i++) {
        /* Rotate values through registers */
        int t = a; a = b; b = c; c = d; d = e; e = f; f = g; g = h; h = t;
        
        /* Complex expression with many intermediate values */
        int x = (a * b) + (c * d) - (e * f) + (g * h);
        int y = (a ^ b) | (c ^ d) & (e ^ f) | (g ^ h);
        int z = x * y - (a + b + c + d + e + f + g + h);
        
        /* Force values to stay in registers */
        asm volatile("" : "+r"(x), "+r"(y), "+r"(z));
        
        /* Indirect call - compiler doesn't know which registers are safe */
        funcs[i % 3](&accumulator);
        
        /* Use values after call - they must be saved/restored */
        x += accumulator;
        y -= accumulator;
        z ^= accumulator;
        
        /* Another call */
        if (x > y) {
            external_func1(&accumulator);
        } else {
            external_func2(&accumulator);
        }
        
        /* More computation with live values */
        a = x % 31;
        b = y % 47;
        c = z % 73;
        d = (x + y + z) % 101;
        
        /* Store periodically to create memory pressure */
        if (i % 8 == 0) {
            data[i % size] = a + b + c + d;
        }
    }
    
    return accumulator;
}

int main() {
    const int SIZE = 256;
    int *data = (int*)malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 3 + 7;
    }
    
    /* Test 1: Direct function pointer call */
    printf("Test 1 starting...\n");
    int result1 = test_caller_save(data, SIZE, external_func1);
    printf("Test 1 result: %d\n", result1);
    
    /* Re-initialize */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 5 + 11;
    }
    
    /* Test 2: Indirect calls */
    printf("Test 2 starting...\n");
    int result2 = test_indirect_calls(data, SIZE);
    printf("Test 2 result: %d\n", result2);
    
    /* Final computation using results */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += data[i];
    }
    
    printf("Final array sum: %d\n", final_sum);
    printf("Total: %d\n", result1 + result2 + final_sum);
    
    free(data);
    return 0;
}
