/* reload_stress.c - Stress GCC's reload pass to trigger rld[] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int global_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int complex_addressing(volatile int *arr, int idx1, int idx2) {
    volatile int temp = 0;
    int result;
    
    /* Force base+index*scale addressing with reloads */
    asm volatile (
        "movl (%[base], %[idx1], 4), %%eax\n\t"
        "addl (%[base], %[idx2], 2), %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=r" (result)
        : [base] "r" (arr), [idx1] "r" (idx1), [idx2] "r" (idx2)
        : "eax", "memory", "cc"
    );
    
    /* Use the result in another operation with different mode */
    char char_result = (char)result;
    asm volatile (
        "movsbl %%al, %%eax\n\t"
        "addl %[temp], %%eax\n\t"
        : "=a" (result)
        : "a" (char_result), [temp] "m" (temp)
        : "cc"
    );
    
    return result;
}

/* Function with explicit register variables and conflicting constraints */
__attribute__((noinline))
static int register_conflicts(int a, int b) {
    register int x asm("r12") = a;
    register int y asm("r13") = b;
    int result;
    
    /* Force reload by using same register for input and output with different constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r,m" (result)      /* Multiple constraints force reload consideration */
        : "r,m" (x), "r,m" (y) /* Inputs with alternative constraints */
        : "eax", "cc"
    );
    
    /* Clobber many registers to force spills */
    asm volatile (
        ""
        :
        : "r" (x), "r" (y)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    return result;
}

/* Function with mixed types and mode changes */
__attribute__((noinline))
static long long mixed_type_ops(char c, short s, int i, long long ll) {
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    
    /* Operations causing mode changes */
    long long result = (long long)vc;      /* zero/sign extend char to long long */
    result += (long long)vs << 8;          /* shift with different size operand */
    result += (long long)vi * 256;         /* multiplication with mode change */
    
    /* Inline asm with mismatched operand sizes */
    asm volatile (
        "addq %1, %0\n\t"
        "movq %0, %%rax\n\t"
        "shrq $32, %%rax\n\t"
        "addl %%eax, %k0\n\t"
        : "+r,m" (result)
        : "r,m" (ll)
        : "rax", "cc"
    );
    
    return result;
}

/* Function using volatile addresses in memory constraints */
__attribute__((noinline))
static void memory_constraints(volatile int *out1, volatile short *out2, 
                               volatile char *out3) {
    int in1 = global_seed;
    short in2 = (short)global_seed;
    char in3 = (char)global_seed;
    
    /* Output operands are memory, inputs are immediates/registers */
    asm volatile (
        "movl %1, %0\n\t"
        : "=m" (*out1)
        : "ri" (in1)  /* register or immediate */
        : "memory"
    );
    
    asm volatile (
        "movw %w1, %0\n\t"
        : "=m" (*out2)
        : "ri" (in2)
        : "memory"
    );
    
    asm volatile (
        "movb %b1, %0\n\t"
        : "=m" (*out3)
        : "ri" (in3)
        : "memory"
    );
    
    /* Complex addressing with pointer arithmetic */
    volatile int *ptr = out1 + global_seed % 16;
    asm volatile (
        "addl $1, %0\n\t"
        : "+m" (*ptr)
        :
        : "cc", "memory"
    );
}

/* Function with many local variables to increase register pressure */
__attribute__((noinline))
static int high_register_pressure(int iterations) {
    volatile int counter = iterations;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    /* Use all variables in computation to force register allocation */
    while (counter-- > 0) {
        a += b; b += c; c += d; d += e;
        e += f; f += g; g += h; h += i;
        i += j; j += k; k += l; l += m;
        m += n; n += o; o += p; p += a;
        
        /* Inline asm that clobbers many registers */
        asm volatile (
            ""
            :
            : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), "r" (f), "r" (g), "r" (h),
              "r" (i), "r" (j), "r" (k), "r" (l), "r" (m), "r" (n), "r" (o), "r" (p)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15", "cc"
        );
    }
    
    /* Mix types and sizes */
    short s1 = (short)a;
    char c1 = (char)b;
    long long ll1 = (long long)c + ((long long)d << 32);
    
    asm volatile (
        "movswl %w1, %%eax\n\t"
        "addb %b2, %%al\n\t"
        "cltq\n\t"
        "addq %3, %%rax\n\t"
        : "=a" (ll1)
        : "r" (s1), "r" (c1), "r" (ll1)
        : "cc"
    );
    
    return (int)ll1 + a + b + c;
}

int main(int argc, char *argv[]) {
    /* Use arguments to prevent constant folding */
    int seed = (argc > 1) ? atoi(argv[1]) : 100;
    global_seed = seed;
    
    /* Array for complex addressing */
    volatile int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i + seed;
    }
    
    /* Variables for memory constraints */
    volatile int out1;
    volatile short out2;
    volatile char out3;
    
    long long checksum = 0;
    
    /* Call functions multiple times with different arguments */
    for (int i = 0; i < 10; i++) {
        checksum += complex_addressing(arr, i, i * 2);
        checksum += register_conflicts(i, seed + i);
        checksum += mixed_type_ops(i, i * 2, i * 3, i * 4);
        
        memory_constraints(&out1, &out2, &out3);
        checksum += out1 + out2 + out3;
        
        checksum += high_register_pressure(5 + i);
        
        /* Modify global seed to change behavior */
        global_seed += i;
    }
    
    /* Additional stress with bitfields and unions */
    {
        union {
            int i;
            short s[2];
            char c[4];
        } u;
        
        u.i = seed;
        
        /* Access with different types */
        checksum += u.s[0];
        checksum += u.c[1];
        
        /* Pointer casting */
        int *ptr = (int*)((char*)&u + 1);
        asm volatile (
            "movl (%1), %%eax\n\t"
            "roll $8, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (*ptr)
            : "r" (ptr)
            : "eax", "memory", "cc"
        );
        
        checksum += u.i;
    }
    
    printf("Checksum: %lld\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
