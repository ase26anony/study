/* reload_test.c - Test program to trigger multiple reload scenarios in GCC */

#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Force noinline to prevent optimization */
#define NOINLINE __attribute__((noinline))

/* Global variables to increase register pressure */
volatile int global_int = 42;
volatile double global_double = 3.14159;
volatile float global_float = 2.71828f;

/* Complex structure to force address computations */
struct nested {
    int data[8];
    struct nested *next;
    double values[4];
};

NOINLINE int trigger_reloads(void) {
    /* Declare diverse variables to force different machine modes */
    int int_var = 123;
    long long_var = 456LL;
    float float_var = 1.234f;
    double double_var = 5.678;
    __m128i vec_var = _mm_set_epi32(1, 2, 3, 4);
    
    /* Arrays for complex addressing */
    int multi_array[16][8];
    double dbl_array[32];
    struct nested complex_struct[4];
    
    /* Pointers for address computations */
    int *ptr1 = &int_var;
    long *ptr2 = &long_var;
    struct nested *struct_ptr = &complex_struct[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            multi_array[i][j] = i * 8 + j;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        dbl_array[i] = i * 0.1;
    }
    
    /* ===== BLOCK A: Register Class Conflict ===== */
    /* Force integer to be reloaded into floating-point register */
    {
        int input = 999;
        double output;
        
        /* This asm requires integer in floating-point register - will trigger reload */
        asm volatile (
            /* Convert integer to double using floating-point register */
            "cvtsi2sd %1, %0\n\t"
            : "=f" (output)
            : "r" (input)
            : 
        );
        
        double_var += output;
    }
    
    /* ===== BLOCK B: Complex Address Reload ===== */
    /* Force complex addressing mode that needs reloading */
    {
        int i = 3, j = 5;
        int result;
        
        /* Complex array addressing that may not fit in one addressing mode */
        asm volatile (
            "movl (%[addr]), %[res]\n\t"
            : [res] "=r" (result)
            : [addr] "r" (&multi_array[i*2 + 1][j*3 % 8])
            : "memory"
        );
        
        int_var += result;
    }
    
    /* ===== BLOCK C: Early-Clobber Multiple Outputs ===== */
    /* Force reloads due to early-clobber constraints */
    {
        int in1 = 111, in2 = 222, in3 = 333;
        int out1, out2;
        
        /* Early-clobber on out2 forces it to not overlap with inputs */
        asm volatile (
            "movl %2, %0\n\t"      /* out1 = in1 */
            "addl %3, %0\n\t"      /* out1 += in2 */
            "movl %0, %1\n\t"      /* out2 = out1 */
            "imull %4, %1\n\t"     /* out2 *= in3 */
            : "=&r" (out1), "=&r" (out2)  /* Both early-clobber */
            : "r" (in1), "r" (in2), "r" (in3)
            : 
        );
        
        long_var += out1 + out2;
    }
    
    /* ===== BLOCK D: Secondary Reload Pattern ===== */
    /* Force secondary reloads for vector operations */
    {
        __m128i vec_input = _mm_set_epi32(5, 6, 7, 8);
        __m128i vec_output;
        int temp;
        
        /* Pattern that might require secondary reload on some architectures */
        asm volatile (
            /* Move vector to temporary, modify, move back */
            "movd %1, %0\n\t"      /* Extract element to temp */
            "addl $100, %0\n\t"    /* Modify in integer register */
            "movd %0, %2\n\t"      /* Move back to vector */
            : "=r" (temp), "+x" (vec_output)
            : "x" (vec_input)
            : 
        );
        
        /* Use the result */
        int* vptr = (int*)&vec_output;
        float_var += vptr[0];
    }
    
    /* ===== BLOCK E: Memory Constraints with Complex Addressing ===== */
    /* Force address reloads for memory operands */
    {
        double result;
        int idx1 = 2, idx2 = 3;
        
        /* Complex addressing in memory constraint */
        asm volatile (
            "movsd (%[mem]), %[res]\n\t"
            : [res] "=x" (result)
            : [mem] "r" (&dbl_array[idx1 * 4 + idx2 * 2])
            : "memory"
        );
        
        double_var *= result;
    }
    
    /* ===== BLOCK F: Multiple Constraint Alternatives ===== */
    /* Force reload by using constraint that's hard to satisfy */
    {
        int value = 777;
        int output;
        
        /* Try multiple constraints, some hard to satisfy */
        asm volatile (
            "movl %1, %0\n\t"
            "addl $111, %0\n\t"
            : "=r,r,m" (output)  /* Multiple constraints */
            : "0,r,i" (value)    /* Matching constraints */
            : 
        );
        
        int_var ^= output;
    }
    
    /* ===== BLOCK G: Structure Pointer Chain ===== */
    /* Force complex address computation through structure */
    {
        int result;
        
        /* Chain structure pointer accesses */
        asm volatile (
            "movl (%[ptr]), %[res]\n\t"
            : [res] "=r" (result)
            : [ptr] "r" (&struct_ptr->next->data[3])
            : "memory"
        );
        
        long_var -= result;
    }
    
    /* ===== BLOCK H: Mixed Register Classes ===== */
    /* Force moves between different register classes */
    {
        double dbl_input = 9.876;
        int int_output;
        
        /* Move from floating-point to integer register */
        asm volatile (
            "movq %1, %%rax\n\t"   /* Move double to RAX */
            "shrq $32, %%rax\n\t"  /* Get high 32 bits */
            "movl %%eax, %0\n\t"   /* Store to output */
            : "=r" (int_output)
            : "x" (dbl_input)
            : "%rax"
        );
        
        float_var += int_output;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = int_var + long_var + (int)float_var + (int)double_var;
    
    /* Use all variables to ensure they're live */
    checksum += ((int*)&vec_var)[0];
    checksum += multi_array[0][0];
    checksum += (int)dbl_array[0];
    checksum += complex_struct[0].data[0];
    
    return checksum;
}

int main(void) {
    int result = trigger_reloads();
    
    /* Print result to prevent optimization */
    printf("Reload test checksum: %d\n", result);
    
    /* Also use globals */
    printf("Globals: %d %f %f\n", global_int, global_double, global_float);
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
