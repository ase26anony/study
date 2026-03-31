/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force no optimization on specific variables */
#define VOL(var) (*(volatile typeof(var)*)&(var))

/* Complex structure to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_int = 42;
double global_double = 3.14159;
__m128i global_vec = {0};
struct nested global_struct = {{0}, {0}, NULL};

int main(void) {
    /* Diverse variable declarations with different types and modes */
    int int_var = 1;
    long long_var = 2;
    float float_var = 3.0f;
    double double_var = 4.0;
    __m128i vec_var = _mm_set_epi32(1, 2, 3, 4);
    __m128 vec_float = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Arrays for complex addressing */
    int array_2d[16][16];
    double dbl_array[32];
    struct nested nested_array[8];
    
    /* Pointers for address computations */
    int *ptr1 = &int_var;
    double *ptr2 = &double_var;
    struct nested *ptr3 = &nested_array[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array_2d[i][j] = i * 16 + j;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        dbl_array[i] = i * 0.5;
    }
    
    for (int i = 0; i < 8; i++) {
        nested_array[i].next = (i < 7) ? &nested_array[i+1] : NULL;
    }
    
    int result = 0;
    
    /* BLOCK A: Register class conflict - integer in floating-point register */
    {
        int input = 12345;
        double output;
        
        /* Force reload by requesting integer in floating-point register */
        asm volatile (
            "mov %1, %%eax\n\t"          /* Load integer into eax */
            "cvtsi2sd %%eax, %0\n\t"     /* Convert to double, uses xmm register */
            : "=f" (output)              /* Output in floating-point register */
            : "r" (input)                /* Input in general-purpose register */
            : "%eax", "memory"
        );
        
        VOL(output) = output;
        result += (int)output;
    }
    
    /* BLOCK B: Complex address reload with multi-dimensional array */
    {
        int i = 5, j = 10;
        int temp;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r" (temp)
            : "m" (array_2d[i*2][j*3])   /* Complex address computation */
            : "memory"
        );
        
        VOL(temp) = temp;
        result += temp;
    }
    
    /* BLOCK C: Early-clobber with multiple outputs */
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int out1, out2, out3;
        
        /* Early-clobber forces separate register for out2 */
        asm volatile (
            "addl %3, %0\n\t"            /* out1 = in1 + in2 */
            "imull %4, %1\n\t"           /* out2 = in2 * in3 (early clobber) */
            "subl %5, %2\n\t"            /* out3 = in3 - in1 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)
            : "r" (in2), "r" (in3), "r" (in1), "0" (in1), "2" (in3)
            : "cc"
        );
        
        VOL(out1) = out1; VOL(out2) = out2; VOL(out3) = out3;
        result += out1 + out2 + out3;
    }
    
    /* BLOCK D: Secondary reload pattern - vector to integer transfer */
    {
        __m128i vec = _mm_set_epi32(10, 20, 30, 40);
        int extracted[4];
        
        /* This may require secondary reloads on some architectures */
        asm volatile (
            "movd %1, %0\n\t"
            "pextrd $1, %1, %2\n\t"
            "pextrd $2, %1, %3\n\t"
            "pextrd $3, %1, %4\n\t"
            : "=r" (extracted[0]), "=r" (extracted[1]), 
              "=r" (extracted[2]), "=r" (extracted[3])
            : "x" (vec)
            : "memory"
        );
        
        for (int k = 0; k < 4; k++) {
            VOL(extracted[k]) = extracted[k];
            result += extracted[k];
        }
    }
    
    /* BLOCK E: Memory operand with displacement too large */
    {
        double sum = 0.0;
        
        /* Force address reload by using large displacement */
        asm volatile (
            "addsd 1024(%1), %0\n\t"     /* Large displacement may need reload */
            "addsd 2048(%2), %0\n\t"
            : "+x" (sum)
            : "r" (dbl_array), "r" (dbl_array)
            : "memory"
        );
        
        VOL(sum) = sum;
        result += (int)sum;
    }
    
    /* BLOCK F: Multiple constraint alternatives */
    {
        int value = 999;
        int output;
        
        /* Multiple constraints can trigger different reload paths */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r,m" (output)            /* Can be register OR memory */
            : "r,m" (value)              /* Matching constraints */
            : "memory"
        );
        
        VOL(output) = output;
        result += output;
    }
    
    /* BLOCK G: Pointer chain addressing */
    {
        int chain_result;
        struct nested *current = &nested_array[0];
        
        /* Complex pointer chain may need address reload */
        asm volatile (
            "movl (%1), %0\n\t"
            : "=r" (chain_result)
            : "r" (&current->next->a[3])  /* Pointer chain */
            : "memory"
        );
        
        VOL(chain_result) = chain_result;
        result += chain_result;
    }
    
    /* BLOCK H: Mixed register classes in one asm */
    {
        double dbl_in = 2.5;
        float flt_out;
        int int_out;
        
        /* Mixing register classes increases reload complexity */
        asm volatile (
            "cvtsd2ss %1, %0\n\t"        /* Convert double to float */
            "cvtss2si %0, %2\n\t"        /* Convert float to int */
            : "=x" (flt_out), "=r" (int_out)
            : "x" (dbl_in)
            : "memory"
        );
        
        VOL(flt_out) = flt_out;
        VOL(int_out) = int_out;
        result += int_out;
    }
    
    /* Final computation to prevent optimization */
    result += global_int + (int)global_double;
    
    /* Use computed goto to force address reload */
    void *labels[] = { &&label1, &&label2, &&label3 };
    goto *labels[result % 3];
    
label1:
    result += 1;
    goto end;
label2:
    result += 2;
    goto end;
label3:
    result += 3;
    
end:
    printf("Result: %d\n", result);
    return result;
}
