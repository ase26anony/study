/* reload_test.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to prevent optimization */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Force register pressure with many live variables */
__attribute__((noinline, noipa))
static long test_reloads(int a, int b, int c, int d, int e,
                         int f, int g, int h, int i, int j,
                         int k, int l, int m, int n, int o,
                         int p, int q, int r, int s, int t) {
    /* Declare many local variables to exhaust registers */
    volatile int v1 = a + 1;
    volatile int v2 = b + 2;
    volatile int v3 = c + 3;
    volatile int v4 = d + 4;
    volatile int v5 = e + 5;
    volatile int v6 = f + 6;
    volatile int v7 = g + 7;
    volatile int v8 = h + 8;
    volatile int v9 = i + 9;
    volatile int v10 = j + 10;
    volatile int v11 = k + 11;
    volatile int v12 = l + 12;
    volatile int v13 = m + 13;
    volatile int v14 = n + 14;
    volatile int v15 = o + 15;
    volatile int v16 = p + 16;
    volatile int v17 = q + 17;
    volatile int v18 = r + 18;
    volatile int v19 = s + 19;
    volatile int v20 = t + 20;
    
    /* Complex multi-dimensional array with volatile indices */
    volatile int idx1 = barrier(a) % 10;
    volatile int idx2 = barrier(b) % 10;
    volatile int idx3 = barrier(c) % 10;
    
    int array[10][10][10];
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                array[x][y][z] = x * 100 + y * 10 + z;
            }
        }
    }
    
    /* Complex addressing mode: array[idx1][idx2][idx3] with scaling */
    /* This often requires secondary reloads on RISC architectures */
    int complex_addr = array[idx1][idx2][idx3];
    
    /* Inline assembly that clobbers many registers */
    /* Force reloads around the asm block */
    __asm__ volatile (
        "# Clobber many registers to force reloads\n"
        "mov %0, %0\n"  /* Use input */
        :
        : "r" (complex_addr)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* Mixed register classes: integer to floating point */
    float f1 = (float)v1;
    float f2 = (float)v2;
    float f3 = f1 * f2;
    int int_from_float = (int)f3;
    
    /* Use explicit register variables to allocate specific registers */
    register int reg_var1 asm ("r12") = v3 + v4;
    register int reg_var2 asm ("r13") = v5 + v6;
    
    /* Complex inline assembly with multiple constraints */
    /* This creates both input and output reloads */
    int result1, result2;
    __asm__ volatile (
        "# Complex constraints for reload testing\n"
        "add %[out1], %[in1], %[in2]\n"
        "sub %[out2], %[in3], %[in4]\n"
        : [out1] "=r" (result1), [out2] "=r" (result2)
        : [in1] "r" (reg_var1), [in2] "r" (v7),
          [in3] "r" (reg_var2), [in4] "r" (v8)
        : "cc"
    );
    
    /* Atomic operations with memory ordering - often need special reloads */
    _Atomic int atomic_var = ATOMIC_VAR_INIT(100);
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    __atomic_store_n(&atomic_var, atomic_val + result1, __ATOMIC_RELAXED);
    
    /* Structure with nested arrays for complex addressing */
    struct nested {
        int data[5][5];
        int extra;
    };
    
    struct nested nested_array[10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                nested_array[i].data[j][k] = i * 25 + j * 5 + k;
            }
        }
        nested_array[i].extra = i * 100;
    }
    
    volatile int struct_idx = barrier(v9) % 10;
    volatile int inner_idx1 = barrier(v10) % 5;
    volatile int inner_idx2 = barrier(v11) % 5;
    
    /* Complex structure addressing - may need secondary reload */
    int struct_val = nested_array[struct_idx].data[inner_idx1][inner_idx2];
    
    /* Union for type-punning between integer and float */
    union pun {
        int i;
        float f;
    } pun_union;
    
    pun_union.i = v12;
    float from_pun = pun_union.f;
    pun_union.f = from_pun * 2.0f;
    int back_to_int = pun_union.i;
    
    /* Long dependency chain using all variables */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   complex_addr + int_from_float + result1 + result2 +
                   atomic_val + struct_val + back_to_int;
    
    /* More inline assembly with memory constraints */
    /* Force memory operands that need reloading */
    int mem_temp;
    __asm__ volatile (
        "# Memory constraint forcing reload\n"
        "ldr %0, [%1]\n"
        : "=r" (mem_temp)
        : "m" (nested_array[struct_idx].extra)
        : "memory"
    );
    
    checksum += mem_temp;
    
    /* Vector extension usage on supported architectures */
    /* This forces moves between vector and scalar registers */
#ifdef __GNUC__
    typedef int v4si __attribute__ ((vector_size (16)));
    v4si vec1 = {v1, v2, v3, v4};
    v4si vec2 = {v5, v6, v7, v8};
    v4si vec3 = vec1 + vec2;
    
    /* Extract elements - forces scalar reloads */
    int vec_elem;
    __asm__ volatile (
        "# Extract vector element\n"
        "umov %w0, %1.s[0]\n"
        : "=r" (vec_elem)
        : "w" (vec3)
    );
    
    checksum += vec_elem;
#endif
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
    int vals[20];
    for (int i = 0; i < 20; i++) {
        vals[i] = barrier(argc + i);
    }
    
    /* Call test function with all variables */
    long result = test_reloads(
        vals[0], vals[1], vals[2], vals[3], vals[4],
        vals[5], vals[6], vals[7], vals[8], vals[9],
        vals[10], vals[11], vals[12], vals[13], vals[14],
        vals[15], vals[16], vals[17], vals[18], vals[19]
    );
    
    printf("Reload test checksum: %ld\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
