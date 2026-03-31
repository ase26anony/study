/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Force no optimization on specific variables */
#define VOLATILE_VAR volatile

/* Complex structure to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Multi-dimensional array for complex addressing */
int multi_array[4][8][16];

/* Global variables to ensure liveness */
int global_int = 42;
double global_double = 3.14159;
__m128i global_vec;

int main(void) {
    /* Diverse variable declarations with different types and modes */
    VOLATILE_VAR int int_var = 1;
    VOLATILE_VAR long long_var = 2;
    VOLATILE_VAR float float_var = 3.0f;
    VOLATILE_VAR double double_var = 4.0;
    VOLATILE_VAR __m128i vec_var;
    VOLATILE_VAR int *int_ptr = &int_var;
    VOLATILE_VAR double *double_ptr = &double_var;
    
    /* Arrays for complex addressing */
    int array1d[100] = {0};
    double array2d[10][20];
    struct nested nested_array[5];
    struct nested *nested_ptr = &nested_array[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) array1d[i] = i;
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 20; j++)
            array2d[i][j] = i * 20.0 + j;
    
    /* Initialize nested structure */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) nested_array[i].a[j] = i * 8 + j;
        for (int j = 0; j < 4; j++) nested_array[i].b[j] = i * 4.0 + j;
        nested_array[i].next = (i < 4) ? &nested_array[i + 1] : NULL;
    }
    
    /* Result variables for outputs */
    int out1, out2, out3;
    double out_d1, out_d2;
    __m128i out_vec;
    
    /* =====================================================================
       BLOCK A: Register Class Conflict Reload
       Force integer to float register reload and vice versa
       ===================================================================== */
    {
        int temp_int = 12345;
        float temp_float = 678.901f;
        double temp_double = 123.456;
        
        /* Mixed register class constraints causing reloads */
        asm volatile (
            /* Integer in floating-point register constraint */
            "mov %[fi], %[intval]\n\t"
            "cvtsi2ss %[fi], %[fout]\n\t"
            /* Float in integer register constraint */
            "movd %[fval], %[io]\n\t"
            "addl $1, %[io]"
            : [fout] "=f" (temp_float), [io] "=r" (temp_int)
            : [intval] "r" (temp_int), [fi] "f" (temp_float), [fval] "f" (temp_float)
            : "cc"
        );
        
        /* Use results to prevent dead code elimination */
        int_var += temp_int;
        float_var += temp_float;
    }
    
    /* =====================================================================
       BLOCK B: Complex Address Reload with Non-trivial Addressing Modes
       ===================================================================== */
    {
        int i = 3, j = 7, k = 11;
        long long offset = 1000;
        
        /* Complex address computation that may need reloading */
        asm volatile (
            /* Multiple memory references with complex addressing */
            "mov (%[addr1]), %%eax\n\t"
            "add (%[addr2]), %%eax\n\t"
            "mov %%eax, %[result]"
            : [result] "=r" (out1)
            : [addr1] "r" (&multi_array[i][j][k]), 
              [addr2] "r" (&array1d[i * 20 + j * 5 + k])
            : "%eax", "memory", "cc"
        );
        
        /* Even more complex addressing with structure chains */
        int idx1 = 2, idx2 = 3;
        asm volatile (
            "mov (%[base], %[idx1], 8), %%rax\n\t"      /* scaled index */
            "mov 16(%%rax, %[idx2], 4), %[res]\n\t"     /* base + scaled index + displacement */
            : [res] "=r" (out2)
            : [base] "r" (nested_array), 
              [idx1] "r" ((long)idx1),
              [idx2] "r" ((long)idx2)
            : "%rax", "cc"
        );
        
        /* Pointer chasing with multiple indirections */
        struct nested *current = nested_ptr;
        for (int count = 0; count < 3 && current; count++) {
            asm volatile (
                "mov 0(%[ptr]), %[val]\n\t"
                : [val] "=r" (out3)
                : [ptr] "r" (&current->a[count])
                : "memory"
            );
            current = current->next;
            int_var += out3;
        }
    }
    
    /* =====================================================================
       BLOCK C: Early-Clobber Multiple Outputs with Input Reuse
       ===================================================================== */
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int early1, early2;
        
        /* Early clobber forces input reloads */
        asm volatile (
            /* out1 is early-clobber, written before all inputs read */
            "mov %[in1], %[out1]\n\t"      /* out1 clobbered early */
            "imul %[in2], %[out1]\n\t"     /* uses out1 (early clobber) */
            "add %[in1], %[out2]\n\t"      /* out2 not early clobber */
            "add %[out1], %[out2]\n\t"     /* uses both outputs */
            "add %[in3], %[out2]"
            : [out1] "=&r" (early1),  /* Early clobber - allocated separate reg */
              [out2] "=r" (early2)    /* Normal output - may share with inputs */
            : [in1] "r" (in1), 
              [in2] "r" (in2), 
              [in3] "r" (in3)
            : "cc"
        );
        
        /* Multiple early clobbers with overlapping constraints */
        double din1 = 1.5, din2 = 2.5, din3 = 3.5;
        double dout1, dout2;
        
        asm volatile (
            /* Two early-clobber outputs with input reuse */
            "movsd %[din1], %[dout1]\n\t"
            "mulsd %[din2], %[dout1]\n\t"
            "movsd %[din3], %[dout2]\n\t"
            "addsd %[dout1], %[dout2]\n\t"
            "addsd %[din1], %[dout2]"
            : [dout1] "=&f" (dout1),  /* Early clobber FP */
              [dout2] "=&f" (dout2)   /* Another early clobber FP */
            : [din1] "f" (din1),
              [din2] "f" (din2),
              [din3] "f" (din3)
            : "cc"
        );
        
        out_d1 = dout1;
        out_d2 = dout2;
        int_var += early1 + early2;
    }
    
    /* =====================================================================
       BLOCK D: Secondary Reload Patterns and Varied Machine Modes
       ===================================================================== */
    {
        /* Different machine modes: SImode, DImode, SFmode, DFmode */
        int32_t si_val = 0x12345678;
        int64_t di_val = 0x123456789ABCDEF0LL;
        float sf_val = 123.456f;
        double df_val = 789.012;
        
        /* Mixed mode operations forcing mode conversions */
        asm volatile (
            /* 32-bit to 64-bit extension needing temporary */
            "movslq %[si], %%rax\n\t"
            "add %[di], %%rax\n\t"
            "mov %%rax, %[out]"
            : [out] "=r" (di_val)
            : [si] "r" (si_val), [di] "r" (di_val)
            : "%rax", "cc"
        );
        
        /* Float/double conversions */
        asm volatile (
            "cvtss2sd %[sf], %[df]\n\t"
            "addsd %[dfin], %[df]"
            : [df] "=f" (df_val)
            : [sf] "f" (sf_val), [dfin] "f" (df_val)
            : "cc"
        );
        
        /* Vector operations with different modes */
        __m128i vec1 = _mm_set_epi32(1, 2, 3, 4);
        __m128i vec2 = _mm_set_epi32(5, 6, 7, 8);
        
        /* Vector operation that might need secondary reload */
        asm volatile (
            "paddd %[v2], %[v1]"
            : [v1] "+x" (vec1)
            : [v2] "x" (vec2)
            : "cc"
        );
        
        out_vec = vec1;
        double_var = df_val;
    }
    
    /* =====================================================================
       BLOCK E: High Register Pressure and Spilling
       ===================================================================== */
    {
        /* Many live variables to increase register pressure */
        VOLATILE_VAR int r0 = 0, r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
        VOLATILE_VAR int r6 = 6, r7 = 7, r8 = 8, r9 = 9, r10 = 10;
        VOLATILE_VAR double d0 = 0.0, d1 = 1.0, d2 = 2.0, d3 = 3.0;
        
        /* Inline asm using many registers */
        asm volatile (
            /* Use all integer variables */
            "add %[v1], %[v0]\n\t"
            "add %[v2], %[v0]\n\t"
            "add %[v3], %[v0]\n\t"
            "add %[v4], %[v0]\n\t"
            "add %[v5], %[v0]\n\t"
            "add %[v6], %[v0]\n\t"
            "add %[v7], %[v0]\n\t"
            "add %[v8], %[v0]\n\t"
            "add %[v9], %[v0]\n\t"
            "add %[v10], %[v0]"
            : [v0] "+r" (r0)
            : [v1] "r" (r1), [v2] "r" (r2), [v3] "r" (r3),
              [v4] "r" (r4), [v5] "r" (r5), [v6] "r" (r6),
              [v7] "r" (r7), [v8] "r" (r8), [v9] "r" (r9),
              [v10] "r" (r10)
            : "cc"
        );
        
        /* Mixed float and integer use */
        asm volatile (
            "cvtsi2sd %[iv], %[dv]\n\t"
            "addsd %[din], %[dv]"
            : [dv] "+f" (d0)
            : [iv] "r" (r0), [din] "f" (d1)
            : "cc"
        );
        
        int_var += r0;
        double_var += d0;
    }
    
    /* =====================================================================
       Compute checksum to prevent optimization and verify execution
       ===================================================================== */
    int checksum = int_var + long_var + (int)float_var + (int)double_var;
    checksum += out1 + out2 + out3;
    checksum += (int)out_d1 + (int)out_d2;
    
    /* Access global to ensure it's used */
    checksum += global_int;
    
    /* Print to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
