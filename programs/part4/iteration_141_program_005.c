/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#else
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi"
#endif

/* Force noinline to ensure actual calls */
__attribute__((noinline)) void external_func1(volatile int* p) {
    *p += 1;
    /* Clobber call-clobbered registers */
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func2(volatile int* p) {
    *p *= 2;
    asm volatile("" : : : CLOBBER_LIST);
}

__attribute__((noinline)) void external_func3(volatile int* p) {
    *p -= 3;
    asm volatile("" : : : CLOBBER_LIST);
}

/* Function pointer type */
typedef void (*func_ptr_t)(volatile int*);

/* Complex calculation using many registers */
__attribute__((noinline, optimize("O2")))
int compute_with_saves(int* arr, int n, func_ptr_t fp) {
    volatile int barrier = 0;
    
    /* Force values into specific registers using explicit register variables */
#ifdef __x86_64__
    register int64_t r10_val asm("r10") = arr[0];
    register int64_t r11_val asm("r11") = arr[1];
    register int64_t r9_val asm("r9") = arr[2];
    register int64_t r8_val asm("r8") = arr[3];
#else
    register int32_t eax_val asm("eax") = arr[0];
    register int32_t ecx_val asm("ecx") = arr[1];
    register int32_t edx_val asm("edx") = arr[2];
    register int32_t esi_val asm("esi") = arr[3];
#endif
    
    int sum = 0;
    
    /* Nested loops with calls to create complex live ranges */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 3; j++) {
            /* Mix of call-clobbered and call-saved register usage */
            int temp1 = arr[i] * (i + 1);
            int temp2 = arr[(i + j) % n] * (j + 1);
            
            /* Use volatile to prevent optimization */
            volatile int vol1 = temp1;
            volatile int vol2 = temp2;
            
            /* Force register pressure with inline assembly */
            asm volatile("" : "+r"(temp1), "+r"(temp2));
            
            /* Live values in registers before call */
#ifdef __x86_64__
            r10_val += temp1;
            r11_val += temp2;
            r9_val ^= temp1;
            r8_val ^= temp2;
            
            /* Make values live across call */
            asm volatile("" : : "r"(r10_val), "r"(r11_val), "r"(r9_val), "r"(r8_val));
#else
            eax_val += temp1;
            ecx_val += temp2;
            edx_val ^= temp1;
            esi_val ^= temp2;
            
            asm volatile("" : : "r"(eax_val), "r"(ecx_val), "r"(edx_val), "r"(esi_val));
#endif
            
            /* Function call that clobbers registers */
            fp(&barrier);
            
            /* Use values after call - forces save/restore */
#ifdef __x86_64__
            temp1 = (int)(r10_val ^ r11_val);
            temp2 = (int)(r9_val ^ r8_val);
#else
            temp1 = (int)(eax_val ^ ecx_val);
            temp2 = (int)(edx_val ^ esi_val);
#endif
            
            /* More operations to keep values live */
            sum += temp1 + temp2 + vol1 + vol2;
            
            /* Another volatile operation */
            asm volatile("" : "+r"(sum));
        }
        
        /* Alternate between different function pointers */
        if (i % 2 == 0) {
            external_func1(&barrier);
        } else {
            external_func2(&barrier);
        }
        
        /* Update array with computed values */
        arr[i] = (sum + i) & 0xFF;
    }
    
    /* Final external call */
    external_func3(&barrier);
    
    return sum + barrier;
}

/* Main function with varying patterns */
int main() {
    const int SIZE = 256;
    int* data = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 37 + 123) & 0xFF;
    }
    
    /* Array of function pointers */
    func_ptr_t funcs[] = {external_func1, external_func2, external_func3};
    
    int total_sum = 0;
    
    /* Multiple iterations with different function pointers */
    for (int iter = 0; iter < 10; iter++) {
        func_ptr_t fp = funcs[iter % 3];
        
        /* Vary the size parameter to create different patterns */
        int n = SIZE - iter * 5;
        if (n < 50) n = 50;
        
        /* Call computation with many live values across calls */
        int result = compute_with_saves(data, n, fp);
        total_sum += result;
        
        /* Modify data for next iteration */
        for (int i = 0; i < n; i++) {
            data[i] = (data[i] + result + i) & 0xFF;
        }
    }
    
    /* Use result to prevent optimization */
    volatile int final_result = total_sum;
    printf("Result: %d\n", final_result);
    
    /* Verify data isn't corrupted */
    int check = 0;
    for (int i = 0; i < SIZE; i++) {
        check ^= data[i];
    }
    printf("Check: %d\n", check);
    
    free(data);
    return 0;
}
