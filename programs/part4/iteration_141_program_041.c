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

/* Complex calculation forcing register usage */
__attribute__((noinline)) int compute_value(int a, int b, int c) {
    /* Force use of multiple registers */
    int t1 = a * b;
    int t2 = b * c;
    int t3 = c * a;
    return t1 + t2 + t3;
}

/* Main test function with register pressure */
void test_caller_save(int *data, int size) {
    /* Use explicit register variables to pressure specific registers */
    #ifdef __x86_64__
    register int64_t r10_val asm("r10") = data[0];
    register int64_t r11_val asm("r11") = data[1];
    #endif
    register int sum asm("eax") = 0;
    register int prod asm("ecx") = 1;
    
    /* Function pointer to force indirect call */
    void (* volatile fp)(int *) = NULL;
    
    /* Nested loops with calls to create complex live ranges */
    for (int i = 0; i < size; i++) {
        /* Load values into registers that must survive calls */
        int val1 = data[i];
        int val2 = data[(i + 1) % size];
        int val3 = data[(i + 2) % size];
        
        /* Force these values to stay in registers */
        asm volatile("" : "+r"(val1), "+r"(val2), "+r"(val3));
        
        /* Switch function pointer to inhibit optimizations */
        switch (i % 3) {
            case 0: fp = external_func1; break;
            case 1: fp = external_func2; break;
            case 2: fp = external_func3; break;
        }
        
        /* Inner loop with register-intensive calculations */
        for (int j = 0; j < 4; j++) {
            /* Compute value using multiple registers */
            int computed = compute_value(val1 + j, val2 - j, val3 * j);
            
            /* Mix of operations on register variables */
            sum += computed;
            prod *= (computed % 7 + 1);
            
            /* Volatile to prevent reordering */
            volatile int temp = sum;
            
            /* Call through function pointer - forces caller-save */
            fp(&temp);
            
            /* Use values after call - they must be restored */
            sum = temp + prod;
            
            #ifdef __x86_64__
            /* Use explicit register variables to keep them live */
            r10_val += sum;
            r11_val ^= prod;
            asm volatile("" : "+r"(r10_val), "+r"(r11_val));
            #endif
        }
        
        /* Store results back, creating dependencies */
        data[i] = sum % 256;
        
        /* More register operations */
        asm volatile("" : "+r"(sum), "+r"(prod));
    }
    
    /* Final computation using all live values */
    int result = sum + prod;
    #ifdef __x86_64__
    result += (int)(r10_val + r11_val);
    #endif
    
    /* Ensure result is used */
    data[0] = result;
}

/* Another test with different pattern */
void test_caller_save2(int *data, int size) {
    /* Array of function pointers */
    void (*funcs[3])(int *) = {external_func1, external_func2, external_func3};
    
    for (int i = 0; i < size; i++) {
        /* Multiple live values in registers */
        int a = data[i];
        int b = data[(i * 3 + 1) % size];
        int c = data[(i * 7 + 2) % size];
        int d = data[(i * 11 + 3) % size];
        
        /* Keep values live across calls */
        for (int k = 0; k < 3; k++) {
            /* Do computation that uses all values */
            int t1 = a * b - c;
            int t2 = b + c * d;
            int t3 = d - a;
            
            /* Force register usage */
            asm volatile("" : "+r"(t1), "+r"(t2), "+r"(t3));
            
            /* Call that clobbers registers */
            funcs[k](&t1);
            
            /* Use all values after call */
            a = t1 + t2;
            b = t2 - t3;
            c = t3 * t1;
            d = a ^ b ^ c;
            
            /* Volatile memory access to prevent optimization */
            volatile int *vp = &data[(i + k) % size];
            *vp = a + b + c + d;
        }
        
        /* Store final results */
        data[i] = a + b + c + d;
    }
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
        data[i] = (i * 37 + 123) % 7919;
    }
    
    /* Run both tests to increase coverage chances */
    test_caller_save(data, SIZE);
    test_caller_save2(data, SIZE / 2);
    
    /* Compute final checksum */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= data[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
