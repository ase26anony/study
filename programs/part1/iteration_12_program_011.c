/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of variables */
static volatile int vol_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int complex_addressing(int idx, volatile int* restrict ptr) {
    volatile char buffer[256];
    volatile short* sptr = (volatile short*)buffer;
    volatile long* lptr = (volatile long*)buffer;
    
    /* Force different machine modes and addressing */
    buffer[idx] = (char)(vol_seed + idx);
    sptr[idx % 128] = (short)(vol_seed * idx);
    lptr[idx % 64] = (long)(vol_seed + idx * 3);
    
    /* Complex address calculation that may need reloads */
    int result;
    asm volatile (
        "movl %[idx], %%eax\n\t"
        "leal (%%eax,%%eax,2), %%ecx\n\t"  /* idx * 3 */
        "movl %[ptr], %%edx\n\t"
        "movl (%%edx,%%ecx,4), %[res]\n\t"
        : [res] "=r" (result)
        : [idx] "rm" (idx), [ptr] "rm" (ptr)
        : "eax", "ecx", "edx", "memory"
    );
    
    return result + buffer[idx] + sptr[idx % 128];
}

/* Function with explicit register variables and conflicting constraints */
__attribute__((noinline))
static int register_conflicts(int a, int b) {
    /* Explicit register variables that conflict with constraints */
    register int x asm("r12") = a + vol_seed;
    register int y asm("r13") = b - vol_seed;
    register int z asm("r14") = a * b;
    
    int result1, result2;
    
    /* Inline asm with mismatched constraints forcing reloads */
    asm volatile (
        "addl %[x], %[y]\n\t"
        "movl %[y], %[r1]\n\t"
        "imull %[z], %[x]\n\t"
        "movl %[x], %[r2]"
        : [r1] "=rm" (result1), [r2] "=rm" (result2)
        : [x] "0" (x), [y] "rm" (y), [z] "rm" (z)
        : "cc", "r12", "r13", "r14"
    );
    
    /* More conflicts with memory operands */
    volatile int mem1 = result1;
    volatile int mem2 = result2;
    
    asm volatile (
        "movl %[m1], %%eax\n\t"
        "addl %[m2], %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=m" (mem1)
        : [m1] "m" (mem1), [m2] "i" (100)  /* Immediate with memory constraint */
        : "eax", "memory"
    );
    
    return mem1 + mem2 + x + y + z;
}

/* Function with mixed types causing mode changes */
__attribute__((noinline))
static long mixed_type_ops(char c, short s, int i, long l) {
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long vl = l;
    
    /* Operations causing mode changes */
    long result = 0;
    
    /* char in 64-bit operation */
    result += (long)vc * 256;
    
    /* short in 32-bit operation with extension */
    result += (long)(vs * vi);
    
    /* Mixed operations with volatile forcing reloads */
    for (volatile int j = 0; j < 4; j++) {
        vc += (char)(j + vol_seed);
        vs -= (short)(vc * 2);
        vi ^= (int)(vs | j);
        vl += (long)(vi * vc);
    }
    
    /* Inline asm with multiple clobbers forcing spills */
    asm volatile (
        "mov %[vc], %%al\n\t"
        "mov %[vs], %%bx\n\t"
        "mov %[vi], %%ecx\n\t"
        "mov %[vl], %%rdx\n\t"
        "add %%rdx, %[res]\n\t"
        "imul %%rcx, %[res]"
        : [res] "+r" (result)
        : [vc] "m" (vc), [vs] "m" (vs), [vi] "m" (vi), [vl] "m" (vl)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    return result;
}

/* Function with pointer arithmetic and complex constraints */
__attribute__((noinline))
static int* pointer_arithmetic(int *base, volatile int offset) {
    volatile int local_arr[32];
    int *result_ptr;
    
    /* Complex address calculation */
    for (volatile int i = 0; i < 16; i++) {
        local_arr[i] = base[i] + offset + i;
        local_arr[i + 16] = base[i] * offset - i;
    }
    
    /* Inline asm with "m" constraint on output */
    asm volatile (
        "lea (%[base],%[offset],4), %[out]\n\t"
        "movl $0x12345678, (%[out])"
        : [out] "=r" (result_ptr)
        : [base] "r" (base), [offset] "r" (offset)
        : "memory"
    );
    
    /* Force more reloads with mismatched constraints */
    int temp;
    asm volatile (
        "movl (%[ptr]), %[tmp]\n\t"
        "addl %%eax, %[tmp]\n\t"
        "movl %[tmp], (%[ptr])"
        : [tmp] "=r" (temp), [ptr] "+r" (result_ptr)
        : "eax" (vol_seed)
        : "cc", "memory"
    );
    
    return result_ptr;
}

/* Union to create subreg operations */
union mixed_data {
    char c[8];
    short s[4];
    int i[2];
    long l;
    void *p;
};

__attribute__((noinline))
static long union_ops(union mixed_data *data) {
    volatile union mixed_data local;
    local.l = data->l;
    
    /* Operations causing subreg accesses */
    local.c[0] += (char)vol_seed;
    local.s[1] = (short)(local.i[0] >> 16);
    local.i[1] ^= 0xFFFF0000;
    
    long result;
    
    /* Inline asm accessing different parts of union */
    asm volatile (
        "movzbl %[c], %%eax\n\t"
        "movzwl %[s], %%ebx\n\t"
        "movl %[i], %%ecx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "cltq\n\t"
        "addq %[l], %%rax\n\t"
        "movq %%rax, %[res]"
        : [res] "=rm" (result)
        : [c] "m" (local.c[0]), [s] "m" (local.s[1]),
          [i] "m" (local.i[0]), [l] "rm" (local.l)
        : "rax", "rbx", "rcx", "rdx", "cc"
    );
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize with volatile to prevent constant propagation */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : vol_seed;
    vol_seed = seed;
    
    /* Many local variables of different types */
    char c1 = (char)(seed + 1);
    short s1 = (short)(seed * 2);
    int i1 = seed + 100;
    long l1 = seed * 1000L;
    int arr[64];
    
    /* Initialize array with non-constant values */
    for (volatile int i = 0; i < 64; i++) {
        arr[i] = seed + i * 3;
    }
    
    /* Call functions to create reload situations */
    int sum = 0;
    
    sum += complex_addressing(seed % 64, arr + 32);
    sum += register_conflicts(seed, seed + 100);
    
    l1 += mixed_type_ops(c1, s1, i1, l1);
    sum += (int)l1;
    
    int *ptr = pointer_arithmetic(arr, seed % 32);
    sum += *ptr;
    
    union mixed_data data;
    data.l = (long)seed * 0x12345678ABCDEFLL;
    sum += (int)union_ops(&data);
    
    /* Additional stress with loop containing inline asm */
    for (volatile int j = 0; j < 8; j++) {
        int temp;
        asm volatile (
            "movl %[sum], %%eax\n\t"
            "roll $3, %%eax\n\t"
            "addl %[j], %%eax\n\t"
            "movl %%eax, %[tmp]\n\t"
            : [tmp] "=rm" (temp)
            : [sum] "rm" (sum), [j] "rm" (j)
            : "eax", "cc"
        );
        sum = temp ^ (seed + j);
    }
    
    /* Final computation to prevent elimination */
    printf("Result checksum: %d\n", sum);
    return sum & 0xFF;
}
