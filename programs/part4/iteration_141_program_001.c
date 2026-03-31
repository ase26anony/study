/* test-caller-save.c */
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
        for (j = 0; j < 3; j++) {
            /* Mix of operations keeping values live in call-clobbered regs */
#ifdef __x86_64__
            r10_val = r10_val * 3 + data[i + j];
            r11_val = r11_val / 2 + data[i + j + 1];
            r8_val = r8_val ^ data[i + j + 2];
            r9_val = r9_val | data[i + j + 3];
            
            /* Force values to be used in asm to keep them live */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), 
                               "+r"(r8_val), "+r"(r9_val));
#else
            eax_val = eax_val * 3 + data[i + j];
            ecx_val = ecx_val / 2 + data[i + j + 1];
            edx_val = edx_val ^ data[i + j + 2];
            
            asm volatile("" : "+r"(eax_val), "+r"(ecx_val), "+r"(edx_val));
#endif
            
            /* Call through function pointer - inhibits optimizations */
            fp(&sum);
            
            /* More operations after call, using values that must survive */
#ifdef __x86_64__
            r10_val = r10_val + sum;
            r11_val = r11_val - sum;
            r8_val = r8_val * (sum + 1);
            r9_val = r9_val & ~sum;
            
            /* Store results back, creating dependencies */
            data[i] = (int)(r10_val + r11_val + r8_val + r9_val);
#else
            eax_val = eax_val + sum;
            ecx_val = ecx_val - sum;
            edx_val = edx_val * (sum + 1);
            
            data[i] = (int)(eax_val + ecx_val + edx_val);
#endif
        }
        
        /* Alternate between different external functions */
        if (i % 2 == 0) {
            external_func1(&sum);
        } else {
            external_func2(&sum);
        }
        
        /* Use volatile to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    /* Final call */
    external_func3(&sum);
    
    /* Return final result using all register values */
#ifdef __x86_64__
    return (int)(r10_val ^ r11_val ^ r8_val ^ r9_val ^ sum);
#else
    return (int)(eax_val ^ ecx_val ^ edx_val ^ sum);
#endif
}

/* Another function to create more call sites */
__attribute__((noinline))
int helper_func(int *arr, int size) {
    int result = 0;
    volatile int temp = 0;
    
    for (int i = 0; i < size; i++) {
        /* Create register pressure */
        register int val asm("eax") = arr[i];
        asm volatile("" : "+r"(val));
        
        /* Call that forces saves */
        external_func1(&temp);
        
        /* Use value after call */
        result += val * temp;
        
        /* Another call */
        external_func2(&temp);
        
        result -= arr[i] / (temp + 1);
    }
    
    return result;
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
        data[i] = (i * 37 + 123) & 0xFF;
    }
    
    /* Array of function pointers to create indirect calls */
    func_ptr_t funcs[] = {external_func1, external_func2, external_func3};
    
    int total_result = 0;
    
    /* Multiple iterations with different function pointers */
    for (int iter = 0; iter < 10; iter++) {
        func_ptr_t fp = funcs[iter % 3];
        
        /* Call test function */
        int result = test_caller_save(data, SIZE - 10, fp);
        total_result ^= result;
        
        /* Call helper to create more call sites */
        result = helper_func(data + 50, 20);
        total_result += result;
        
        /* Modify data slightly */
        for (int i = 0; i < SIZE; i++) {
            data[i] = (data[i] * 13 + 7) & 0xFFF;
        }
    }
    
    printf("Final result: %d\n", total_result);
    
    /* Verify data wasn't corrupted */
    int check = 0;
    for (int i = 0; i < SIZE; i++) {
        check ^= data[i];
    }
    printf("Data checksum: %08x\n", check);
    
    free(data);
    return 0;
}
