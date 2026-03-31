/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External functions that will clobber registers */
void __attribute__((noinline)) external_func1(int *p) {
    *p += 1;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func2(int *p) {
    *p *= 2;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) external_func3(int *p) {
    *p -= 3;
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function with intensive register usage and calls */
int __attribute__((noinline)) compute_with_saves(int *data, int size) {
    volatile int barrier = 0;
    
    /* Force values into specific registers */
    register long r10_val asm("r10") = data[0];
    register long r11_val asm("r11") = data[1];
    register long r9_val asm("r9") = data[2];
    register long r8_val asm("r8") = data[3];
    
    /* Mix of call-clobbered and call-saved registers */
    long rax_val = data[4];
    long rcx_val = data[5];
    long rdx_val = data[6];
    long rsi_val = data[7];
    long rdi_val = data[8];
    
    /* Call-saved registers */
    long r12_val = data[9];
    long r13_val = data[10];
    long r14_val = data[11];
    long r15_val = data[12];
    
    /* Function pointer array */
    void (*funcs[3])(int *) = {external_func1, external_func2, external_func3};
    
    /* Nested loops with calls - creates complex live ranges */
    for (int outer = 0; outer < 3; ++outer) {
        /* Artificial dependency to prevent optimization */
        asm volatile("" : "+r"(rax_val), "+r"(rcx_val), "+r"(rdx_val));
        
        for (int inner = 0; inner < size; ++inner) {
            /* Keep many values live across calls */
            r10_val += data[inner] * outer;
            r11_val ^= data[inner] + inner;
            r9_val = (r9_val * 1103515245 + 12345) & 0x7fffffff;
            
            /* Mix in call-saved registers */
            r12_val += r10_val;
            r13_val ^= r11_val;
            r14_val = r14_val * 6364136223846793005ULL + 1;
            
            /* Use volatile to create memory barrier */
            barrier = inner;
            
            /* Indirect call - inhibits optimizations */
            funcs[inner % 3](&data[inner]);
            
            /* More operations keeping values live */
            rax_val = rax_val * 3 + r10_val;
            rcx_val = rcx_val / 2 + r11_val;
            rdx_val = rdx_val ^ r9_val;
            
            /* Use call-clobbered registers immediately before next iteration */
            rsi_val = rax_val + rcx_val;
            rdi_val = rdx_val * r10_val;
            
            /* Force spill/reload around calls */
            asm volatile("" : "+r"(rsi_val), "+r"(rdi_val));
        }
        
        /* Cross-iteration dependencies */
        r10_val = (r10_val + r12_val) & 0xffff;
        r11_val = (r11_val ^ r13_val) | 1;
        r9_val = (r9_val + r14_val) % 1000;
    }
    
    /* Final computation using all registers */
    long result = r10_val + r11_val + r9_val + r8_val +
                  rax_val + rcx_val + rdx_val + rsi_val + rdi_val +
                  r12_val + r13_val + r14_val + r15_val;
    
    /* Store results back */
    data[0] = r10_val;
    data[1] = r11_val;
    data[2] = r9_val;
    data[3] = (int)result;
    
    return (int)result;
}

/* Another function with different pattern */
int __attribute__((noinline)) compute_with_more_saves(int *data, int size) {
    /* Use explicit register variables for call-clobbered registers */
    register int a asm("rax") = 1;
    register int b asm("rbx") = 2;
    register int c asm("rcx") = 3;
    register int d asm("rdx") = 4;
    register int si asm("rsi") = 5;
    register int di asm("rdi") = 6;
    
    volatile int v = 0;
    
    for (int i = 0; i < size; ++i) {
        /* Complex expression with many live values */
        a = a * data[i] + i;
        b = b ^ data[i] * 3;
        c = c + data[i] / 2;
        d = d - data[i] + a;
        
        /* Force values to be live across call */
        v = a + b + c + d;
        
        /* Direct call to external function */
        external_func1(&data[i]);
        
        /* Continue using the values */
        si = si * a + b;
        di = di ^ c + d;
        
        /* Another call with different function */
        if (i % 2 == 0) {
            external_func2(&data[i]);
        } else {
            external_func3(&data[i]);
        }
        
        /* More computations */
        a = si + di;
        b = si * di;
        
        /* Inline assembly to prevent reordering */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
    }
    
    return a + b + c + d + si + di;
}

int main() {
    const int SIZE = 256;
    int *data = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    printf("Initial data[0] = %d\n", data[0]);
    
    /* First computation with intensive register usage */
    int result1 = compute_with_saves(data, SIZE / 4);
    printf("Result 1 = %d, data[0] = %d\n", result1, data[0]);
    
    /* Second computation with different pattern */
    int result2 = compute_with_more_saves(data, SIZE / 8);
    printf("Result 2 = %d, data[0] = %d\n", result2, data[0]);
    
    /* Final verification */
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += data[i];
    }
    printf("Final sum = %d\n", sum);
    
    free(data);
    return 0;
}
