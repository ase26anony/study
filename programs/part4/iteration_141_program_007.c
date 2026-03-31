/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that clobber registers */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func2(long *p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func3(unsigned *p) {
    *p ^= 0xAAAA;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(void*);

/* Main test function with intensive register usage */
int __attribute__((noinline)) test_function(int *data, int size, func_ptr_t fp) {
    /* Use explicit register variables to pressure call-clobbered registers */
    register long r10 asm("r10") = data[0];
    register long r11 asm("r11") = data[1];
    register long r9 asm("r9") = data[2];
    register long r8 asm("r8") = data[3];
    
    /* Mix with regular variables that use call-saved registers */
    volatile long sum1 = 0;  /* Prevent optimization */
    volatile long sum2 = 0;
    long acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Nested loops with function calls */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            /* Complex calculations keeping values live in registers */
            r10 = (r10 * 1103515245 + 12345) & 0x7fffffff;
            r11 = (r11 * 1103515245 + 12345) & 0x7fffffff;
            r9 = (r9 * 1103515245 + 12345) & 0x7fffffff;
            r8 = (r8 * 1103515245 + 12345) & 0x7fffffff;
            
            /* Force values to be computed in call-clobbered registers */
            asm volatile("" : "+r"(r10), "+r"(r11), "+r"(r9), "+r"(r8));
            
            /* Call external function - values must survive across call */
            fp(&data[i]);
            
            /* More calculations using the live values */
            acc1 += r10 + r11;
            acc2 += r9 * r8;
            acc3 += (r10 ^ r11) | (r9 & r8);
            
            /* Use volatile to prevent reordering */
            sum1 = acc1;
            sum2 = acc2;
            
            /* Force register pressure with inline asm */
            asm volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3));
        }
        
        /* Store results back, creating dependencies */
        data[i] = (acc1 + acc2 + acc3) & 0xFFFF;
        
        /* Rotate register values */
        long tmp = r10;
        r10 = r11;
        r11 = r9;
        r9 = r8;
        r8 = tmp;
    }
    
    /* Final computation using all registers */
    long result = r10 + r11 + r9 + r8 + acc1 + acc2 + acc3;
    return (int)(result & 0x7FFFFFFF);
}

/* Another test with indirect calls */
int __attribute__((noinline)) test_indirect_calls(int *data, int size) {
    /* Array of function pointers */
    func_ptr_t funcs[] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3
    };
    
    register long rax_sim asm("r10") = 0x12345678;
    register long rcx_sim asm("r11") = 0x9ABCDEF0;
    register long rdx_sim asm("r9") = 0x55555555;
    
    volatile int counter = 0;
    long total = 0;
    
    for (int i = 0; i < size; i += 2) {
        /* Keep values live across multiple different calls */
        for (int f = 0; f < 3; f++) {
            /* Complex computation in registers */
            rax_sim = (rax_sim << 3) | (rax_sim >> 61);
            rcx_sim = (rcx_sim * 13) + 17;
            rdx_sim = rdx_sim ^ (rdx_sim >> 16);
            
            /* Call via function pointer - inhibits optimizations */
            funcs[f](&data[i]);
            
            /* Use the register values after call */
            total += rax_sim + rcx_sim + rdx_sim;
            
            /* Force spill/reload with volatile */
            counter++;
            
            /* Inline asm to create artificial dependencies */
            asm volatile("" : "+r"(rax_sim), "+r"(rcx_sim), "+r"(rdx_sim));
        }
        
        /* Store using register values */
        data[i] = (rax_sim ^ rcx_sim ^ rdx_sim) & 0xFF;
        data[i + 1] = total & 0xFF;
    }
    
    return total & 0x7FFF;
}

int main() {
    const int SIZE = 256;
    int *data = malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data[i] = i * 1103515245 + 12345;
    }
    
    printf("Starting caller-save test...\n");
    
    /* Test 1: Direct function call with register pressure */
    int result1 = test_function(data, SIZE / 2, (func_ptr_t)external_func1);
    printf("Test 1 result: %d\n", result1);
    
    /* Test 2: Indirect calls */
    int result2 = test_indirect_calls(data + SIZE / 2, SIZE / 2);
    printf("Test 2 result: %d\n", result2);
    
    /* Verify some results */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= data[i];
    }
    printf("Final checksum: %08x\n", checksum);
    
    free(data);
    return 0;
}
