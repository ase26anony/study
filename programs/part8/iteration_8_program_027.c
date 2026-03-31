/* reload_test.c - Test program to trigger multiple reload scenarios in GCC */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>  /* For vector types */

/* Force no optimization on specific variables */
#define VOLATILE_VAR volatile

/* Complex structure to force address computations */
struct nested {
    int data[8][8];
    double fp_data[4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_array[256];
double global_fp[128];
__m128i global_vec[64];

/* Function to create complex addressing patterns */
int complex_address(int i, int j, int k) {
    /* Multi-dimensional array with non-constant indices */
    int multi_dim[16][16][16];
    
    /* Force address computation that may need reloading */
    int result;
    VOLATILE_VAR int *ptr;
    
    /* Complex address computation */
    ptr = &multi_dim[i][j][k];
    
    /* Inline asm with memory constraint and complex address */
    asm volatile (
        "movl (%[mem]), %[out]\n\t"
        : [out] "=r" (result)
        : [mem] "m" (*ptr)
        : "memory"
    );
    
    return result;
}

/* Function to trigger register class conflicts */
double register_class_conflict(double x, int y) {
    double result;
    int temp;
    
    /* Force integer into floating-point register */
    asm volatile (
        "mov %[in_int], %%eax\n\t"
        "cvtsi2sd %%eax, %[out]\n\t"
        : [out] "=f" (result)
        : [in_int] "r" (y)
        : "%eax"
    );
    
    /* Force floating-point value into integer register */
    asm volatile (
        "movq %[in_fp], %%rax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=r" (temp)
        : [in_fp] "f" (x)
        : "%rax"
    );
    
    return result + temp;
}

/* Function with early-clobber and multiple outputs */
void early_clobber_test(int a, int b, int c, int *out1, int *out2) {
    int tmp1, tmp2;
    
    /* Early-clobber: out2 is written before all inputs are consumed */
    asm volatile (
        "movl %[in1], %[out2]\n\t"      /* Early clobber of out2 */
        "addl %[in2], %[out2]\n\t"
        "imull %[in3], %[out2]\n\t"
        "movl %[out2], %[out1]\n\t"     /* Use out2 to compute out1 */
        "subl $1, %[out1]\n\t"
        : [out1] "=r" (tmp1), [out2] "=&r" (tmp2)  /* & = early clobber */
        : [in1] "r" (a), [in2] "r" (b), [in3] "r" (c)
        : "cc"
    );
    
    *out1 = tmp1;
    *out2 = tmp2;
}

/* Function to trigger secondary reloads */
void secondary_reload_test(__m128i *vec, int64_t imm) {
    __m128i result;
    
    /* Pattern that often requires secondary reloads:
       Moving a 64-bit immediate to vector register */
    asm volatile (
        "movq %[imm], %%rax\n\t"        /* First reload: imm -> GPR */
        "movq %%rax, %[out]\n\t"        /* Second reload: GPR -> vector */
        : [out] "=x" (result)
        : [imm] "r" (imm)
        : "%rax"
    );
    
    /* Store result */
    *vec = result;
}

/* Function with mixed mode reloads */
void mixed_mode_reloads(void) {
    int8_t byte_var = 127;
    int16_t short_var = 32767;
    int32_t int_var = 2147483647;
    int64_t long_var = 9223372036854775807LL;
    float float_var = 3.14159f;
    double double_var = 2.718281828459045;
    __m128i vec_var;
    
    /* Force reloads with different machine modes */
    asm volatile (
        "movb %[byte], %%al\n\t"
        "movw %[short], %%ax\n\t"
        "movl %[int], %%eax\n\t"
        "movq %[long], %%rax\n\t"
        "movss %[float], %%xmm0\n\t"
        "movsd %[double], %%xmm1\n\t"
        : 
        : [byte] "r" (byte_var),
          [short] "r" (short_var),
          [int] "r" (int_var),
          [long] "r" (long_var),
          [float] "f" (float_var),
          [double] "f" (double_var)
        : "%rax", "%xmm0", "%xmm1", "cc"
    );
}

/* Function with high register pressure */
void high_register_pressure(void) {
    /* Many live variables to force spills and reloads */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    double f1 = 1.1, f2 = 2.2, f3 = 3.3, f4 = 4.4;
    __m128i vec1, vec2, vec3, vec4;
    
    /* Complex computation using all variables */
    asm volatile (
        /* Integer operations */
        "addl %[v2], %[v1]\n\t"
        "subl %[v3], %[v1]\n\t"
        "imull %[v4], %[v1]\n\t"
        "addl %[v5], %[v2]\n\t"
        "subl %[v6], %[v2]\n\t"
        "imull %[v7], %[v2]\n\t"
        "addl %[v8], %[v3]\n\t"
        
        /* Floating point operations */
        "addsd %[f2], %[f1]\n\t"
        "subsd %[f3], %[f1]\n\t"
        "mulsd %[f4], %[f1]\n\t"
        
        /* Output constraints */
        : [v1] "+r" (v1), [v2] "+r" (v2), [v3] "+r" (v3),
          [f1] "+f" (f1)
        : [v4] "r" (v4), [v5] "r" (v5), [v6] "r" (v6),
          [v7] "r" (v7), [v8] "r" (v8),
          [f2] "f" (f2), [f3] "f" (f3), [f4] "f" (f4)
        : "cc", "memory"
    );
    
    /* Use results to prevent optimization */
    global_array[0] = v1 + v2 + v3;
    global_fp[0] = f1;
}

/* Main function orchestrating all tests */
int main(void) {
    int result = 0;
    int out1, out2;
    struct nested data[4];
    __m128i vec_result;
    
    /* Initialize data */
    memset(data, 0, sizeof(data));
    for (int i = 0; i < 4; i++) {
        data[i].next = &data[(i + 1) % 4];
    }
    
    printf("Starting reload tests...\n");
    
    /* Test 1: Complex address reloads */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result += complex_address(i, j, i * j % 4);
        }
    }
    
    /* Test 2: Register class conflicts */
    double fp_result = 0.0;
    for (int i = 0; i < 10; i++) {
        fp_result += register_class_conflict(3.14 * i, i * 100);
    }
    result += (int)fp_result;
    
    /* Test 3: Early-clobber with multiple outputs */
    for (int i = 0; i < 5; i++) {
        early_clobber_test(i, i*2, i*3, &out1, &out2);
        result += out1 + out2;
    }
    
    /* Test 4: Secondary reload patterns */
    for (int64_t i = 0; i < 5; i++) {
        secondary_reload_test(&vec_result, 0x123456789ABCDEF0LL + i);
        global_vec[i % 64] = vec_result;
    }
    
    /* Test 5: Mixed mode reloads */
    mixed_mode_reloads();
    
    /* Test 6: High register pressure */
    high_register_pressure();
    
    /* Test 7: Pointer chasing with complex addressing */
    struct nested *ptr = &data[0];
    for (int i = 0; i < 8; i++) {
        /* Complex addressing through structure pointer */
        int index = (i * 7) % 8;
        asm volatile (
            "movl (%[base], %[index], 4), %%eax\n\t"
            "addl %%eax, %[sum]\n\t"
            : [sum] "+r" (result)
            : [base] "r" (ptr->data[index]),
              [index] "r" (index)
            : "%eax", "cc", "memory"
        );
        ptr = ptr->next;
    }
    
    /* Test 8: Memory operand with displacement too large */
    /* Force use of index register for large displacement */
    asm volatile (
        "leaq global_array(%%rip), %%rax\n\t"
        "movl 1024(%%rax), %%ebx\n\t"  /* Large displacement */
        "addl %%ebx, %[res]\n\t"
        : [res] "+r" (result)
        : 
        : "%rax", "%rbx", "cc", "memory"
    );
    
    /* Final checksum to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Use all global arrays to prevent optimization */
    for (int i = 0; i < 256; i++) {
        result ^= global_array[i];
    }
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
