/* reload_stress.c - Stress GCC's reload pass to trigger rld[] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;
volatile void *g_volatile_ptr = NULL;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int use_explicit_registers(int a, int b) {
    /* Explicit register variables that conflict with constraints */
    register int x asm("r12") = a + g_volatile_seed;
    register int y asm("r13") = b - g_volatile_seed;
    register int z asm("r14") = 0;
    
    /* Inline asm with mismatched constraints */
    asm volatile (
        "addl %[x], %[y]\n\t"
        "movl %[y], %[z]\n\t"
        : [z] "=r" (z)          /* Output in register */
        : [x] "rm" (x),         /* Input: register OR memory */
          [y] "0" (y)           /* Input tied to output */
        : "cc", "r12", "r13", "r14"
    );
    
    /* More complex constraints forcing reloads */
    int result;
    asm volatile (
        "imull %[x], %[y]\n\t"
        "movl %[y], %[out]\n\t"
        : [out] "=rm" (result)  /* Output: register OR memory */
        : [x] "r" (x),          /* Input must be register */
          [y] "rm" (y)          /* Input: register OR memory */
        : "cc", "rax", "rdx"
    );
    
    return result + z;
}

/* Function with memory constraints and volatile addresses */
__attribute__((noinline))
static void memory_constraints_ops(volatile int *arr, int idx) {
    volatile int temp = 0;
    volatile int *volatile ptr = &temp;
    
    /* Complex addressing with volatile index */
    int *base = (int*)arr;
    volatile int index = idx;
    
    /* Inline asm with memory output and immediate input */
    asm volatile (
        "movl %[imm], (%[mem])\n\t"
        : [mem] "=m" (*(int*)(base + index))  /* Memory output */
        : [imm] "i" (0x1234)                  /* Immediate input */
        : "memory"
    );
    
    /* Multiple constraints forcing reloads */
    int val1 = *ptr;
    int val2 = g_volatile_seed;
    
    asm volatile (
        "leal (%[v1], %[v2], 2), %[v1]\n\t"
        : [v1] "+r,m" (val1)    /* Read-write with alternatives */
        : [v2] "r,m" (val2)     /* Input with alternatives */
        : "cc"
    );
    
    /* Store with complex address calculation */
    *(base + index + 1) = val1;
}

/* Function with mixed types causing mode changes */
__attribute__((noinline))
static long mixed_type_computations(char c, short s, int i, long l) {
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long vl = l;
    
    /* Operations causing mode changes */
    long result = 0;
    for (volatile int j = 0; j < 4; j++) {
        /* Mix types - will require extensions/truncations */
        result += (long)vc;      /* char -> long */
        result += (long)vs;      /* short -> long */
        result += vi;            /* int -> long (may need extension) */
        result *= vl;            /* long multiplication */
        
        /* Bitfield operations */
        union {
            struct {
                unsigned int a : 3;
                unsigned int b : 5;
                unsigned int c : 8;
            } bits;
            unsigned int word;
        } u;
        
        u.word = (unsigned int)result;
        vi = u.bits.b + u.bits.c;
        
        /* Pointer casting causing address reloads */
        uintptr_t addr = (uintptr_t)&vc;
        addr += (uintptr_t)&vs;
        void *ptr = (void*)addr;
        (void)ptr;  /* Use to prevent elimination */
    }
    
    /* Inline asm with clobbered registers */
    asm volatile (
        "mov %[in], %%rax\n\t"
        "add $1, %%rax\n\t"
        "mov %%rax, %[out]\n\t"
        : [out] "=rm" (result)
        : [in] "rm" (result)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc", "memory"
    );
    
    return result;
}

/* Function with many local variables causing register pressure */
__attribute__((noinline))
static int high_register_pressure(int iterations) {
    /* Many local variables of different types */
    char c1 = 1, c2 = 2, c3 = 3;
    short s1 = 100, s2 = 200, s3 = 300;
    int i1 = 1000, i2 = 2000, i3 = 3000, i4 = 4000, i5 = 5000;
    long l1 = 10000, l2 = 20000, l3 = 30000;
    float f1 = 1.1f, f2 = 2.2f;
    double d1 = 3.3, d2 = 4.4;
    
    /* Take addresses to force spills */
    volatile char *pc1 = &c1, *pc2 = &c2;
    volatile short *ps1 = &s1, *ps2 = &s2;
    volatile int *pi1 = &i1, *pi2 = &i2, *pi3 = &i3;
    
    /* Complex loop with many operations */
    volatile int sum = 0;
    for (volatile int i = 0; i < iterations; i++) {
        /* Mixed operations causing reloads */
        i1 = *pc1 + *pc2;
        i2 = *ps1 * *ps2;
        i3 = i1 ^ i2;
        
        /* Inline asm with many clobbers */
        asm volatile (
            "movl %[a], %%eax\n\t"
            "addl %[b], %%eax\n\t"
            "movl %%eax, %[c]\n\t"
            : [c] "=rm" (i4)
            : [a] "rm" (i1),
              [b] "rm" (i2)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc"
        );
        
        /* Pointer arithmetic with volatile */
        int offset = g_volatile_seed % 16;
        sum += *(pi1 + offset % 3);
        sum += *(pi2 + offset % 2);
        sum += *(pi3 + offset % 1);
        
        /* Type conversions */
        l1 = (long)i1 * (long)i2;
        l2 = (long)s1 + (long)s2;
        l3 = l1 / (l2 ? l2 : 1);
        
        /* Use all variables to prevent elimination */
        f1 = (float)i1 / 100.0f;
        f2 = (float)i2 / 200.0f;
        d1 = (double)l1;
        d2 = (double)l2;
    }
    
    /* Final computation using all locals */
    return sum + c1 + c2 + c3 + s1 + s2 + s3 + i1 + i2 + i3 + i4 + i5 
           + (int)l1 + (int)l2 + (int)l3 + (int)f1 + (int)f2 + (int)d1 + (int)d2;
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize volatile global */
    g_volatile_seed = (argc > 1) ? atoi(argv[1]) : 42;
    g_volatile_ptr = &result;
    
    /* Array for complex addressing */
    volatile int arr[32];
    for (int i = 0; i < 32; i++) {
        arr[i] = i * g_volatile_seed;
    }
    
    /* Call functions to trigger various reload scenarios */
    result += use_explicit_registers(argc, g_volatile_seed);
    
    memory_constraints_ops((int*)arr, argc % 16);
    
    result += mixed_type_computations(
        (char)(g_volatile_seed & 0xFF),
        (short)(g_volatile_seed & 0xFFFF),
        g_volatile_seed * 2,
        (long)g_volatile_seed * 100
    );
    
    result += high_register_pressure(argc + 2);
    
    /* Additional stress with loop */
    for (volatile int i = 0; i < 10; i++) {
        /* Force many reloads in loop */
        register int r1 asm("r8") = i;
        register int r2 asm("r9") = result;
        
        asm volatile (
            "xchgl %[a], %[b]\n\t"
            : [a] "+r" (r1), [b] "+r" (r2)
            :
            : "cc"
        );
        
        result = r1 + r2;
        
        /* Complex memory operation */
        int idx = (i * 7) % 32;
        asm volatile (
            "movl (%[ptr], %[idx], 4), %%eax\n\t"
            "addl %%eax, %[res]\n\t"
            : [res] "+rm" (result)
            : [ptr] "r" (arr),
              [idx] "r" (idx)
            : "rax", "cc", "memory"
        );
    }
    
    /* Compute checksum */
    long checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += arr[i];
    }
    checksum += result;
    
    printf("Result: %d, Checksum: %ld\n", result, checksum);
    
    return (checksum > 0) ? 0 : 1;
}
