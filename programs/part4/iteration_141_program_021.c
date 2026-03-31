/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that will clobber registers */
#ifdef __GNUC__
__attribute__((noinline, noclone))
#else
__declspec(noinline)
#endif
void external_func1(volatile int* p) {
    *p += 1;
    /* Clobber call-clobbered registers via inline asm */
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

#ifdef __GNUC__
__attribute__((noinline, noclone))
#else
__declspec(noinline)
#endif
void external_func2(volatile long* p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(volatile void*);

/* Force register allocation with explicit register variables */
#ifdef __x86_64__
#define REG_R10 register long r10 asm("r10")
#define REG_R11 register long r11 asm("r11") 
#define REG_R9  register long r9  asm("r9")
#else
/* For 32-bit or other architectures */
#define REG_R10 register long r10
#define REG_R11 register long r11
#define REG_R9  register long r9
#endif

/* Main test function with complex register usage */
long test_caller_save_insertion(int* data, int size, func_ptr_t fp1, func_ptr_t fp2) {
    volatile int barrier = 0;
    long total = 0;
    
    /* Use explicit register variables to create pressure */
    REG_R10 = data[0] + data[1];
    REG_R11 = data[2] * data[3];
    REG_R9  = data[4] - data[5];
    
    /* Nested loops with function calls */
    for (int i = 0; i < size; i += 8) {
        /* Inner loop with register-intensive calculations */
        for (int j = 0; j < 8 && (i + j) < size; j++) {
            /* Live values in registers across calls */
            int idx = i + j;
            
            /* Force values into call-clobbered registers */
            register long rax_val asm("rax") = data[idx];
            register long rcx_val asm("rcx") = data[(idx + 1) % size];
            register long rdx_val asm("rdx") = data[(idx + 2) % size];
            
            /* Perform arithmetic keeping results live */
            rax_val = rax_val * 3 + rcx_val;
            rcx_val = rdx_val * 7 - rax_val;
            rdx_val = rax_val ^ rcx_val;
            
            /* Artificial dependency to prevent optimization */
            asm volatile("" : "+r"(rax_val), "+r"(rcx_val), "+r"(rdx_val));
            
            /* Mix of direct and indirect calls */
            if (idx % 3 == 0) {
                external_func1(&barrier);
            } else if (idx % 3 == 1) {
                fp1(&barrier);
            } else {
                fp2(&barrier);
            }
            
            /* Use the live values after call - forces save/restore */
            REG_R10 += rax_val;
            REG_R11 ^= rcx_val;
            REG_R9  |= rdx_val;
            
            /* More arithmetic with register variables */
            rax_val = REG_R10 * 5;
            rcx_val = REG_R11 + 11;
            rdx_val = REG_R9 - 13;
            
            /* Another call with different register pressure */
            external_func2(&barrier);
            
            /* Final computation and store back */
            data[idx] = (int)(rax_val + rcx_val + rdx_val);
            
            /* Update total with all register values */
            total += REG_R10 + REG_R11 + REG_R9 + rax_val + rcx_val + rdx_val;
            
            /* Rotate register values to create complex live ranges */
            long tmp = REG_R10;
            REG_R10 = REG_R11;
            REG_R11 = REG_R9;
            REG_R9 = tmp;
        }
        
        /* Additional function call in outer loop */
        if (i % 16 == 0) {
            external_func1(&barrier);
        }
    }
    
    /* Final aggregation using all register variables */
    total = total + REG_R10 * 2 + REG_R11 * 3 + REG_R9 * 5;
    
    /* Force compiler to keep register values live */
    asm volatile("" : : "r"(REG_R10), "r"(REG_R11), "r"(REG_R9));
    
    return total;
}

/* Alternate test with different pattern */
long test_caller_save_pattern2(int* data, int size) {
    volatile long accum = 0;
    
    for (int iter = 0; iter < 3; iter++) {
        /* Different register usage pattern each iteration */
        REG_R10 = iter * 1000;
        REG_R11 = iter * 2000;
        REG_R9  = iter * 3000;
        
        for (int i = 0; i < size; i++) {
            /* Complex expression spanning multiple calls */
            int val = data[i];
            
            /* Multiple calls in sequence with live values */
            external_func1((volatile int*)&val);
            
            register long r8_val asm("r8") = val * REG_R10;
            register long rsi_val asm("rsi") = val + REG_R11;
            register long rdi_val asm("rdi") = val ^ REG_R9;
            
            external_func2((volatile long*)&val);
            
            /* Interleaved computation and calls */
            r8_val = r8_val + rsi_val;
            external_func1((volatile int*)&val);
            rsi_val = rsi_val - rdi_val;
            external_func2((volatile long*)&val);
            rdi_val = rdi_val * r8_val;
            
            /* Store results forcing register saves */
            data[i] = (int)(r8_val + rsi_val + rdi_val);
            accum += data[i];
            
            /* Update register variables */
            REG_R10 += r8_val;
            REG_R11 ^= rsi_val;
            REG_R9  |= rdi_val;
        }
    }
    
    return accum;
}

int main() {
    const int SIZE = 256;
    int* data = (int*)malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    
    /* Create function pointers to inhibit optimizations */
    func_ptr_t fp1 = (func_ptr_t)external_func1;
    func_ptr_t fp2 = (func_ptr_t)external_func2;
    
    /* Run both test patterns */
    long result1 = test_caller_save_insertion(data, SIZE, fp1, fp2);
    long result2 = test_caller_save_pattern2(data, SIZE);
    
    /* Final result to prevent dead code elimination */
    long final_result = result1 + result2;
    
    /* Use the result */
    printf("Result: %ld\n", final_result);
    
    /* Verify some data was modified */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= data[i];
    }
    printf("Checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
