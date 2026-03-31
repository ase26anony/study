/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void use_complex_addresses(volatile int* arr, int idx1, int idx2) {
    /* Force base+index addressing with reloads */
    volatile long result = 0;
    
    /* Array access with volatile indices */
    volatile int* ptr1 = &arr[idx1];
    volatile int* ptr2 = &arr[idx2];
    
    /* Inline asm with memory constraints that may need reloads */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (*ptr1)          /* Output to memory */
        : "m" (*ptr2), "i" (5)  /* Input from memory + immediate */
        : "eax", "memory"
    );
    
    /* More complex addressing with pointer arithmetic */
    int offset = g_volatile_seed & 0xF;
    volatile int* complex_ptr = ptr1 + offset;
    
    asm volatile (
        "movl (%1), %%ebx\n\t"
        "imull %%ebx, %%ebx\n\t"
        "movl %%ebx, (%0)\n\t"
        : 
        : "r" (complex_ptr), "r" (ptr2)
        : "ebx", "memory"
    );
}

/* Function with explicit register variables and mismatched constraints */
__attribute__((noinline))
static void explicit_register_conflicts(void) {
    /* Explicit register variables that conflict with constraints */
    register int r12_var asm("r12") = g_volatile_seed + 1;
    register int r13_var asm("r13") = g_volatile_seed + 2;
    register int r14_var asm("r14") = g_volatile_seed + 3;
    
    volatile int output;
    
    /* Inline asm with conflicting constraints */
    asm volatile (
        "addl %[in1], %[in2]\n\t"
        "movl %[in2], %[out]\n\t"
        : [out] "=rm" (output)      /* Output can be reg or memory */
        : [in1] "r" (r12_var),      /* Input must be register */
          [in2] "r" (r13_var)       /* Another register input */
        : "cc"
    );
    
    /* Force use of all explicit registers in different constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r" (r14_var)            /* Read-write register operand */
        : "m" (output)              /* Memory operand */
        : "eax", "cc"
    );
    
    /* Clobber many registers to force spills/reloads */
    asm volatile (
        "nop\n\t"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static void mixed_type_operations(volatile char c, volatile short s, 
                                  volatile int i, volatile long long ll) {
    /* Operations that cause mode changes */
    long long temp;
    
    /* char -> long long extension */
    temp = (long long)c;
    
    /* Inline asm with mismatched modes */
    asm volatile (
        "movsx %1, %0\n\t"          /* Sign extend */
        "add %2, %0\n\t"            /* Add different sized operand */
        : "=r" (temp)
        : "r" ((short)c), "r" (i)   /* Mixed size inputs */
        : "cc"
    );
    
    /* Store with truncation */
    s = (short)temp;
    
    /* Pointer casting creating subreg operations */
    volatile int* int_ptr = (volatile int*)&ll;
    volatile char* char_ptr = (volatile char*)int_ptr;
    
    /* Access with different types */
    asm volatile (
        "movb (%1), %%al\n\t"
        "movb %%al, (%0)\n\t"
        : 
        : "r" (char_ptr + 1), "r" (char_ptr + 2)
        : "al", "memory"
    );
    
    /* Union to force reinterpretation */
    union {
        long long ll;
        struct {
            int a;
            int b;
        } parts;
    } u;
    
    u.ll = ll;
    u.parts.a = i;
    
    /* Bit-field operations */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 24;
    } bits = {0};
    
    bits.a = c & 0x7;
    bits.b = s & 0x1F;
    bits.c = i & 0xFFFFFF;
    
    /* Use bit-fields in computation */
    asm volatile (
        "movl %1, %%eax\n\t"
        "shrl $3, %%eax\n\t"
        "orl %2, %%eax\n\t"
        : "=r" (i)
        : "r" (bits.c), "r" ((int)bits.a)
        : "eax", "cc"
    );
}

/* Function with multiple alternative constraints */
__attribute__((noinline))
static void alternative_constraints(volatile int* mem) {
    int temp1, temp2;
    
    /* Multiple alternative constraints that may force reloads */
    asm volatile (
        "movl %[input], %[output]\n\t"
        "addl $1, %[output]\n\t"
        : [output] "=r,m" (temp1)   /* Output: register OR memory */
        : [input] "rm,i" (g_volatile_seed)  /* Input: reg/mem OR immediate */
        : "cc"
    );
    
    /* Complex constraints with earlyclobber */
    asm volatile (
        "leal (%[a], %[b], 2), %[out]\n\t"
        : [out] "=&r" (temp2)       /* Earlyclobber - can't share reg with inputs */
        : [a] "r" (temp1), 
          [b] "r" (g_volatile_seed)
        : "cc"
    );
    
    /* Memory output with register input */
    asm volatile (
        "imull %1, %1\n\t"
        "movl %1, %0\n\t"
        : "=m" (*mem)
        : "r" (temp2)
        : "cc", "memory"
    );
    
    /* Force address reload with 'p' constraint */
    volatile int* addr = mem + (g_volatile_seed & 3);
    asm volatile (
        "incl (%0)\n\t"
        : 
        : "p" (addr)                /* Address constraint */
        : "memory"
    );
}

/* Main function that creates register pressure */
int main(int argc, char* argv[]) {
    /* Use args to prevent constant folding */
    int base = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Many local variables of different types */
    volatile char c1 = base & 0xFF;
    volatile char c2 = (base >> 8) & 0xFF;
    volatile short s1 = base & 0xFFFF;
    volatile short s2 = (base >> 16) & 0xFFFF;
    volatile int i1 = base;
    volatile int i2 = base * 2;
    volatile int i3 = base * 3;
    volatile int i4 = base * 4;
    volatile long long ll1 = (long long)base * 100;
    volatile long long ll2 = (long long)base * 200;
    
    /* Array for complex addressing */
    volatile int arr[100];
    for (int j = 0; j < 100; j++) {
        arr[j] = base + j;
    }
    
    /* Call functions repeatedly to increase reload opportunities */
    for (volatile int k = 0; k < 10; k++) {
        use_complex_addresses((int*)arr, k, k + 5);
        explicit_register_conflicts();
        mixed_type_operations(c1 + k, s1 + k, i1 + k, ll1 + k);
        alternative_constraints(&arr[k % 50]);
        
        /* Modify variables to prevent optimization */
        c1 ^= k;
        s1 += k;
        i1 *= (k + 1);
        ll1 -= k;
    }
    
    /* Compute checksum to ensure all operations are preserved */
    long long checksum = 0;
    checksum += c1 + c2;
    checksum += s1 + s2;
    checksum += i1 + i2 + i3 + i4;
    checksum += ll1 + ll2;
    
    for (int j = 0; j < 100; j++) {
        checksum += arr[j];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
