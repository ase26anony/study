#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int global_seed = 42;

/* Function to force register pressure with explicit register variables */
__attribute__((noinline))
static int func_explicit_registers(int a, int b, int c, int d, int e, int f) {
    /* Explicit register variables - compete for specific registers */
    register int r12_val asm("r12") = a + 1;
    register int r13_val asm("r13") = b + 2;
    register int r14_val asm("r14") = c + 3;
    register int r15_val asm("r15") = d + 4;
    
    /* Volatile to prevent optimization */
    volatile int v1 = e;
    volatile int v2 = f;
    
    int result;
    
    /* Inline assembly with mismatched constraints and clobbers */
    asm volatile (
        /* Output constraint doesn't match input mode */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=m" (result)        /* Memory output */
        : [in1] "r" (r12_val),       /* Register input */
          [in2] "rm" (v1)            /* Register or memory - ambiguous */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc", "memory"
    );
    
    /* More assembly with conflicting constraints */
    asm volatile (
        "imull %[in3], %[in4]\n\t"
        "movl %%eax, %[out2]\n\t"
        : [out2] "=rm" (v2)          /* Register or memory output */
        : [in3] "r" (r13_val),       /* Register input */
          [in4] "i" (global_seed)    /* Immediate input - may need reload */
        : "rax", "rdx", "cc"
    );
    
    return result + v2 + r14_val + r15_val;
}

/* Function to force address reloads with volatile pointers */
__attribute__((noinline))
static void func_volatile_addresses(volatile int* ptr1, volatile short* ptr2, 
                                   volatile char* ptr3, int idx) {
    /* Complex address calculation with volatile index */
    volatile int v_idx = idx;
    
    /* Multiple memory accesses with different widths */
    char c_val = ptr3[v_idx];
    short s_val = ptr2[v_idx * 2];
    int i_val = ptr1[v_idx];
    
    /* Inline assembly with memory output and register input */
    asm volatile (
        "movb %[char_in], %%al\n\t"
        "movw %[short_in], %%bx\n\t"
        "addw %%bx, %%ax\n\t"
        "movb %%al, %[char_out]\n\t"
        : [char_out] "=m" (ptr3[v_idx + 1])    /* Memory output */
        : [char_in] "r" (c_val),               /* Register input */
          [short_in] "rm" (s_val)              /* Register or memory */
        : "rax", "rbx", "cc"
    );
    
    /* Assembly with immediate to memory */
    asm volatile (
        "movl $0x12345678, %[out]\n\t"
        : [out] "=m" (ptr1[v_idx + 2])         /* Direct memory store */
        :                                     /* No inputs */
        : "memory"
    );
    
    /* Mixed width operation requiring mode change */
    long long ll_val = (long long)i_val * (long long)s_val;
    ptr2[v_idx] = (short)(ll_val & 0xFFFF);
}

/* Function with mixed types and mode conversions */
__attribute__((noinline))
static long long func_mixed_types(char c, short s, int i, long l) {
    /* Volatile locals to prevent optimization */
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long vl = l;
    
    /* Mixed type operations causing mode changes */
    long long ll_result = 0;
    
    /* char -> long long extension */
    ll_result += (long long)vc;
    
    /* short -> int -> long long with arithmetic */
    ll_result += (long long)(vi * vs);
    
    /* Complex expression with multiple conversions */
    for (volatile int j = 0; j < 4; j++) {
        /* Each iteration requires reloading volatile j */
        char temp_c = vc + j;
        short temp_s = vs - j;
        int temp_i = vi * (temp_c + 1);
        
        /* Inline assembly with multiple constraints */
        asm volatile (
            "movsbl %[c_in], %%eax\n\t"        /* Sign extend char */
            "movswl %[s_in], %%ebx\n\t"        /* Sign extend short */
            "addl %%ebx, %%eax\n\t"
            "cltq\n\t"                         /* Sign extend to 64-bit */
            "addq %%rax, %[ll_out]\n\t"
            : [ll_out] "+r" (ll_result)        /* Read-write register */
            : [c_in] "rm" (temp_c),            /* Char in register/memory */
              [s_in] "rm" (temp_s)             /* Short in register/memory */
            : "rax", "rbx", "cc"
        );
    }
    
    /* Union to force strange memory accesses */
    union {
        int i;
        short s[2];
        char c[4];
    } u;
    
    u.i = vi;
    ll_result += u.s[0] * u.s[1];
    
    return ll_result;
}

/* Function with pointer arithmetic and complex addressing */
__attribute__((noinline))
static int func_complex_addressing(int* base, int offset1, int offset2) {
    volatile int voff1 = offset1;
    volatile int voff2 = offset2;
    
    /* Complex address calculation */
    int* ptr1 = base + voff1;
    int* ptr2 = base + voff2;
    
    int result;
    
    /* Assembly with base+index addressing that might need reloads */
    asm volatile (
        "movl (%[base], %[idx1], 4), %%eax\n\t"
        "addl (%[base], %[idx2], 4), %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=rm" (result)
        : [base] "r" (base),
          [idx1] "r" (voff1),
          [idx2] "r" (voff2)
        : "rax", "cc", "memory"
    );
    
    return result;
}

int main(int argc, char** argv) {
    /* Initialize with volatile to prevent constant folding */
    volatile int seed = global_seed + (argc > 1 ? atoi(argv[1]) : 0);
    
    /* Many local variables of different types */
    char c1 = seed & 0xFF;
    short s1 = (seed >> 8) & 0xFFFF;
    int i1 = seed * 3;
    long l1 = seed * 5L;
    long long ll1 = seed * 7LL;
    
    volatile char vc = c1 + 1;
    volatile short vs = s1 - 1;
    volatile int vi = i1 * 2;
    volatile long vl = l1 / 2;
    
    /* Arrays for address calculations */
    int arr_int[32];
    short arr_short[64];
    char arr_char[128];
    
    /* Initialize arrays */
    for (int i = 0; i < 32; i++) {
        arr_int[i] = seed + i;
    }
    for (int i = 0; i < 64; i++) {
        arr_short[i] = (short)(seed - i);
    }
    for (int i = 0; i < 128; i++) {
        arr_char[i] = (char)(seed ^ i);
    }
    
    /* Call functions repeatedly to increase reload pressure */
    int sum = 0;
    
    for (volatile int iter = 0; iter < 8; iter++) {
        /* Force reloads by using iter in address calculations */
        int idx = iter * 4;
        
        /* Call function with explicit register variables */
        sum += func_explicit_registers(vi, vs, vc, idx, iter, seed);
        
        /* Call function with volatile addresses */
        func_volatile_addresses((volatile int*)arr_int, 
                               (volatile short*)arr_short,
                               (volatile char*)arr_char,
                               idx % 32);
        
        /* Call function with mixed types */
        ll1 += func_mixed_types(arr_char[idx], arr_short[idx*2], 
                               arr_int[idx/2], vl);
        
        /* Call function with complex addressing */
        sum += func_complex_addressing(arr_int, idx % 16, (idx + 1) % 16);
        
        /* Additional inline assembly with many clobbers */
        asm volatile (
            "movl %[val1], %%eax\n\t"
            "movl %[val2], %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=rm" (arr_int[iter])
            : [val1] "ri" (sum),      /* Register or immediate */
              [val2] "rm" (vi)        /* Register or memory */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "cc", "memory"
        );
    }
    
    /* Final checksum computation using all variables */
    long long final_checksum = ll1;
    final_checksum += sum;
    final_checksum += arr_int[0];
    final_checksum += arr_short[0];
    final_checksum += arr_char[0];
    final_checksum += vi;
    final_checksum += vs;
    final_checksum += vc;
    
    printf("Checksum: %lld\n", final_checksum);
    
    return (int)(final_checksum & 0x7FFFFFFF);
}
