/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions that will be called */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) external_func2(int *p) {
    *p *= 2;
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) external_func3(int *p) {
    *p -= 3;
    asm volatile("" : : : "memory");
}

/* Function with extensive register usage and calls */
int __attribute__((noinline)) compute_with_saves(int *data, int size, 
                                                 void (*callback)(int*)) {
    /* Use explicit register variables to target call-clobbered registers */
    register long r10 asm("r10") = data[0];
    register long r11 asm("r11") = data[1];
    register long r12 asm("r12") = data[2];  /* Call-saved on x86-64 */
    register long r13 asm("r13") = data[3];  /* Call-saved on x86-64 */
    register long rax_val asm("rax") = 0;
    register long rcx_val asm("rcx") = 0;
    register long rdx_val asm("rdx") = 0;
    
    volatile int barrier = 0;
    
    /* Complex loop with multiple live values across calls */
    for (int i = 0; i < size; i++) {
        /* Create artificial dependencies to prevent optimization */
        asm volatile("" : "+r"(r10), "+r"(r11) : : "memory");
        
        /* Mix of operations using both call-clobbered and call-saved registers */
        rax_val = r10 + r11 + i;
        rcx_val = r12 - r13 + i * 2;
        rdx_val = (r10 * r11) ^ (r12 | r13);
        
        /* Force values to be live across the call */
        barrier = rax_val + rcx_val + rdx_val;
        
        /* Call external function - values in rax, rcx, rdx need saving */
        callback(&barrier);
        
        /* Use the values after call - they must be restored */
        r10 = rax_val ^ barrier;
        r11 = rcx_val + barrier;
        r12 = rdx_val - barrier;  /* Call-saved register */
        r13 = (r10 + r11) * barrier;  /* Call-saved register */
        
        /* Store results back, creating more register pressure */
        data[i % 8] = r10 + r11 + r12 + r13;
        
        /* Nested loop to create complex control flow */
        for (int j = 0; j < 3; j++) {
            /* More register operations */
            register long r8_val asm("r8") = r10 + j;
            register long r9_val asm("r9") = r11 - j;
            
            /* Another call inside nested loop */
            if (j % 2 == 0) {
                external_func1(&barrier);
            } else {
                external_func2(&barrier);
            }
            
            /* Use values after call */
            r10 = r8_val ^ barrier;
            r11 = r9_val & barrier;
            
            /* Volatile to prevent reordering */
            asm volatile("" : : "r"(r8_val), "r"(r9_val) : "memory");
        }
    }
    
    /* Final computation using all registers */
    asm volatile("" : "+r"(r10), "+r"(r11), "+r"(r12), "+r"(r13),
                       "+r"(rax_val), "+r"(rcx_val), "+r"(rdx_val) : : "memory");
    
    return r10 + r11 + r12 + r13 + rax_val + rcx_val + rdx_val + barrier;
}

/* Function with indirect calls via function pointer */
int __attribute__((noinline)) indirect_calls(int *data, int size) {
    /* Array of function pointers */
    void (*funcs[3])(int*) = {external_func1, external_func2, external_func3};
    
    int result = 0;
    volatile int counter = 0;
    
    /* Loop with indirect calls */
    for (int i = 0; i < size; i++) {
        /* Use multiple call-clobbered registers */
        register long a asm("rax") = data[i];
        register long b asm("rcx") = data[(i + 1) % size];
        register long c asm("rdx") = data[(i + 2) % size];
        register long d asm("rsi") = data[(i + 3) % size];
        
        /* Keep values live across indirect call */
        a = a * b + c;
        b = b ^ d - i;
        c = c | a * 3;
        d = d & b + 7;
        
        /* Artificial dependency */
        counter = a + b + c + d;
        
        /* Indirect call - compiler doesn't know which registers are clobbered */
        funcs[i % 3](&counter);
        
        /* Use values after call - forces save/restore */
        a = a + counter;
        b = b - counter;
        c = c ^ counter;
        d = d | counter;
        
        /* Store results, creating more register pressure */
        data[i] = a + b + c + d;
        result += data[i];
        
        /* Inline assembly to prevent optimization */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d) : : "memory");
    }
    
    return result;
}

/* Main function that drives the test */
int main() {
    const int SIZE = 256;
    int *data = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        data[i] = rand() % 100;
    }
    
    printf("Starting caller-save test...\n");
    
    /* Test 1: Direct calls with register pressure */
    int result1 = compute_with_saves(data, 32, external_func1);
    printf("Result 1: %d\n", result1);
    
    /* Test 2: Indirect calls */
    int result2 = indirect_calls(data, 64);
    printf("Result 2: %d\n", result2);
    
    /* Test 3: Mix of different call patterns */
    for (int i = 0; i < 16; i++) {
        void (*fp)(int*) = (i % 2) ? external_func2 : external_func3;
        
        /* Create register pressure */
        register long r14 asm("r14") = data[i];      /* Call-saved */
        register long r15 asm("r15") = data[i + 1];  /* Call-saved */
        register long rbx asm("rbx") = data[i + 2];  /* Call-saved */
        
        /* Use call-clobbered registers too */
        register long rax asm("rax") = r14 + 1;
        register long rcx asm("rcx") = r15 * 2;
        register long rdx asm("rdx") = rbx / 3;
        
        /* Force them to be live */
        volatile int temp = rax + rcx + rdx;
        
        /* Call through function pointer */
        fp(&temp);
        
        /* Use all registers after call */
        r14 = rax ^ temp;
        r15 = rcx + temp;
        rbx = rdx - temp;
        
        /* Store back */
        data[i] = r14 + r15 + rbx;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(rax), "r"(rcx), "r"(rdx) : "memory");
    }
    
    /* Final checksum */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= data[i];
    }
    printf("Final checksum: %d\n", checksum);
    
    free(data);
    return 0;
}
