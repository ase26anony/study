/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int volatile_seed = 12345;

/* Function to get unpredictable values */
static inline int get_unpredictable(int base) {
    return base ^ volatile_seed;
}

/* NOINLINE to prevent inlining and keep RTL complex */
__attribute__((noinline, optimize("O0")))
void explicit_register_conflict(int arg1, int arg2) {
    /* Explicit register variables that conflict */
    register int x asm("r12") = get_unpredictable(arg1);
    register int y asm("r13") = get_unpredictable(arg2);
    register int z asm("r14") = 0;
    
    /* Inline assembly with mismatched constraints */
    asm volatile (
        "addl %[in1], %[out]\n\t"
        "subl %[in2], %[out]"
        : [out] "+r,m" (z)        /* Multiple alternatives: register OR memory */
        : [in1] "r,m,i" (x),      /* Register, memory, or immediate */
          [in2] "r,m,i" (y)       /* Multiple constraints force reload decisions */
        : "cc", "memory"
    );
    
    /* Force use of result */
    volatile_seed ^= z;
}

__attribute__((noinline))
void memory_address_reloads(volatile int* addr1, volatile short* addr2) {
    int temp1, temp2;
    long long temp3;
    
    /* Complex addressing with volatile base */
    volatile int* volatile_ptr = addr1;
    int offset = get_unpredictable(10);
    
    /* Assembly with memory output and register input - may need reload */
    asm volatile (
        "movl %[imm], (%[mem])\n\t"
        "movl (%[mem]), %[reg]"
        : [mem] "=m" (*volatile_ptr),  /* Memory output */
          [reg] "=r" (temp1)           /* Register output */
        : [imm] "ri" (get_unpredictable(100))  /* Register or immediate */
        : "memory"
    );
    
    /* Mixed mode operations */
    char char_val = (char)temp1;
    temp3 = (long long)char_val * 256;  /* Mode change: char -> long long */
    
    /* Store with different mode */
    *addr2 = (short)temp3;  /* long long -> short truncation */
    
    /* Another asm with many clobbers */
    asm volatile (
        "mov %[in], %%rax\n\t"
        "add $1, %%rax\n\t"
        "mov %%rax, %[out]"
        : [out] "=rm" (temp2)
        : [in] "rm" (temp1)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc"
    );
    
    volatile_seed += temp2;
}

__attribute__((noinline))
void mixed_type_computations(int iterations) {
    volatile int vi = iterations;
    char c = 'A';
    short s = 1000;
    int i = 100000;
    long long ll = 0;
    int* ptr = (int*)&i;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int v = 0; v < vi; v = v + 1) {
        /* Mixed type operations causing mode changes */
        ll = (long long)c + (long long)s * (long long)i;
        
        /* Pointer arithmetic with type punning */
        ptr = (int*)((char*)ptr + (c & 3));
        *ptr ^= (int)ll;
        
        /* Bitfield-like operations */
        union {
            int full;
            struct {
                unsigned short low;
                unsigned short high;
            } parts;
        } u;
        
        u.full = (int)ll;
        s = u.parts.low;
        c = (char)u.parts.high;
        
        /* Inline asm with subreg operations */
        asm volatile (
            "movzbl %[char_in], %[int_out]\n\t"  /* zero extend char to int */
            "shll $16, %[int_out]"
            : [int_out] "=r" (i)
            : [char_in] "r" (c)
            : "cc"
        );
    }
    
    volatile_seed ^= (int)ll;
}

__attribute__((noinline))
void complex_constraints_example(void) {
    /* Multiple variables with different types */
    volatile char vc = 64;
    volatile short vs = 32000;
    volatile int vi = 1000000;
    volatile long long vll = 0x123456789ABCDEF0LL;
    
    int r1, r2, r3;
    long long r4;
    
    /* Assembly with many operands and constraints */
    asm volatile (
        "mov %[in1], %[out1]\n\t"
        "imul %[in2], %[out1]\n\t"
        "add %[in3], %[out1]\n\t"
        "mov %[out1], %[out2]\n\t"
        "shr $8, %[out2]\n\t"
        "mov %[out2], %[out3]\n\t"
        "and $0xFF, %[out3]"
        : [out1] "=&r,r,m" (r1),  /* Early clobber with alternatives */
          [out2] "=r,m,r" (r2),   /* Different alternative order */
          [out3] "=r,m,r" (r3)
        : [in1] "r,m,i" (vi),     /* Multiple constraints */
          [in2] "r,m,i" ((int)vc),
          [in3] "r,m,i" ((int)vs)
        : "cc", "memory", "rax", "rdx"
    );
    
    /* Use results in another asm */
    asm volatile (
        "mov %[in1], %%rax\n\t"
        "mov %[in2], %%rbx\n\t"
        "add %%rbx, %%rax\n\t"
        "mov %%rax, %[out]"
        : [out] "=rm" (r4)
        : [in1] "rm" ((long long)r1),
          [in2] "rm" ((long long)r2 * (long long)r3)
        : "rax", "rbx", "rcx", "cc"
    );
    
    volatile_seed += (int)r4;
}

int main(int argc, char* argv[]) {
    /* Initialize with unpredictable values */
    int arg1 = get_unpredictable(argc);
    int arg2 = get_unpredictable((int)(uintptr_t)argv);
    
    /* Local variables of different types */
    volatile int vi1 = arg1;
    volatile int vi2 = arg2;
    volatile char vc = 'X';
    volatile short vs = 12345;
    volatile long long vll = 0;
    int array[100];
    volatile int* volatile_ptr = array;
    
    /* Initialize array with complex pattern */
    for (volatile int i = 0; i < 100; i++) {
        array[i] = get_unpredictable(i) ^ (i * 137);
    }
    
    /* Call functions repeatedly to stress reload pass */
    for (int i = 0; i < 10; i++) {
        explicit_register_conflict(vi1 + i, vi2 - i);
        memory_address_reloads(&vi1, &vs);
        mixed_type_computations(5 + (i % 3));
        complex_constraints_example();
        
        /* Modify volatile variables */
        vi1 ^= volatile_seed;
        vi2 += volatile_seed;
        vc += (char)(volatile_seed & 0xFF);
        vs -= (short)(volatile_seed & 0xFFFF);
        
        /* Pointer arithmetic with volatile */
        volatile_ptr = array + (volatile_seed % 50);
        *volatile_ptr = get_unpredictable(*volatile_ptr);
    }
    
    /* Compute checksum to prevent elimination */
    long long checksum = 0;
    checksum += vi1;
    checksum += vi2;
    checksum += (int)vc;
    checksum += vs;
    checksum += vll;
    
    for (int i = 0; i < 100; i++) {
        checksum += array[i];
    }
    
    checksum += volatile_seed;
    
    printf("Checksum: %lld\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
