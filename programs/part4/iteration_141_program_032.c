/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* External functions that will clobber registers */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func2(long *p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func3(unsigned long *p) {
    *p ^= 0xAAAAAAAA;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(void*);

/* Main test function with extensive register pressure */
long __attribute__((noinline)) test_caller_save(int *data, int size, func_ptr_t fp) {
    /* Use explicit register variables to pressure specific call-clobbered registers */
    register long r10_val asm("r10") = 0;
    register long r11_val asm("r11") = 0;
    register long r8_val asm("r8") = 0;
    register long r9_val asm("r9") = 0;
    
    /* Regular variables that will use call-saved registers */
    long sum1 = 0, sum2 = 0, sum3 = 0;
    volatile long vol_var = 0; /* Prevent optimizations */
    
    /* Nested loops to create complex live ranges */
    for (int i = 0; i < size; i++) {
        /* Load values into call-clobbered registers */
        r10_val = data[i] * 3L;
        r11_val = data[i] + 7L;
        
        /* Use inline assembly to create dependencies */
        asm volatile("" : "+r"(r10_val), "+r"(r11_val));
        
        for (int j = 0; j < 3; j++) {
            /* More register pressure in inner loop */
            r8_val = r10_val * r11_val + j;
            r9_val = r10_val ^ r11_val ^ j;
            
            /* Mix with call-saved register usage */
            sum1 += r8_val;
            sum2 += r9_val;
            
            /* Volatile access to prevent reordering */
            vol_var = r8_val + r9_val;
            
            /* Function call that clobbers registers */
            if (fp) {
                long temp = r8_val + r9_val + sum1;
                fp(&temp);
                sum3 += temp;
            }
            
            /* Use values after call - forces save/restore */
            r10_val = r8_val * 2 + vol_var;
            r11_val = r9_val / 2 + vol_var;
            
            /* Another inline assembly barrier */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val));
        }
        
        /* Store results back, keeping values live */
        data[i] = (int)(r10_val + r11_val + sum1 + sum2 + sum3);
    }
    
    return sum1 + sum2 + sum3 + r10_val + r11_val + r8_val + r9_val;
}

/* Another test with indirect calls */
long __attribute__((noinline)) test_indirect_calls(int *data, int size) {
    long result = 0;
    
    /* Array of function pointers */
    func_ptr_t funcs[] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3,
        NULL
    };
    
    /* Use multiple call-clobbered registers */
    register long rax_val asm("rax") = 0x12345678;
    register long rcx_val asm("rcx") = 0x87654321;
    register long rdx_val asm("rdx") = 0xABCDEF01;
    
    for (int i = 0; i < size; i += 4) {
        /* Complex computation spreading across many registers */
        rax_val = data[i] * 0x5A5A5A5A;
        rcx_val = data[i+1] ^ 0xA5A5A5A5;
        rdx_val = data[i+2] + 0x33333333;
        
        /* Mix with call-saved registers */
        long saved1 = rax_val * 2;
        long saved2 = rcx_val / 3;
        long saved3 = rdx_val ^ 0xFFFFFFFF;
        
        /* Call different functions via pointer */
        for (int f = 0; f < 3 && funcs[f]; f++) {
            /* Values must survive across calls */
            long temp1 = rax_val + saved1;
            long temp2 = rcx_val + saved2;
            long temp3 = rdx_val + saved3;
            
            /* Inline assembly to prevent optimization */
            asm volatile("" : "+r"(temp1), "+r"(temp2), "+r"(temp3));
            
            /* Indirect call */
            funcs[f](&temp1);
            
            /* Use values after call */
            rax_val = temp1 * temp2;
            rcx_val = temp2 ^ temp3;
            rdx_val = temp3 + temp1;
            
            /* Force dependency chain */
            asm volatile("" : "+r"(rax_val), "+r"(rcx_val), "+r"(rdx_val));
        }
        
        /* Store results using all live values */
        data[i] = (int)(rax_val);
        data[i+1] = (int)(rcx_val);
        data[i+2] = (int)(rdx_val);
        data[i+3] = (int)(saved1 + saved2 + saved3);
        
        result += rax_val + rcx_val + rdx_val + saved1 + saved2 + saved3;
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
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 3 + 7;
    }
    
    printf("Testing caller-save insertion...\n");
    
    /* Test 1: Direct calls with register pressure */
    long result1 = test_caller_save(data, SIZE, (func_ptr_t)external_func1);
    printf("Result 1: %ld\n", result1);
    
    /* Re-initialize */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 5 + 11;
    }
    
    /* Test 2: Indirect calls */
    long result2 = test_indirect_calls(data, SIZE);
    printf("Result 2: %ld\n", result2);
    
    /* Test 3: Mixed pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i;
    }
    
    /* Alternate between different external functions */
    for (int iter = 0; iter < 10; iter++) {
        func_ptr_t fp;
        switch (iter % 3) {
            case 0: fp = (func_ptr_t)external_func1; break;
            case 1: fp = (func_ptr_t)external_func2; break;
            case 2: fp = (func_ptr_t)external_func3; break;
        }
        test_caller_save(data + (iter * 8) % SIZE, 16, fp);
    }
    
    /* Final checksum */
    long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += data[i];
    }
    printf("Final checksum: %ld\n", checksum);
    
    free(data);
    return 0;
}
