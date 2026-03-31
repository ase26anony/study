/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;
volatile void *g_volatile_ptr = NULL;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int complex_addressing(int idx, volatile char *base) {
    volatile int offset = 7;
    int result;
    
    /* Force base+index addressing with volatile components */
    asm volatile (
        "movl (%[base],%[idx],4), %[res]\n\t"
        "addl %[offset], %[res]"
        : [res] "=r" (result)
        : [base] "r" (base), [idx] "r" (idx), [offset] "ri" (offset)
        : "memory", "cc"
    );
    
    return result;
}

/* Function with explicit register variables and mismatched constraints */
__attribute__((noinline))
static void register_conflict(volatile long *out) {
    /* Explicit register variables that conflict with constraints */
    register long r12_val asm("r12") = g_volatile_seed + 1;
    register long r13_val asm("r13") = g_volatile_seed + 2;
    
    /* Inline asm with output memory constraint and input register constraints */
    /* This creates operand mismatches requiring reloads */
    asm volatile (
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]"
        : [out] "=m" (*out)          /* Memory output */
        : [in1] "r" (r12_val),       /* Register input 1 */
          [in2] "r" (r13_val)        /* Register input 2 */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r14", "r15", "cc", "memory"
    );
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static int mixed_type_ops(volatile char c, volatile short s, volatile int i) {
    volatile long long ll_result = 0;
    volatile double d_temp = 0.0;
    
    /* Operations causing mode changes and subreg operations */
    ll_result = (long long)c * (long long)s;  /* char->long long, short->long long */
    ll_result += (long long)i << 8;           /* int->long long with shift */
    
    /* Pointer arithmetic with different types */
    volatile char *ptr = (volatile char*)&ll_result;
    int byte_sum = 0;
    
    for (volatile int j = 0; j < 8; j++) {
        byte_sum += ptr[j];  /* Complex addressing with volatile index */
    }
    
    /* Union causing type punning */
    union {
        long long ll;
        double d;
        int i[2];
    } pun;
    
    pun.ll = ll_result;
    pun.d += 1.0;  /* Changes representation */
    
    return pun.i[0] + pun.i[1] + byte_sum;
}

/* Function with multiple alternative constraints */
__attribute__((noinline))
static void alternative_constraints(volatile int *arr, int n) {
    for (volatile int i = 0; i < n; i++) {
        int temp;
        volatile int idx = i * 2;
        
        /* Multiple alternative constraints forcing reload decisions */
        asm volatile (
            "movl (%[arr],%[idx],4), %[temp]\n\t"
            "imull %[imm], %[temp]"
            : [temp] "=r,r" (temp)           /* Register output alternative */
            : [arr] "r,r" (arr),             /* Register base alternative */
              [idx] "r,m" (idx),             /* Register OR memory index */
              [imm] "i,i" (g_volatile_seed)  /* Immediate alternative */
            : "cc", "memory"
        );
        
        arr[i] = temp;
    }
}

/* Function with bit-field operations */
__attribute__((noinline))
static int bitfield_ops(void) {
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } bits = {0};
    
    volatile unsigned int source = g_volatile_seed;
    
    /* Bit-field assignments causing subreg operations */
    bits.a = source & 0x7;
    bits.b = (source >> 3) & 0x1F;
    bits.c = (source >> 8) & 0xFF;
    bits.d = (source >> 16) & 0xFFFF;
    
    /* Extract and combine bit-fields */
    unsigned int result = bits.a | (bits.b << 3) | (bits.c << 8) | (bits.d << 16);
    
    /* Cast between pointer and integer types */
    volatile char *ptr = (volatile char*)&result;
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += ptr[i];
    }
    
    return sum;
}

/* Main function creating maximum register pressure */
int main(int argc, char *argv[]) {
    /* Initialize with volatile to prevent constant folding */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : g_volatile_seed;
    
    /* Many local variables of different types */
    volatile char c1 = seed & 0xFF;
    volatile char c2 = (seed >> 8) & 0xFF;
    volatile short s1 = seed & 0xFFFF;
    volatile short s2 = (seed >> 16) & 0xFFFF;
    volatile int i1 = seed;
    volatile int i2 = seed * 2;
    volatile long l1 = seed * 3L;
    volatile long l2 = seed * 4L;
    volatile long long ll1 = seed * 5LL;
    volatile long long ll2 = seed * 6LL;
    
    /* Pointer variables */
    volatile int *ptr1 = &i1;
    volatile int *ptr2 = &i2;
    volatile char *cptr = (volatile char*)&l1;
    
    /* Array with complex addressing */
    volatile int arr[32];
    for (volatile int i = 0; i < 32; i++) {
        arr[i] = seed + i;
    }
    
    /* Call functions to create various reload scenarios */
    
    /* 1. Complex addressing with volatile components */
    int addr_result = complex_addressing(seed % 8, cptr);
    
    /* 2. Register conflict with explicit register variables */
    volatile long out_val;
    register_conflict(&out_val);
    
    /* 3. Mixed type operations */
    int mixed_result = mixed_type_ops(c1, s1, i1);
    
    /* 4. Alternative constraints */
    alternative_constraints((volatile int*)arr, 8);
    
    /* 5. Bit-field operations */
    int bitfield_result = bitfield_ops();
    
    /* Additional inline asm with many clobbers to force spills/reloads */
    asm volatile (
        "movq %[ll1], %%rax\n\t"
        "addq %[ll2], %%rax\n\t"
        "movq %%rax, %[temp]\n\t"
        "movl %[i1], %%ebx\n\t"
        "imull %[i2], %%ebx\n\t"
        "addl %%ebx, %[addr_result]"
        : [temp] "=m" (ll1), [addr_result] "+r" (addr_result)
        : [ll1] "r" (ll1), [ll2] "r" (ll2), [i1] "r" (i1), [i2] "r" (i2)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    /* Loop with volatile counter causing many reloads */
    volatile int sum = 0;
    for (volatile int k = 0; k < 16; k++) {
        /* Mixed operations in loop */
        sum += arr[k] + c1 + s1 + i1;
        
        /* Pointer arithmetic with type mixing */
        volatile char *p = (volatile char*)&arr[k];
        for (volatile int j = 0; j < 4; j++) {
            sum += p[j];
        }
    }
    
    /* Final computation using all results */
    int final_result = addr_result + mixed_result + bitfield_result + sum;
    final_result += (int)out_val;
    
    /* Use all variables to prevent elimination */
    final_result += c1 + c2 + s1 + s2;
    final_result += i1 + i2;
    final_result += (int)l1 + (int)l2;
    final_result += (int)ll1 + (int)ll2;
    final_result += (int)(intptr_t)ptr1 + (int)(intptr_t)ptr2;
    
    /* Array checksum */
    for (volatile int i = 0; i < 32; i++) {
        final_result += arr[i];
    }
    
    printf("Result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
