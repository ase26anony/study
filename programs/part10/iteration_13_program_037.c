/* reload_test.c - Test program to stress GCC's reload pass */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to make it opaque */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Volatile memory for forcing memory operations */
volatile int vol_mem[256];
volatile long vol_long[256];
volatile float vol_float[256];

/* Complex structure with nested arrays */
struct nested {
    int a[4][4];
    long b[2][2];
    float c[3][3];
    volatile int d;
};

/* Global to prevent optimization */
struct nested global_nested;

/* Function that uses many registers and complex addressing */
__attribute__((noinline, optimize("O1")))
int test_reloads(int seed) {
    /* Declare many scalar variables to create register pressure */
    register int r0 asm ("r12") = seed + 1;
    register int r1 asm ("r13") = seed + 2;
    int v1 = barrier(seed);
    int v2 = barrier(v1 * 3);
    int v3 = barrier(v2 + 7);
    int v4 = barrier(v3 ^ 0x55);
    int v5 = barrier(v4 << 1);
    int v6 = barrier(v5 | 0xAA);
    int v7 = barrier(v6 - 23);
    int v8 = barrier(v7 & 0xFF);
    int v9 = barrier(v8 / 2);
    int v10 = barrier(v9 % 17);
    long l1 = barrier(v10) * 1000L;
    long l2 = l1 + 5000L;
    long l3 = l2 * 3L;
    long l4 = l3 - 7000L;
    float f1 = (float)v1 * 1.5f;
    float f2 = (float)v2 * 2.5f;
    float f3 = (float)v3 * 3.5f;
    
    /* Force spills with many live variables across function calls */
    v1 = barrier(v1 + v2);
    v2 = barrier(v2 + v3);
    v3 = barrier(v3 + v4);
    v4 = barrier(v4 + v5);
    v5 = barrier(v5 + v6);
    v6 = barrier(v6 + v7);
    v7 = barrier(v7 + v8);
    v8 = barrier(v8 + v9);
    v9 = barrier(v9 + v10);
    v10 = barrier(v10 + v1);
    
    /* Complex addressing modes - SIB addressing on x86 */
    volatile int idx1 = barrier(seed) & 0xF;
    volatile int idx2 = barrier(seed + 1) & 0xF;
    volatile int scale = 4;
    
    /* Force secondary reloads with complex memory addressing */
    int* array = (int*)malloc(256 * sizeof(int));
    for (int i = 0; i < 256; i++) {
        array[i] = barrier(i * 3);
    }
    
    /* Access with complex addressing: array[base + index*scale] */
    int base = barrier(seed) & 0x3F;
    int index = barrier(seed + 2) & 0x3F;
    
    /* This should trigger secondary reloads on many architectures */
    int complex_addr_val = array[base + index * scale];
    complex_addr_val += array[index + base * 2];
    complex_addr_val += array[base * 3 + index * 5];
    
    /* Inline assembly that clobbers many registers */
    __asm__ volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %%eax\n"
        "mov %[val2], %%ebx\n"
        "add %%ebx, %%eax\n"
        "mov %%eax, %[result]\n"
        : [result] "=r" (v1)
        : [val1] "rm" (v2), [val2] "rm" (v3)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* More register pressure with mixed types */
    __asm__ volatile (
        "# Mixed type operations\n"
        "mov %[long_val], %%rax\n"
        "cvtsi2ss %[int_val], %%xmm0\n"
        "movss %%xmm0, %[float_out]\n"
        : [float_out] "=m" (f1)
        : [long_val] "r" (l1), [int_val] "r" (v4)
        : "rax", "xmm0", "xmm1", "memory"
    );
    
    /* Access volatile memory with atomic operations */
    int atomic_val = __atomic_load_n(&vol_mem[idx1], __ATOMIC_RELAXED);
    __atomic_store_n(&vol_mem[idx2], atomic_val + v5, __ATOMIC_RELAXED);
    
    /* Force floating point reloads */
    f2 = f1 * 2.0f + (float)v6;
    f3 = f2 / 1.5f - (float)v7;
    
    /* Use register variables in complex expressions */
    r0 = r0 + r1 * 3 - v8;
    r1 = r1 ^ r0 | v9;
    
    /* Complex structure access with variable indices */
    struct nested local_nested;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            volatile int idx_i = barrier(i + j) & 0x3;
            volatile int idx_j = barrier(i * j) & 0x3;
            local_nested.a[idx_i][idx_j] = v1 + i * 4 + j;
        }
    }
    
    /* Multi-dimensional array with complex addressing */
    int md_array[8][8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                md_array[i][j][k] = barrier(i * 64 + j * 8 + k);
            }
        }
    }
    
    /* Access with multiple variable indices - likely needs secondary reload */
    volatile int i_idx = barrier(v2) & 0x7;
    volatile int j_idx = barrier(v3) & 0x7;
    volatile int k_idx = barrier(v4) & 0x7;
    
    int md_val = md_array[i_idx][j_idx][k_idx];
    md_val += md_array[k_idx][i_idx][j_idx];
    md_val += md_array[j_idx][k_idx][i_idx];
    
    /* Union for type punning - forces moves between register classes */
    union {
        float f;
        int i;
        uint32_t u;
    } punner;
    
    punner.f = f1;
    v1 = barrier(punner.i);
    punner.i = v2;
    f2 = barrier(punner.f);
    
    /* More arithmetic to keep all variables live */
    l1 = l1 + l2 - l3 * l4;
    l2 = l2 ^ l1 | l3;
    l3 = l3 * 2 + l4;
    l4 = l4 - l1 / 2;
    
    v1 = v1 + v2 - v3 * v4;
    v2 = v2 ^ v1 | v3;
    v3 = v3 * 2 + v4;
    v4 = v4 - v1 / 2;
    v5 = v5 + v6 - v7 * v8;
    v6 = v6 ^ v5 | v7;
    v7 = v7 * 2 + v8;
    v8 = v8 - v5 / 2;
    v9 = v9 + v10 - complex_addr_val;
    v10 = v10 ^ v9 | md_val;
    
    /* Final checksum using all variables */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)l1 + (int)l2 + (int)l3 + (int)l4;
    checksum += (int)f1 + (int)f2 + (int)f3;
    checksum += r0 + r1;
    checksum += complex_addr_val + md_val + atomic_val;
    
    free(array);
    return barrier(checksum);
}

/* Second test function focusing on specific reload patterns */
__attribute__((noinline, optimize("O1")))
int test_secondary_reloads(int seed) {
    /* Use explicit register variables to allocate specific registers */
    register int reg_a asm ("eax") = seed;
    register int reg_b asm ("ebx") = seed * 2;
    register int reg_c asm ("ecx") = seed * 3;
    
    /* Complex memory operand that may need secondary reload */
    volatile long* mem_ptr = (volatile long*)&vol_long[0];
    
    /* Inline assembly with memory constraint and complex addressing */
    long result;
    __asm__ volatile (
        "# Force secondary reload with complex address\n"
        "mov (%[ptr], %[idx], 8), %[res]\n"
        : [res] "=r" (result)
        : [ptr] "r" (mem_ptr), [idx] "r" (reg_a & 0x1F)
        : "memory"
    );
    
    /* More complex: memory address with displacement */
    __asm__ volatile (
        "mov 0x100(%[base], %[index], 4), %[out]\n"
        : [out] "=r" (reg_b)
        : [base] "r" (mem_ptr), [index] "r" (reg_b & 0xF)
        : "memory"
    );
    
    /* Force reload between different register classes */
    float float_temp;
    __asm__ volatile (
        "cvtsi2ss %[int_val], %%xmm0\n"
        "movss %%xmm0, %[float_out]\n"
        : [float_out] "=m" (float_temp)
        : [int_val] "r" (reg_c)
        : "xmm0"
    );
    
    /* Use the float value in integer context */
    int int_from_float = (int)float_temp;
    
    /* Create addressing with scale, index, and base all in use */
    int* complex_array = (int*)malloc(1024 * sizeof(int));
    for (int i = 0; i < 1024; i++) {
        complex_array[i] = barrier(i * 5);
    }
    
    volatile int scale_factor = 4;
    volatile int offset = 128;
    int idx_a = barrier(seed) & 0xFF;
    int idx_b = barrier(seed + 100) & 0xFF;
    
    /* Complex addressing that likely needs secondary reload */
    int val1 = complex_array[offset + idx_a * scale_factor];
    int val2 = complex_array[idx_b * 8 + offset / 2];
    
    /* Mix with register variables */
    reg_a = reg_a + val1 * reg_b - val2 / reg_c;
    reg_b = reg_b ^ reg_a | val1;
    reg_c = reg_c * 2 + val2;
    
    free(complex_array);
    return barrier(reg_a + reg_b + reg_c + int_from_float);
}

int main(int argc, char** argv) {
    /* Initialize volatile memory */
    for (int i = 0; i < 256; i++) {
        vol_mem[i] = barrier(i);
        vol_long[i] = barrier(i * 1000L);
        vol_float[i] = (float)barrier(i) * 1.5f;
    }
    
    /* Initialize global structure */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            global_nested.a[i][j] = barrier(i * 4 + j);
        }
    }
    
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    /* Run tests multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        int result1 = test_reloads(seed + i * 100);
        int result2 = test_secondary_reloads(seed + i * 200);
        total = barrier(total + result1 + result2);
    }
    
    printf("Reload test checksum: %d\n", total);
    return total == 0 ? 1 : 0;
}
