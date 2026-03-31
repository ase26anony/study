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
    /* Use explicit register variables to create pressure */
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
    
    volatile int sum = 0;  /* Prevent optimization */
    int i, j;
    
    /* Nested loops with calls to create complex live ranges */
    for (i = 0; i < n; i++) {
        /* Mix of operations in call-clobbered registers */
        for (j = 0; j < 3; j++) {
            /* Live values in call-clobbered registers before call */
#ifdef __x86_64__
            r10_val = r10_val * 3 + data[i + j];
            r11_val = r11_val / 2 + data[i + j + 1];
            r8_val = r8_val ^ data[i + j + 2];
            r9_val = r9_val | data[i + j + 3];
            
            /* Force these values to be live across call */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), 
                               "+r"(r8_val), "+r"(r9_val));
#else
            eax_val = eax_val * 3 + data[i + j];
            ecx_val = ecx_val / 2 + data[i + j + 1];
            edx_val = edx_val ^ data[i + j + 2];
            
            asm volatile("" : "+r"(eax_val), "+r"(ecx_val), "+r"(edx_val));
#endif
            
            /* Function call that clobbers registers */
            fp(&data[i]);
            
            /* Use the live values after call */
#ifdef __x86_64__
            sum += (int)(r10_val + r11_val + r8_val + r9_val);
#else
            sum += (int)(eax_val + ecx_val + edx_val);
#endif
            
            /* More operations to keep values live */
#ifdef __x86_64__
            r10_val = r10_val + sum;
            r11_val = r11_val - sum;
            r8_val = r8_val * 2;
            r9_val = r9_val / 2;
#else
            eax_val = eax_val + sum;
            ecx_val = ecx_val - sum;
            edx_val = edx_val * 2;
#endif
        }
        
        /* Another call with different pattern */
        if (i % 2 == 0) {
            external_func1(&sum);
        } else {
            external_func2(&sum);
        }
        
        /* Indirect call via function pointer */
        func_ptr_t fp2 = (i % 3 == 0) ? external_func1 : 
                        (i % 3 == 1) ? external_func2 : external_func3;
        fp2(&sum);
    }
    
    /* Final computation using register values */
#ifdef __x86_64__
    return (int)(r10_val + r11_val + r8_val + r9_val + sum);
#else
    return (int)(eax_val + ecx_val + edx_val + sum);
#endif
}

int main() {
    const int SIZE = 256;
    int *data = malloc(SIZE * sizeof(int));
    int i;
    
    /* Initialize data */
    for (i = 0; i < SIZE; i++) {
        data[i] = i + 1;
    }
    
    /* Array of function pointers */
    func_ptr_t funcs[] = {external_func1, external_func2, external_func3};
    
    /* Run test multiple times with different function pointers */
    int total = 0;
    for (i = 0; i < 10; i++) {
        total += test_caller_save(data, 50, funcs[i % 3]);
        
        /* Modify data to change patterns */
        data[i * 5] = total % 100;
    }
    
    printf("Result: %d\n", total);
    
    /* Verify computation */
    int check = 0;
    for (i = 0; i < SIZE; i++) {
        check += data[i];
    }
    printf("Data sum: %d\n", check);
    
    free(data);
    return 0;
}
