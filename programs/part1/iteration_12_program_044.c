/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization and constant propagation */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void use_complex_addresses(volatile int *arr, int idx1, int idx2) {
    /* Force base+index*scale addressing with reloads */
    register int r12_val asm("r12") = idx1;
    register int r13_val asm("r13") = idx2;
    
    /* Inline asm with mismatched constraints */
    asm volatile (
        "movl %[idx1], %[out1]\n\t"
        "addl %[idx2], %[out2]\n\t"
        : [out1] "=m" (arr[r12_val + 0]),  /* Memory output */
          [out2] "=m" (arr[r13_val * 2])   /* Complex addressing */
        : [idx1] "ri" (r12_val),           /* Register or immediate */
          [idx2] "r" (r13_val)             /* Register only */
        : "memory", "cc", "rax", "rbx", "rcx", "rdx", "rsi", "rdi"
    );
}

/* Function with explicit register variables and clobbers */
__attribute__((noinline))
static int register_pressure(register int a asm("r10"), 
                             register char b asm("r11"),
                             register short c asm("r12")) {
    int result;
    long long temp;
    
    /* Mixed type operations causing mode changes */
    temp = (long long)a + (long long)b * 256 + (long long)c;
    
    /* Inline asm with many clobbered registers */
    asm volatile (
        "mov %[a], %%r10\n\t"
        "mov %[b], %%r11\n\t"
        "mov %[c], %%r12\n\t"
        "add %%r11, %%r10\n\t"
        "add %%r12, %%r10\n\t"
        "mov %%r10, %[res]\n\t"
        : [res] "=r" (result)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c)
        : "r10", "r11", "r12", "cc", "rax", "rbx", "rcx", "rdx", "rsi", "rdi"
    );
    
    return result + (int)temp;
}

/* Function with memory constraints and immediates */
__attribute__((noinline))
static void memory_constraints(volatile int *ptr1, volatile short *ptr2) {
    int temp1, temp2;
    
    /* Take addresses of volatile variables */
    int *addr1 = (int *)ptr1;
    short *addr2 = (short *)ptr2;
    
    /* Assembly with memory output and immediate input */
    asm volatile (
        "movl $0x12345678, %[mem1]\n\t"
        "movw $0x9ABC, %[mem2]\n\t"
        : [mem1] "=m" (*addr1),
          [mem2] "=m" (*addr2)
        : /* no inputs */
        : "memory", "cc", "rax", "rbx"
    );
    
    /* More complex constraints with alternatives */
    asm volatile (
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out2]\n\t"
        : [out1] "=r,m" (temp1),
          [out2] "=r,m" (temp2)
        : [in1] "r,i,m" (g_volatile_seed),
          [in2] "r,i,m" (g_volatile_seed + 1)
        : "cc"
    );
}

/* Function with type mixing and conversions */
__attribute__((noinline))
static long long type_mixing(char c, short s, int i, long long ll) {
    volatile int counter = 0;
    long long result = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (counter = 0; counter < 10; counter++) {
        /* Mixed type operations requiring extensions/truncations */
        int temp_int = (int)c + (int)s * (counter + 1);
        long long temp_ll = (long long)i + (long long)temp_int;
        
        /* Bitfield-like operations */
        union {
            struct {
                unsigned char a : 3;
                unsigned char b : 5;
            } bits;
            unsigned char byte;
        } u;
        
        u.byte = c + counter;
        temp_ll += u.bits.a * 100 + u.bits.b * 10;
        
        /* Pointer casting creating complex RTL */
        uintptr_t ptr_val = (uintptr_t)&counter;
        result += temp_ll + (ptr_val & 0xFFF);
    }
    
    return result;
}

/* Main function creating maximum register pressure */
int main(int argc, char *argv[]) {
    /* Many local variables of different types */
    volatile int v1 = g_volatile_seed;
    volatile char v2 = 'A' + argc;
    volatile short v3 = 1000 + argc;
    volatile long v4 = 123456789L * argc;
    volatile int *ptr_arr[10];
    volatile int data[100];
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 100; i++) {
        data[i] = g_volatile_seed + i * 3;
    }
    
    /* Take addresses to create complex addressing */
    for (int i = 0; i < 10; i++) {
        ptr_arr[i] = &data[i * 10];
    }
    
    /* Call functions to create various reload situations */
    long long checksum = 0;
    
    /* 1. Complex addressing with register pressure */
    use_complex_addresses((int *)data, argc, argc * 2);
    checksum += data[argc] + data[argc * 2];
    
    /* 2. Explicit register variables with clobbers */
    int reg_result = register_pressure(v1, v2, v3);
    checksum += reg_result;
    
    /* 3. Memory constraints */
    memory_constraints(&v1, (short *)&v3);
    checksum += v1 + v3;
    
    /* 4. Type mixing in loop */
    long long mix_result = type_mixing(v2, v3, v1, v4);
    checksum += mix_result;
    
    /* Additional stress: multiple calls with different args */
    for (volatile int i = 0; i < 5; i++) {
        checksum += register_pressure(v1 + i, v2 + i, v3 + i);
        use_complex_addresses((int *)data, i, i + 10);
        checksum += data[i] + data[i + 10];
    }
    
    /* Use all variables to prevent elimination */
    checksum += (long long)v1 * v2 * v3 * v4;
    checksum += (long long)&v1 & 0xFFFF;
    checksum += (long long)&v2 & 0xFFFF;
    checksum += (long long)&v3 & 0xFFFF;
    
    /* Final computation to ensure all code is used */
    printf("Checksum: %lld\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
