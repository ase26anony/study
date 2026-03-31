/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
    *p ^= 0xAAAAAAAAAAAAAAAA;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function pointer type */
typedef void (*func_ptr_t)(void*);

/* Main test function with register pressure */
int __attribute__((noinline)) test_function(int *data, int size, func_ptr_t fp) {
    /* Use explicit register variables for call-clobbered registers */
    register long r10_val asm("r10") = 0;
    register long r11_val asm("r11") = 0;
    register long r8_val asm("r8") = 0;
    register long r9_val asm("r9") = 0;
    
    /* Use call-saved registers too */
    register long r12_val asm("r12") = 0;
    register long r13_val asm("r13") = 0;
    register long r14_val asm("r14") = 0;
    register long r15_val asm("r15") = 0;
    
    /* Volatile variables to prevent optimization */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile long v3 = 3;
    
    int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < size; i++) {
        /* Inner loop with register-intensive calculations */
        for (int j = 0; j < 4; j++) {
            /* Load values into call-clobbered registers */
            r10_val = data[i] + j;
            r11_val = data[i] * j;
            r8_val = r10_val ^ r11_val;
            r9_val = r10_val + r11_val + r8_val;
            
            /* Use inline assembly to create dependencies */
            asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(r8_val), "+r"(r9_val));
            
            /* Mix with call-saved registers */
            r12_val = r10_val * 2;
            r13_val = r11_val * 3;
            r14_val = r8_val * 4;
            r15_val = r9_val * 5;
            
            /* More arithmetic keeping values live */
            r10_val += r12_val;
            r11_val += r13_val;
            r8_val += r14_val;
            r9_val += r15_val;
            
            /* Volatile operations */
            v1 = r10_val;
            v2 = r11_val;
            v3 = r8_val + r9_val;
            
            /* Function call that clobbers registers */
            if (j % 2 == 0) {
                external_func1(&data[i]);
            } else {
                long temp = r10_val + r11_val;
                external_func2(&temp);
                r10_val = temp;
            }
            
            /* Use values after call - they must be saved/restored */
            r12_val += r10_val;
            r13_val += r11_val;
            r14_val += r8_val;
            r15_val += r9_val;
            
            /* Another call via function pointer */
            if (fp) {
                long temp2 = r12_val + r13_val;
                fp(&temp2);
                r12_val = temp2;
            }
            
            /* More calculations with live values */
            sum += r10_val + r11_val + r8_val + r9_val;
            sum += r12_val + r13_val + r14_val + r15_val;
            
            /* Force register spilling with array access */
            data[(i + j) % size] = sum;
        }
        
        /* Nested loop with indirect call */
        for (int k = 0; k < 2; k++) {
            register long rax_val asm("rax") = data[i] + k;
            register long rcx_val asm("rcx") = data[i] * k;
            register long rdx_val asm("rdx") = data[i] ^ k;
            
            asm volatile("" : "+r"(rax_val), "+r"(rcx_val), "+r"(rdx_val));
            
            /* Call that clobbers these specific registers */
            external_func3((unsigned long*)&data[i]);
            
            /* Use values after call */
            sum += rax_val * 2 + rcx_val * 3 + rdx_val * 4;
            
            /* Inline assembly to prevent reordering */
            asm volatile("" : : "r"(rax_val), "r"(rcx_val), "r"(rdx_val));
        }
    }
    
    return sum;
}

/* Alternate test with different patterns */
int __attribute__((noinline)) test_function2(int *data, int size) {
    int result = 0;
    
    /* Create complex live ranges */
    for (int i = 0; i < size - 1; i++) {
        register int a asm("r10") = data[i];
        register int b asm("r11") = data[i + 1];
        register int c asm("r8") = 0;
        register int d asm("r9") = 0;
        
        /* Loop with multiple calls */
        for (int iter = 0; iter < 3; iter++) {
            c = a * iter;
            d = b * (iter + 1);
            
            /* Force values to be live across call */
            asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
            
            /* Call that clobbers registers */
            if (iter == 0) {
                external_func1(&data[i]);
            } else if (iter == 1) {
                long temp = a + b;
                external_func2(&temp);
                a = temp;
            } else {
                unsigned long temp = c ^ d;
                external_func3(&temp);
                c = temp;
            }
            
            /* Complex use of live values */
            result += a * c - b * d;
            
            /* Store to memory to force spills */
            data[i] = result;
        }
    }
    
    return result;
}

int main() {
    const int SIZE = 256;
    int *data = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pattern */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        data[i] = rand() % 100;
    }
    
    /* Array of function pointers */
    func_ptr_t funcs[] = {
        (func_ptr_t)external_func1,
        (func_ptr_t)external_func2,
        (func_ptr_t)external_func3,
        NULL
    };
    
    int total = 0;
    
    /* Multiple iterations with different patterns */
    for (int iter = 0; iter < 10; iter++) {
        /* Test with different function pointers */
        func_ptr_t fp = funcs[iter % 3];
        
        /* Call test functions */
        total += test_function(data, SIZE, fp);
        total += test_function2(data, SIZE);
        
        /* Modify data slightly */
        for (int i = 0; i < SIZE; i++) {
            data[i] = (data[i] * 13 + 7) % 1000;
        }
    }
    
    printf("Result: %d\n", total);
    
    /* Verify computation */
    int check = 0;
    for (int i = 0; i < SIZE; i++) {
        check += data[i];
    }
    printf("Data checksum: %d\n", check);
    
    free(data);
    return 0;
}
