/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization and constant propagation */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int complex_addressing(int idx, volatile int* base) {
    volatile char buffer[256];
    volatile short* ptr = (volatile short*)&buffer[0];
    int result;
    
    /* Force register variable with specific constraint */
    register int r12_val asm("r12") = idx * 2;
    register int r13_val asm("r13") = *base;
    
    /* Inline asm with mismatched constraints and clobbers */
    asm volatile (
        "movl %[idx], %%eax\n\t"
        "addl %%r12d, %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=m" (result)          /* Output to memory */
        : [idx] "rm" (r13_val),           /* Input: register or memory */
          "r" (r12_val)                   /* Input in specific register */
        : "rax", "rbx", "rcx", "rdx",     /* Clobber many registers */
          "rsi", "rdi", "r8", "r9",
          "r10", "r11", "cc", "memory"
    );
    
    /* Complex address calculation */
    ptr += (r12_val & 0x7F);
    *ptr = (short)(result + g_volatile_seed);
    
    return result + *ptr;
}

/* Function with mixed types and mode changes */
__attribute__((noinline))
static long mixed_type_ops(volatile char c, volatile short s, volatile int i) {
    volatile long long ll_result = 0;
    volatile int* volatile ptr_arr[8];
    volatile int dummy = 0;
    
    /* Take addresses of volatile variables */
    ptr_arr[0] = &dummy;
    ptr_arr[1] = (volatile int*)&c;
    ptr_arr[2] = (volatile int*)&s;
    ptr_arr[3] = (volatile int*)&i;
    
    /* Mixed type operations forcing mode changes */
    for (volatile int j = 0; j < 4; j++) {
        /* char in 64-bit operation */
        ll_result += (long long)c * j;
        
        /* short in 32-bit operation with memory constraint */
        int temp;
        asm volatile (
            "movw %[short_val], %%ax\n\t"
            "cwtl\n\t"
            "imull %%eax, %[idx]\n\t"
            "movl %%eax, %[temp]\n\t"
            : [temp] "=rm" (temp)
            : [short_val] "rm" (s),
              [idx] "r" (j)
            : "rax", "rdx", "cc"
        );
        
        /* Store with complex address */
        *ptr_arr[j & 3] = temp + (int)ll_result;
        
        /* Pointer arithmetic */
        ptr_arr[4 + (j & 3)] = ptr_arr[j & 3] + (j * 3);
    }
    
    /* Another asm with output memory constraint and input immediate */
    asm volatile (
        "movq %[input], %%rax\n\t"
        "addq $0x12345678, %%rax\n\t"
        "movq %%rax, %[output]\n\t"
        : [output] "=m" (ll_result)
        : [input] "i" (0x98765432)        /* Immediate input */
        : "rax", "cc"
    );
    
    return (long)ll_result + *ptr_arr[0];
}

/* Function with explicit register variables and conflicts */
__attribute__((noinline))
static int register_conflicts(int a, int b, int c) {
    /* Explicit register variables */
    register int x asm("r10") = a + g_volatile_seed;
    register int y asm("r11") = b * 2;
    register int z asm("rbx") = c - 1;
    
    volatile int results[4];
    volatile int* volatile ptr = &results[0];
    
    /* Multiple asm statements with conflicting constraints */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "addl %[y], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        : [out1] "=m" (results[0])
        : [x] "r" (x), [y] "rm" (y)
        : "rax", "cc"
    );
    
    /* Different constraint pattern */
    asm volatile (
        "movl %[z], %%ebx\n\t"
        "subl $5, %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        : [out2] "=rm" (results[1])
        : [z] "r" (z)
        : "rbx", "cc"
    );
    
    /* Force spill/reload around clobber */
    asm volatile (
        "pushq %%rax\n\t"
        "pushq %%rbx\n\t"
        "pushq %%rcx\n\t"
        "movl $0xAA, %%eax\n\t"
        "movl $0xBB, %%ebx\n\t"
        "movl $0xCC, %%ecx\n\t"
        "popq %%rcx\n\t"
        "popq %%rbx\n\t"
        "popq %%rax\n\t"
        : : : "rax", "rbx", "rcx", "cc", "memory"
    );
    
    /* Pointer chasing to create complex addresses */
    ptr += (x & 3);
    *ptr = y + z;
    ptr += (y & 1);
    *ptr = x * z;
    
    return results[0] + results[1] + *ptr;
}

/* Function with bitfields and unions */
__attribute__((noinline))
static int bitfield_union_ops(volatile int input) {
    union {
        volatile struct {
            unsigned int a : 3;
            unsigned int b : 5;
            unsigned int c : 8;
            unsigned int d : 16;
        } bits;
        volatile uint32_t full;
    } data;
    
    volatile char char_array[32];
    volatile short* short_ptr = (volatile short*)char_array;
    
    data.full = input ^ g_volatile_seed;
    
    /* Operations causing subreg/zero_extend */
    for (volatile int i = 0; i < 8; i++) {
        /* Bitfield extraction */
        unsigned int extracted = (data.bits.b << 2) | data.bits.a;
        
        /* Store with mixed types */
        char_array[i] = (char)(extracted & 0xFF);
        short_ptr[i % 16] = (short)(extracted * i);
        
        /* Inline asm with memory output and register input */
        asm volatile (
            "movzbl %[char_val], %%eax\n\t"
            "addl %%eax, %[int_val]\n\t"
            : [int_val] "+rm" (data.full)
            : [char_val] "m" (char_array[i])
            : "rax", "cc"
        );
    }
    
    /* Final asm with multiple alternatives */
    int final_result;
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "imull %[in2], %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=r,m" (final_result)     /* Alternative constraints */
        : [in1] "r,m" (data.full),
          [in2] "r,m" (input)
        : "rax", "rdx", "cc"
    );
    
    return final_result + char_array[0];
}

int main(int argc, char* argv[]) {
    volatile int seed = g_volatile_seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    volatile char c1 = (char)(seed + 1);
    volatile short s1 = (short)(seed * 2);
    volatile int i1 = seed ^ 0x55AA55AA;
    volatile long l1 = (long)seed * 100;
    
    int checksum = 0;
    
    /* Call functions multiple times with different arguments */
    for (volatile int iter = 0; iter < 3; iter++) {
        checksum += complex_addressing(iter + seed, &i1);
        checksum += mixed_type_ops(c1 + iter, s1 - iter, i1 ^ iter);
        checksum += register_conflicts(seed + iter, seed * iter, seed - iter);
        checksum += bitfield_union_ops(seed ^ (iter << 8));
        
        /* Modify volatiles to prevent optimization */
        c1 += (char)iter;
        s1 -= (short)iter;
        i1 ^= iter * 0x123456;
        l1 += iter * 1000;
    }
    
    /* Use all variables in final calculation */
    checksum += (int)c1 + (int)s1 + (int)l1;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r" (checksum) : "memory");
    
    printf("Checksum: %d (Seed: %d)\n", checksum, seed);
    
    return checksum != 0 ? 0 : 1;
}
