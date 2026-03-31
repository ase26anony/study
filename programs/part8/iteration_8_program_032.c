/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>  /* For vector types */

/* Force no optimization on specific variables */
#define NOOPT __attribute__((optimize("O0")))

/* Complex structure to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_int = 42;
double global_double = 3.14159;
float global_float_array[16];
__m128i global_vector;

NOOPT int main(void) {
    /* Diverse variable declarations with different types and sizes */
    volatile int int_var = 1;
    volatile long long_var = 2;
    volatile float float_var = 3.0f;
    volatile double double_var = 4.0;
    volatile __m128i vec_var;
    volatile int *int_ptr = &int_var;
    volatile double *double_ptr = &double_var;
    
    /* Arrays for complex addressing */
    int multi_array[8][8];
    double complex_array[4][4][4];
    struct nested nested_array[4];
    struct nested *nested_ptr = &nested_array[0];
    
    /* Initialize arrays to prevent optimization */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            multi_array[i][j] = i * 8 + j;
        }
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                complex_array[i][j][k] = i * 16.0 + j * 4.0 + k;
            }
        }
        nested_array[i].next = (i < 3) ? &nested_array[i + 1] : NULL;
    }
    
    /* Variables for output from asm */
    int out1, out2, out3;
    double out_double;
    float out_float;
    long long out_ll;
    
    /* ============================================
       BLOCK A: Register Class Conflict Reload
       ============================================ */
    {
        /* Force integer into floating-point register */
        int temp_int = int_var + global_int;
        
        asm volatile (
            /* Request float register for integer value */
            "mov %[temp], %[outf]\n\t"
            : [outf] "=f" (out_float)      /* Output in float register */
            : [temp] "r" (temp_int)        /* Input in general register */
            : /* No clobbers */
        );
        
        /* Use the result to prevent optimization */
        float_var += out_float;
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       ============================================ */
    {
        /* Complex addressing mode that may need reloading */
        int i = int_var & 0x7;
        int j = long_var & 0x7;
        int k = (int_var + long_var) & 0x3;
        
        /* This address computation is complex and may need a reload */
        volatile double *addr = &complex_array[i][j][k];
        
        asm volatile (
            /* Memory constraint with complex address */
            "movq (%[addr]), %[out]\n\t"
            : [out] "=r" (out_ll)          /* Output */
            : [addr] "r" (addr)            /* Address in register */
            : "memory"
        );
        
        /* Chain pointer dereference for more complex addressing */
        double_ptr = (volatile double *)out_ll;
        for (int idx = 0; idx < 2; idx++) {
            if (nested_ptr) {
                nested_ptr = nested_ptr->next;
            }
        }
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       ============================================ */
    {
        int in1 = int_var * 2;
        int in2 = long_var * 3;
        int in3 = in1 + in2;
        
        asm volatile (
            /* Multiple outputs with early clobber */
            "mov %[in1], %[out1]\n\t"
            "add %[in2], %[out1]\n\t"      /* out1 = in1 + in2 */
            "mov %[out1], %[out2]\n\t"     /* out2 gets same value */
            "imul %[in3], %[out1]\n\t"     /* Modify out1 again */
            : [out1] "=&r" (out1),         /* Early clobber! */
              [out2] "=r" (out2)           /* Regular output */
            : [in1] "r" (in1),
              [in2] "r" (in2),
              [in3] "r" (in3)
            : "cc"                         /* Clobber condition codes */
        );
        
        /* Use outputs */
        int_var += out1 + out2;
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Pattern
       ============================================ */
    {
        /* Large immediate that might need secondary reload on some archs */
        long long large_imm = 0x123456789ABCDEF0LL;
        
        asm volatile (
            /* Pattern that often needs secondary reload */
            "mov %[imm], %[out]\n\t"
            "not %[out]\n\t"
            : [out] "=r" (out_ll)
            : [imm] "i" (0x12345678)       /* Immediate constraint */
            : /* No clobbers */
        );
        
        /* Another pattern: memory to vector with possible secondary reload */
        __m128i temp_vec;
        asm volatile (
            "movdqu %[mem], %[vec]\n\t"
            : [vec] "=x" (temp_vec)        /* XMM register */
            : [mem] "m" (global_vector)    /* Memory operand */
            : /* No clobbers */
        );
        vec_var = temp_vec;
    }
    
    /* ============================================
       BLOCK E: Mixed Mode Reloads
       ============================================ */
    {
        /* Different machine modes in same asm */
        char char_var = 'A';
        short short_var = 0x1234;
        
        asm volatile (
            /* Mixed size operations */
            "mov %[char], %%al\n\t"
            "mov %[short], %%bx\n\t"
            "add %%bx, %%ax\n\t"
            "mov %%ax, %[out]\n\t"
            : [out] "=r" (out3)
            : [char] "r" ((int)char_var),
              [short] "r" ((int)short_var)
            : "ax", "bx"                   /* Explicit register clobbers */
        );
    }
    
    /* ============================================
       BLOCK F: High Register Pressure
       ============================================ */
    {
        /* Many live variables to force spills and reloads */
        int r1 = int_var + 1;
        int r2 = r1 * 2;
        int r3 = r2 + long_var;
        int r4 = r3 ^ 0xFF;
        int r5 = r4 << 2;
        int r6 = r5 - out1;
        int r7 = r6 & 0xFFFF;
        int r8 = r7 | out2;
        int r9 = r8 / 2;
        int r10 = r9 + out3;
        
        /* Use all in a complex asm */
        asm volatile (
            "add %[a], %[b]\n\t"
            "sub %[c], %[b]\n\t"
            "imul %[d], %[b]\n\t"
            "xor %[e], %[b]\n\t"
            : [b] "+r" (r2)                /* Read-write operand */
            : [a] "r" (r1),
              [c] "r" (r3),
              [d] "r" (r4),
              [e] "r" (r5)
            : "cc"
        );
        
        /* Chain computations to keep variables live */
        r6 = r2 + r6;
        r7 = r3 + r7;
        r8 = r4 + r8;
        r9 = r5 + r9;
        r10 = r6 + r10;
        
        /* Final use */
        int_var = r10;
    }
    
    /* ============================================
       Compute checksum to prevent optimization
       ============================================ */
    unsigned long long checksum = 0;
    
    checksum += int_var;
    checksum += long_var;
    checksum += *(unsigned int*)&float_var;
    checksum += *(unsigned long long*)&double_var;
    checksum += out1;
    checksum += out2;
    checksum += out3;
    checksum += out_ll & 0xFFFFFFFF;
    checksum += (out_ll >> 32) & 0xFFFFFFFF;
    
    /* Add array contents */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += multi_array[i][j];
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
