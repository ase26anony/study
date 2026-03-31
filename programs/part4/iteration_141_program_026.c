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

/* Complex function with register pressure */
__attribute__((noinline, optimize("O0")))
int compute_with_saves(int *data, int size, void (*callback)(int*)) {
    volatile int keep_alive = 0;  /* Prevent optimizations */
    
    /* Use explicit register variables to force specific register usage */
#ifdef __x86_64__
    register int64_t r10_val asm("r10") = data[0];
    register int64_t r11_val asm("r11") = data[1];
    register int64_t rcx_val asm("rcx") = data[2];
    register int64_t rdx_val asm("rdx") = data[3];
#else
    register int32_t eax_val asm("eax") = data[0];
    register int32_t ecx_val asm("ecx") = data[1];
    register int32_t edx_val asm("edx") = data[2];
#endif
    
    int sum = 0;
    
    /* Nested loops with calls - creates complex live ranges */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 3; j++) {
            /* Mix of operations keeping values live in call-clobbered regs */
#ifdef __x86_64__
            r10_val = r10_val * 3 + i;
            r11_val = r11_val ^ r10_val;
            rcx_val = rcx_val + r11_val * j;
            rdx_val = rdx_val - rcx_val;
            
            /* Force values to be used in inline asm to keep them live */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(rcx_val), "+r"(rdx_val));
            
            /* Call external function - values in r10, r11, rcx, rdx must be saved */
            callback(&data[i]);
            
            /* Continue using the register values after call */
            sum += r10_val + r11_val + rcx_val + rdx_val;
            keep_alive = sum;  /* Volatile use */
#else
            eax_val = eax_val * 3 + i;
            ecx_val = ecx_val ^ eax_val;
            edx_val = edx_val + ecx_val * j;
            
            asm volatile("" : "+r"(eax_val), "+r"(ecx_val), "+r"(edx_val));
            
            callback(&data[i]);
            
            sum += eax_val + ecx_val + edx_val;
            keep_alive = sum;
#endif
            
            /* Additional arithmetic to create more register pressure */
            for (int k = 0; k < 2; k++) {
                int temp = data[i] * k;
                sum += temp;
                asm volatile("" : "+r"(temp));
            }
        }
    }
    
    return sum + keep_alive;
}

/* Function with indirect calls */
__attribute__((noinline))
int test_indirect_calls(int *data, int size) {
    void (*func_array[3])(int*) = {external_func1, external_func2, external_func3};
    int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Use different function pointers to inhibit optimizations */
        void (*fp)(int*) = func_array[i % 3];
        
        /* Create live values in registers before indirect call */
        register int val1 asm("eax") = data[i];
        register int val2 asm("ecx") = data[i + 1];
        register int val3 asm("edx") = data[i + 2];
        
        /* Complex computation keeping values live */
        for (int j = 0; j < 4; j++) {
            val1 = val1 * 1103515245 + 12345;
            val2 = val2 ^ val1;
            val3 = val3 + val2 * j;
            
            /* Use inline asm to prevent reordering/elimination */
            asm volatile("" : "+r"(val1), "+r"(val2), "+r"(val3));
            
            /* Indirect call - must save call-clobbered registers */
            fp(&data[i]);
            
            /* Use values after call */
            result += val1 + val2 + val3;
            
            /* Force spill/reload with volatile */
            volatile int temp = result;
            result = temp;
        }
    }
    
    return result;
}

/* Main test driver */
int main() {
    const int SIZE = 256;
    int *data = malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 3 + 7;
    }
    
    printf("Testing caller-save insertion patterns...\n");
    
    /* Test 1: Direct calls with register pressure */
    int result1 = compute_with_saves(data, 64, external_func1);
    printf("Result 1: %d\n", result1);
    
    /* Test 2: Indirect calls */
    int result2 = test_indirect_calls(data, 32);
    printf("Result 2: %d\n", result2);
    
    /* Test 3: Mixed pattern */
    for (int iter = 0; iter < 10; iter++) {
        for (int i = 0; i < 32; i++) {
            /* Create many live values */
            int a = data[i];
            int b = data[i + 32];
            int c = data[i + 64];
            int d = data[i + 96];
            
            /* Force into specific registers */
            register int ra asm("eax") = a;
            register int rb asm("ecx") = b;
            register int rc asm("edx") = c;
            register int rd asm("esi") = d;
            
            /* Use them */
            asm volatile("" : "+r"(ra), "+r"(rb), "+r"(rc), "+r"(rd));
            
            /* Call sequence */
            if (iter % 2 == 0) {
                external_func1(&data[i]);
            } else {
                external_func2(&data[i]);
            }
            
            /* Continue using values */
            data[i] = ra + rb + rc + rd;
            
            /* Another call */
            external_func3(&data[i]);
        }
    }
    
    /* Final computation */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += data[i];
    }
    
    printf("Final sum: %d\n", final_sum);
    
    free(data);
    return 0;
}
