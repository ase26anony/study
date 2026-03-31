/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;
volatile void *g_volatile_ptr = NULL;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int complex_addressing(int idx, volatile int *base) {
    volatile char buffer[256];
    volatile short *sptr = (volatile short *)buffer;
    volatile long *lptr = (volatile long *)buffer;
    
    /* Force different addressing modes with volatile index */
    volatile int vidx = idx;
    int result = 0;
    
    /* Complex address calculation that may need reloads */
    asm volatile (
        "movl %[idx], %%eax\n\t"
        "leal (%%eax,%%eax,2), %%ecx\n\t"
        "movl %[base], %%edx\n\t"
        "addl (%%edx,%%ecx,4), %[res]\n\t"
        : [res] "+r" (result)
        : [idx] "rm" (vidx), [base] "rm" (base)
        : "eax", "ecx", "edx", "memory", "cc"
    );
    
    /* Mixed mode operations */
    char c = buffer[vidx % 256];
    short s = sptr[vidx % 128];
    long l = lptr[vidx % 64];
    
    /* Operations requiring mode changes */
    result += (int)c + (int)s + (int)l;
    
    return result;
}

/* Function with explicit register variables and conflicting constraints */
__attribute__((noinline))
static int register_conflicts(int a, int b) {
    /* Explicit register variables */
    register int x asm("r12") = a;
    register int y asm("r13") = b;
    register int z asm("r14");
    
    /* Inline asm with mismatched constraints */
    asm volatile (
        "addl %[in1], %[out]\n\t"
        "subl %[in2], %[out]\n\t"
        : [out] "=r,m" (z)  /* Alternative constraints */
        : [in1] "r,m,i" (x), [in2] "r,m,i" (y)
        : "cc"
    );
    
    /* Clobber many registers to force spills */
    asm volatile (
        ""
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13",
          "r14", "r15", "memory", "cc"
    );
    
    return z;
}

/* Function with memory output constraints and immediate inputs */
__attribute__((noinline))
static void memory_constraints(volatile int *out1, volatile long *out2) {
    int temp1, temp2;
    
    /* Memory output with immediate input - may require reload */
    asm volatile (
        "movl $0x12345678, %[mem1]\n\t"
        "movq $0x9ABCDEF012345678, %[mem2]\n\t"
        : [mem1] "=m" (*out1), [mem2] "=m" (*out2)
        :
        : "memory"
    );
    
    /* Mixed constraints with alternatives */
    asm volatile (
        "imull %%eax, %%eax\n\t"
        "movl %%eax, %[res]\n\t"
        : [res] "=rm" (temp1)
        : "a" (0x100)
        : "cc"
    );
    
    *out1 += temp1;
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static long mixed_types_operations(char c, short s, int i, long l) {
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long vl = l;
    
    long result = 0;
    
    /* Operations causing mode changes */
    result = (long)vc;           /* zero/sign extend char to long */
    result += (long)vs << 8;     /* short to long with shift */
    result += (long)vi * 256;    /* int to long with multiplication */
    result += vl;                /* long stays long */
    
    /* Bitfield operations */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } bits = {0};
    
    bits.a = vc & 0x7;
    bits.b = (vs >> 3) & 0x1F;
    bits.c = vi & 0xFF;
    bits.d = (vi >> 8) & 0xFFFF;
    
    result += bits.a + bits.b + bits.c + bits.d;
    
    /* Union for type punning */
    union {
        int i;
        float f;
        char bytes[4];
    } pun;
    
    pun.i = vi;
    for (int j = 0; j < 4; j++) {
        result += pun.bytes[j];
    }
    
    return result;
}

/* Function with pointer arithmetic and complex constraints */
__attribute__((noinline))
static int pointer_arithmetic(volatile int *arr, int size, int idx) {
    volatile int *ptr = arr;
    int sum = 0;
    volatile int v_idx = idx;
    
    /* Complex pointer arithmetic that may need reloads */
    for (volatile int i = 0; i < size; i++) {
        /* Address calculation with multiple components */
        volatile int *elem = ptr + v_idx + i * 2 - size / 2;
        
        /* Inline asm with memory constraint */
        int val;
        asm volatile (
            "movl (%[addr]), %[val]\n\t"
            : [val] "=r" (val)
            : [addr] "r" (elem)
            : "memory"
        );
        
        sum += val;
        
        /* Clobber registers to force more reloads */
        asm volatile ("" ::: "rax", "rbx", "rcx", "rdx");
    }
    
    return sum;
}

/* Main function that orchestrates all stress tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize with volatile to prevent constant folding */
    volatile int seed = g_volatile_seed + argc;
    
    /* Array with volatile elements */
    volatile int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = seed + i * 3;
    }
    
    /* Test 1: Complex addressing modes */
    result += complex_addressing(seed % 50, (volatile int *)arr);
    
    /* Test 2: Register conflicts */
    result += register_conflicts(seed, seed * 2);
    
    /* Test 3: Memory constraints */
    volatile int out1;
    volatile long out2;
    memory_constraints(&out1, &out2);
    result += out1 + (int)out2;
    
    /* Test 4: Mixed types */
    result += mixed_types_operations(
        (char)(seed & 0xFF),
        (short)(seed & 0xFFFF),
        seed,
        (long)seed * 1000
    );
    
    /* Test 5: Pointer arithmetic */
    result += pointer_arithmetic(arr, 50, seed % 25);
    
    /* Loop to increase register pressure */
    for (volatile int i = 0; i < 10; i++) {
        /* Use many local variables to increase register pressure */
        int a = result + i;
        char b = (char)(a & 0xFF);
        short c = (short)(a & 0xFFFF);
        long d = (long)a * i;
        void *p = &a;
        
        /* Inline asm using all variables with clobbers */
        asm volatile (
            "addl %%eax, %[res]\n\t"
            "addb %%bl, %[res_b]\n\t"
            "addw %%cx, %[res_c]\n\t"
            "addq %%rdx, %[res_d]\n\t"
            : [res] "+r" (result), 
              [res_b] "+r" (*(char *)&result),
              [res_c] "+r" (*(short *)&result),
              [res_d] "+r" (*(long *)&result)
            : "a" (a), "b" (b), "c" (c), "d" (d), [ptr] "r" (p)
            : "cc", "memory"
        );
    }
    
    /* Final checksum to prevent elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
