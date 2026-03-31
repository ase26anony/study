/* reload_stress_test.c
 * A program designed to stress GCC's reload pass and trigger
 * initialization of reload records, particularly those needing
 * secondary reloads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Use inline asm to make it opaque */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Volatile memory locations to force memory operations */
volatile int vol_mem[256];
volatile long vol_long[256];
volatile float vol_float[256];

/* Complex structure with nested arrays */
struct nested {
    int a[4];
    long b[2];
    struct {
        int x;
        int y;
    } inner[3];
};

/* Global arrays for complex addressing */
int multi_array[8][8][8];
struct nested complex_structs[16];

/* Inline asm helper to clobber many registers */
#define CLOBBER_MANY_ASM() __asm__ volatile ( \
    "# Clobber many registers\n" \
    "movl $0, %%eax\n" \
    "movl $0, %%ebx\n" \
    "movl $0, %%ecx\n" \
    "movl $0, %%edx\n" \
    "movl $0, %%esi\n" \
    "movl $0, %%edi\n" \
    : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory")

/* For ARM/RISC-V architectures, we'd use different clobber lists */
#if defined(__arm__)
#define CLOBBER_MANY_ASM_ARM() __asm__ volatile ( \
    "mov r0, #0\n" \
    "mov r1, #0\n" \
    "mov r2, #0\n" \
    "mov r3, #0\n" \
    "mov r4, #0\n" \
    "mov r5, #0\n" \
    "mov r6, #0\n" \
    "mov r7, #0\n" \
    : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "memory")
#endif

/* Test function with many live variables and complex operations */
__attribute__((noinline))
static long test_reload_stress(int seed, int idx1, int idx2, int idx3) {
    /* Declare many scalar variables to exhaust registers */
    register int r0 asm ("r12") = seed + 1;  /* Explicit register variable */
    int v1 = barrier(seed * 2);
    int v2 = barrier(seed + 3);
    int v3 = barrier(seed * 4);
    int v4 = barrier(seed + 5);
    int v5 = barrier(seed * 6);
    int v6 = barrier(seed + 7);
    int v7 = barrier(seed * 8);
    int v8 = barrier(seed + 9);
    int v9 = barrier(seed * 10);
    int v10 = barrier(seed + 11);
    int v11 = barrier(seed * 12);
    int v12 = barrier(seed + 13);
    int v13 = barrier(seed * 14);
    int v14 = barrier(seed + 15);
    int v15 = barrier(seed * 16);
    int v16 = barrier(seed + 17);
    int v17 = barrier(seed * 18);
    int v18 = barrier(seed + 19);
    int v19 = barrier(seed * 20);
    int v20 = barrier(seed + 21);
    
    long l1 = (long)v1 * v2;
    long l2 = (long)v3 * v4;
    long l3 = (long)v5 * v6;
    long l4 = (long)v7 * v8;
    long l5 = (long)v9 * v10;
    
    float f1 = (float)v11 / 256.0f;
    float f2 = (float)v12 / 256.0f;
    float f3 = (float)v13 / 256.0f;
    
    /* Complex addressing mode 1: SIB addressing with all components */
    /* array[base + index*scale] where scale != 1, 2, 4, 8 */
    volatile int* volatile ptr = vol_mem;
    int scale = 3;  /* Non-power-of-two scale forces computation */
    int base = idx1;
    int index = idx2;
    
    /* This should require secondary reload on many architectures */
    int val1 = ptr[base + index * scale];
    
    /* Use explicit register in inline asm with complex constraint */
    int temp;
    __asm__ volatile (
        "# Complex constraint with memory operand\n"
        "movl %[memval], %[temp]\n"
        : [temp] "=r" (temp)
        : [memval] "m" (ptr[base + index * 2 + 3])  /* Complex address */
        : "memory"
    );
    
    /* More complex: nested structure with variable indices */
    volatile int idx_a = idx1 & 0x3;
    volatile int idx_b = idx2 & 0x1;
    volatile int idx_c = idx3 & 0x2;
    
    /* Complex addressing with multiple variable indices */
    int val2 = complex_structs[idx_a].inner[idx_b].x;
    int val3 = complex_structs[idx_c].a[idx_b];
    
    /* Multi-dimensional array with variable indices */
    int val4 = multi_array[idx_a][idx_b][idx_c];
    
    /* Clobber many registers to force reloads around asm */
    CLOBBER_MANY_ASM();
    
    /* Long dependency chain using all variables */
    r0 = r0 + v1 + v2 - v3 + v4 - v5 + v6;
    v1 = v1 + v7 - v8 + v9 - v10 + v11;
    v2 = v2 + v12 - v13 + v14 - v15 + v16;
    v3 = v3 + v17 - v18 + v19 - v20 + r0;
    
    l1 = l1 + l2 - l3 + l4 - l5 + (long)v1;
    l2 = l2 + (long)v2 * 3 - (long)v3 * 2 + (long)v4;
    
    /* Mix integer and float operations */
    f1 = f1 + (float)v5 / 128.0f - (float)v6 / 64.0f;
    f2 = f2 * 2.0f - f3 + (float)v7 / 32.0f;
    
    /* Atomic operations with complex addresses */
    int atomic_temp;
    __atomic_load(&multi_array[idx_a][idx_b][idx_c], &atomic_temp, __ATOMIC_RELAXED);
    atomic_temp += val1 + val2 + val3 + val4;
    __atomic_store(&multi_array[idx_a][idx_b][idx_c], &atomic_temp, __ATOMIC_RELAXED);
    
    /* More inline asm with register constraints */
    int out1, out2;
    __asm__ volatile (
        "# Multiple output operands with different constraints\n"
        "movl %[in1], %[out1]\n"
        "leal (%[in2], %[in3], 4), %[out2]\n"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [in1] "r" (v1), [in2] "r" (v2), [in3] "r" (v3)
        : "cc"
    );
    
    /* Use union for type-punning between int and float */
    union {
        int i;
        float f;
    } punner;
    
    punner.i = v8;
    f3 = punner.f * 0.5f;  /* Forces move between register classes */
    punner.f = f3;
    v8 = punner.i;
    
    /* Another clobber to force more reloads */
    CLOBBER_MANY_ASM();
    
    /* Final computation using all variables */
    long result = (long)r0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  l1 + l2 + l3 + l4 + l5 +
                  (long)(f1 * 1000.0f) + (long)(f2 * 1000.0f) + (long)(f3 * 1000.0f) +
                  val1 + val2 + val3 + val4 + temp + out1 + out2 + atomic_temp;
    
    return barrier(result);
}

/* Second test function focusing on different reload patterns */
__attribute__((noinline))
static int test_secondary_reloads(int base_idx) {
    /* Use explicit register variables to allocate specific registers */
    register int reg_a asm ("ebx") = base_idx * 2;
    register int reg_b asm ("esi") = base_idx * 3;
    register int reg_c asm ("edi") = base_idx * 4;
    
    /* Complex memory operation that likely needs secondary reload */
    int result;
    
    /* Memory operand with displacement that might not be encodable */
    __asm__ volatile (
        "# Memory operand with large displacement\n"
        "movl 0x1234(,%0,4), %1\n"
        : "=r" (result)
        : "0" (reg_a)
        : "memory"
    );
    
    /* Multiple memory accesses with complex addressing */
    for (int i = 0; i < 8; i++) {
        /* Varying scale factors */
        int scale = (i % 3) + 2;  /* 2, 3, or 4 */
        vol_mem[reg_b + i * scale] = reg_c + i;
        
        /* Nested array access */
        multi_array[i & 7][(i + 1) & 7][(i + 2) & 7] += 
            complex_structs[i & 3].a[(i + 1) & 3];
    }
    
    /* Mix with float operations */
    float fvals[4];
    for (int i = 0; i < 4; i++) {
        fvals[i] = (float)(reg_a + i) / 256.0f;
        vol_float[i] = fvals[i] * 2.0f;
    }
    
    /* Type punning between float and int arrays */
    union {
        float f[4];
        int i[4];
    } converter;
    
    for (int i = 0; i < 4; i++) {
        converter.f[i] = fvals[i];
        reg_c += converter.i[i];  /* Forces moves between register classes */
    }
    
    return barrier(reg_a + reg_b + reg_c + result);
}

int main(int argc, char *argv[]) {
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        vol_mem[i] = i * 3;
        vol_long[i] = i * 5L;
        vol_float[i] = i * 0.1f;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                multi_array[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 4; j++) {
            complex_structs[i].a[j] = i * 10 + j;
        }
        for (int j = 0; j < 2; j++) {
            complex_structs[i].b[j] = i * 20L + j;
        }
        for (int j = 0; j < 3; j++) {
            complex_structs[i].inner[j].x = i * 30 + j * 2;
            complex_structs[i].inner[j].y = i * 30 + j * 2 + 1;
        }
    }
    
    /* Use command line arguments to create variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    int idx1 = (argc > 2) ? atoi(argv[2]) : 3;
    int idx2 = (argc > 3) ? atoi(argv[3]) : 5;
    int idx3 = (argc > 4) ? atoi(argv[4]) : 7;
    
    /* Call test functions multiple times with different parameters */
    long total = 0;
    
    for (int i = 0; i < 10; i++) {
        long result1 = test_reload_stress(seed + i, 
                                         (idx1 + i) & 0x7, 
                                         (idx2 + i * 2) & 0x7,
                                         (idx3 + i * 3) & 0x7);
        
        int result2 = test_secondary_reloads((seed + i * 11) & 0xF);
        
        total += result1 + result2;
        
        /* Modify globals between calls to prevent optimization */
        vol_mem[i] = result1 & 0xFF;
        vol_long[i] = result2 * 2L;
    }
    
    /* Final computation using volatile memory */
    int final_check = 0;
    for (int i = 0; i < 20; i++) {
        final_check += vol_mem[i];
        final_check += (int)vol_long[i];
    }
    
    total += final_check;
    
    printf("Result: %ld\n", total);
    return (int)(total & 0x7FFFFFFF);
}
