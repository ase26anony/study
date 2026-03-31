/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For vector types */

/* Force no optimization on specific variables */
#define VOL(var) (*(volatile __typeof__(var)*)&(var))

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

int main(void) {
    /* Diverse variable declarations with different types and modes */
    int int_var = 1;
    long long ll_var = 2;
    float float_var = 3.0f;
    double double_var = 4.0;
    __m128i vec_var = _mm_set_epi32(1, 2, 3, 4);
    volatile int vol_int = 5;
    
    /* Arrays for complex addressing */
    int array_2d[10][20];
    double darray[100];
    struct nested nested_array[5];
    struct nested *nptr = &nested_array[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            array_2d[i][j] = i * 20 + j;
        }
    }
    
    for (int i = 0; i < 5; i++) {
        nested_array[i].next = &nested_array[(i + 1) % 5];
    }
    
    /* BLOCK A: Register class conflict reload */
    /* Force integer to float register reload */
    {
        int input = int_var + 1;
        double output;
        
        /* Request float register for integer computation */
        asm volatile (
            "/* Block A: Integer to float register reload */\n\t"
            "mov %1, %%eax\n\t"
            "cvtsi2sd %%eax, %0\n\t"
            : "=f" (output)        /* Output in floating-point register */
            : "r" (input)          /* Input in general-purpose register */
            : "%eax", "memory"
        );
        
        double_var += output;
        VOL(output);
    }
    
    /* BLOCK B: Complex address reload with multiple indexing */
    {
        int i = int_var % 10;
        int j = ll_var % 20;
        int k = vol_int % 5;
        int result;
        
        /* Complex addressing: array[i][j] + nested_array[k].a[i*2] */
        asm volatile (
            "/* Block B: Complex address reload */\n\t"
            "movl %[base], %%eax\n\t"
            "addl %[offset1], %%eax\n\t"
            "addl %[offset2], %%eax\n\t"
            "movl (%%eax), %[res]\n\t"
            : [res] "=r" (result)
            : [base] "r" (&array_2d[0][0]),
              [offset1] "r" (i * 20 * sizeof(int)),
              [offset2] "r" (j * sizeof(int)),
              "m" (array_2d)        /* Memory constraint to force address computation */
            : "%eax", "memory"
        );
        
        int_var += result;
        VOL(result);
    }
    
    /* BLOCK C: Early-clobber multiple outputs */
    {
        int in1 = int_var;
        int in2 = int_var * 2;
        int out1, out2;
        
        /* Early-clobber on out2 forces reloads */
        asm volatile (
            "/* Block C: Early-clobber multiple outputs */\n\t"
            "movl %2, %0\n\t"      /* out1 = in1 */
            "imull %3, %0\n\t"     /* out1 *= in2 - CLOBBERS %0 EARLY! */
            "movl %2, %1\n\t"      /* out2 = in1 */
            "addl %3, %1\n\t"      /* out2 += in2 */
            : "=&r" (out1), "=&r" (out2)  /* Both early-clobber */
            : "r" (in1), "r" (in2)
            : "cc"
        );
        
        ll_var += out1 + out2;
        VOL(out1); VOL(out2);
    }
    
    /* BLOCK D: Secondary reload pattern - vector to integer transfer */
    {
        __m128i vec = vec_var;
        int64_t low, high;
        
        /* Extract low and high parts - may require secondary reloads */
        asm volatile (
            "/* Block D: Vector extraction with secondary reloads */\n\t"
            "movq %2, %0\n\t"      /* Extract low 64 bits */
            "pextrq $1, %2, %1\n\t" /* Extract high 64 bits */
            : "=r" (low), "=r" (high)
            : "x" (vec)            /* Input in vector register */
            : "memory"
        );
        
        ll_var += low + high;
        VOL(low); VOL(high);
    }
    
    /* BLOCK E: Memory operand with displacement too large */
    {
        double result;
        int index = 1000;  /* Large index for potential displacement overflow */
        
        /* Force address reload due to large displacement */
        asm volatile (
            "/* Block E: Large displacement address reload */\n\t"
            "movsd %1, %0\n\t"
            : "=x" (result)
            : "m" (darray[index])  /* May need address reload if displacement too large */
            : "memory"
        );
        
        double_var += result;
        VOL(result);
    }
    
    /* BLOCK F: Multiple constraints with conflicting requirements */
    {
        int a = int_var;
        int b = vol_int;
        int c, d;
        
        /* Input used in multiple ways forcing different reloads */
        asm volatile (
            "/* Block F: Multiple constraint conflicts */\n\t"
            "lea (%2, %3, 2), %0\n\t"   /* c = a + b*2 */
            "imul %2, %3\n\t"           /* b *= a */
            "mov %3, %1\n\t"            /* d = b */
            : "=r" (c), "=r" (d)
            : "0" (a), "r" (b)          /* '0' means same as output 0 */
            : "cc"
        );
        
        int_var = c + d;
        VOL(c); VOL(d);
    }
    
    /* BLOCK G: Pointer chain forcing address reload */
    {
        int result;
        struct nested *ptr = nptr;
        
        /* Complex pointer chain: ptr->next->next->a[3] */
        asm volatile (
            "/* Block G: Pointer chain address reload */\n\t"
            "movq (%1), %%rax\n\t"      /* rax = ptr->next */
            "movq (%%rax), %%rax\n\t"   /* rax = rax->next */
            "movl 12(%%rax), %0\n\t"    /* result = rax->a[3] */
            : "=r" (result)
            : "r" (ptr)
            : "%rax", "memory"
        );
        
        vol_int += result;
        VOL(result);
    }
    
    /* BLOCK H: Mixed register classes in one asm */
    {
        int int_in = int_var;
        float float_in = float_var;
        double double_out;
        
        /* Mixed integer and float registers in same asm */
        asm volatile (
            "/* Block H: Mixed register classes */\n\t"
            "cvtsi2ss %1, %%xmm0\n\t"   /* Convert int to float */
            "addss %2, %%xmm0\n\t"      /* Add float input */
            "cvtss2sd %%xmm0, %0\n\t"   /* Convert to double output */
            : "=f" (double_out)
            : "r" (int_in), "f" (float_in)
            : "%xmm0", "memory"
        );
        
        double_var += double_out;
        VOL(double_out);
    }
    
    /* Compute checksum to prevent optimization */
    uint64_t checksum = 0;
    checksum += int_var;
    checksum += ll_var;
    checksum += *(uint64_t*)&double_var;
    checksum += vol_int;
    
    /* Use results to prevent dead code elimination */
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
