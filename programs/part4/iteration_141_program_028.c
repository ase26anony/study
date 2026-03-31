/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-strict-aliasing caller-save-test.c -o caller-save-test */

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

/* Main test function with register pressure */
long __attribute__((noinline)) test_caller_save(int *data, int size) {
    /* Use explicit register variables to pressure call-clobbered registers */
    register long r10 asm("r10") = 0;
    register long r11 asm("r11") = 0;
    register long r8 asm("r8") = 0;
    register long r9 asm("r9") = 0;
    
    /* Also use call-saved registers */
    register long r12 asm("r12") = 0;
    register long r13 asm("r13") = 0;
    register long r14 asm("r14") = 0;
    register long r15 asm("r15") = 0;
    
    volatile int barrier = 0;
    
    /* Array of function pointers to force indirect calls */
    func_ptr_t funcs[3] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3
    };
    
    /* Nested loops with calls to create complex live ranges */
    for (int outer = 0; outer < 3; ++outer) {
        r12 = data[outer] * 2;  /* Use call-saved register */
        
        for (int inner = 0; inner < size; ++inner) {
            /* Load values into call-clobbered registers */
            r10 = data[inner] + outer;
            r11 = data[inner + 1] * 3;
            r8 = data[inner + 2] ^ 0xFF;
            r9 = r10 + r11 + r8;
            
            /* Force these values to be live across the call */
            asm volatile("" : "+r"(r10), "+r"(r11), "+r"(r8), "+r"(r9));
            
            /* Mix with call-saved registers */
            r13 = r12 + inner;
            r14 = r13 * r10;
            r15 = r14 - r11;
            
            /* Create artificial dependency to prevent optimization */
            barrier = r9;
            
            /* Indirect call - compiler doesn't know which registers it clobbers */
            funcs[inner % 3]((void*)&barrier);
            
            /* Use the live values after the call */
            r10 = r10 + barrier;
            r11 = r11 * (barrier + 1);
            r8 = r8 ^ barrier;
            r9 = r9 - barrier;
            
            /* More operations keeping values live */
            r12 = r12 + r10;
            r13 = r13 + r11;
            r14 = r14 + r8;
            r15 = r15 + r9;
            
            /* Store results back, creating memory pressure */
            data[inner] = (int)(r10 + r11 + r8 + r9);
            
            /* Force all registers to be used */
            asm volatile("" : : "r"(r12), "r"(r13), "r"(r14), "r"(r15));
        }
        
        /* Another call with different register usage pattern */
        external_func1(&data[outer]);
        
        /* Keep call-saved registers live across multiple calls */
        r12 = r12 + r13;
        r13 = r13 + r14;
        r14 = r14 + r15;
        
        external_func2((long*)&data[outer + 1]);
        
        r15 = r15 + r12;
    }
    
    /* Final computation using all registers */
    long result = r10 + r11 + r8 + r9 + r12 + r13 + r14 + r15;
    
    /* Force result to be used */
    asm volatile("" : "+r"(result));
    
    return result;
}

/* Another test function with different pattern */
int __attribute__((noinline)) test_nested_calls(int *arr, int n) {
    volatile int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Use multiple call-clobbered registers in computation */
        register int t1 asm("r10") = arr[i];
        register int t2 asm("r11") = arr[i + 1];
        register int t3 asm("r8") = arr[i + 2];
        register int t4 asm("r9") = arr[i + 3];
        
        /* Complex expression with many live values */
        t1 = t1 * t2 + t3;
        t2 = t2 * t3 + t4;
        t3 = t3 * t4 + t1;
        t4 = t4 * t1 + t2;
        
        /* Force values to be live */
        asm volatile("" : "+r"(t1), "+r"(t2), "+r"(t3), "+r"(t4));
        
        /* Call that clobbers registers */
        external_func3((unsigned long*)&sum);
        
        /* Use values after call */
        arr[i] = t1 + t2;
        arr[i + 1] = t3 + t4;
        
        /* Another call */
        external_func1(&sum);
        
        /* More computation */
        t1 = t1 ^ t2;
        t2 = t2 ^ t3;
        
        external_func2((long*)&sum);
        
        t3 = t3 ^ t4;
        t4 = t4 ^ t1;
        
        sum += t1 + t2 + t3 + t4;
    }
    
    return sum;
}

int main() {
    /* Initialize test data */
    int data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = i + 1;
    }
    
    /* Run first test */
    long result1 = test_caller_save(data, 100);
    printf("Result 1: %ld\n", result1);
    
    /* Re-initialize for second test */
    for (int i = 0; i < 256; i++) {
        data[i] = 256 - i;
    }
    
    /* Run second test with different pattern */
    int result2 = test_nested_calls(data, 50);
    printf("Result 2: %d\n", result2);
    
    /* Verify some results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= data[i];
    }
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
