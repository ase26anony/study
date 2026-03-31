/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Force noinline to prevent optimization */
#define NOINLINE __attribute__((noinline))

/* Global variables to increase register pressure */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile __m128i global_vec = {0};

NOINLINE int compute_checksum(int a, int b, int c, int d, 
                               double da, double db, 
                               __m128i va, __m128i vb) {
    int sum = a + b + c + d;
    sum += (int)da + (int)db;
    sum += ((int*)&va)[0] + ((int*)&vb)[0];
    return sum;
}

int main(void) {
    /* Diverse variable declarations with different types and sizes */
    int int_var1 = 1234, int_var2 = 5678, int_var3 = 9012;
    long long_var1 = 0x12345678, long_var2 = 0x9ABCDEF0;
    float float_var1 = 1.234f, float_var2 = 5.678f;
    double double_var1 = 3.14159, double_var2 = 2.71828;
    __m128i vec_var1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec_var2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* Arrays for complex addressing */
    int multi_array[10][20][30];
    double dbl_array[100][50];
    volatile int* volatile_ptr = (volatile int*)&global_counter;
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 20; j++) {
            for (int k = 0; k < 30; k++) {
                multi_array[i][j][k] = i * 400 + j * 20 + k;
            }
        }
    }
    
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            dbl_array[i][j] = i * 100.0 + j * 2.0;
        }
    }
    
    /* BLOCK A: Register Class Conflict */
    /* Force integer to float register reload */
    {
        int temp_int = int_var1 + 1000;
        double temp_double;
        
        /* This asm requires an integer in a floating-point register */
        asm volatile (
            "mov %1, %%eax\n\t"
            "cvtsi2sd %%eax, %0\n\t"
            : "=f" (temp_double)    /* Output in floating-point register */
            : "r" (temp_int)        /* Input in general-purpose register */
            : "%eax", "memory"
        );
        
        double_var1 = temp_double;
        global_counter++;
    }
    
    /* BLOCK B: Complex Address Reload with Multiple Indexing */
    {
        int idx1 = int_var2 % 10;
        int idx2 = int_var3 % 20;
        int idx3 = long_var1 % 30;
        int result;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl (%[addr]), %0\n\t"
            : "=r" (result)
            : [addr] "r" (&multi_array[idx1][idx2][idx3])
            : "memory"
        );
        
        /* Even more complex address calculation inline */
        int complex_idx = (idx1 * 400 + idx2 * 20 + idx3) * 2;
        asm volatile (
            "addl $100, %0\n\t"
            : "+r" (complex_idx)
            :
            : "cc"
        );
        
        int_var1 = result + complex_idx;
    }
    
    /* BLOCK C: Early-Clobber Multiple Outputs */
    {
        int out1, out2, out3;
        int in1 = int_var1 * 2;
        int in2 = int_var2 + 100;
        int in3 = int_var3 - 50;
        
        /* Early-clobber on out2 forces separate register allocation */
        asm volatile (
            "movl %2, %0\n\t"      /* out1 = in1 */
            "imull %3, %0\n\t"     /* out1 *= in2 */
            "movl %3, %1\n\t"      /* out2 = in2 (early clobbered) */
            "addl %4, %1\n\t"      /* out2 += in3 */
            "movl %0, %3\n\t"      /* Modify in2 (demonstrate why early-clobber needed) */
            "subl %1, %3\n\t"      /* in2 -= out2 */
            "movl %3, %2\n\t"      /* in1 = modified in2 */
            : "=&r" (out1), "=&r" (out2), "+r" (in1), "+r" (in2)
            : "r" (in3)
            : "cc"
        );
        
        int_var2 = out1 + out2 + in1 + in2;
    }
    
    /* BLOCK D: Secondary Reload Pattern with 64-bit constant */
    {
        long long big_const = 0x123456789ABCDEF0LL;
        long long result_ll;
        
        /* Moving 64-bit constant may require secondary reload on some arches */
        asm volatile (
            "movq %1, %0\n\t"
            "addq $0x1111111111111111, %0\n\t"
            : "=r" (result_ll)
            : "r" (big_const)
            : "cc"
        );
        
        /* Try with different register classes */
        double dbl_result;
        asm volatile (
            "movq %1, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=m" (dbl_result)
            : "r" (big_const)
            : "%rax", "memory"
        );
        
        long_var1 = result_ll;
        global_double = dbl_result;
    }
    
    /* BLOCK E: Mixed Mode Reloads (different data widths) */
    {
        char char_var = 65;
        short short_var = 32000;
        int int_result;
        
        /* Mixed width operations forcing mode conversions */
        asm volatile (
            "movsbl %1, %0\n\t"
            "addw %2, %w0\n\t"
            : "=r" (int_result)
            : "r" (char_var), "r" (short_var)
            : "cc"
        );
        
        /* Floating point with different precision */
        float float_result;
        asm volatile (
            "cvtss2sd %1, %%xmm0\n\t"
            "cvtsd2ss %%xmm0, %0\n\t"
            : "=x" (float_result)
            : "x" (float_var1)
            : "%xmm0"
        );
        
        int_var3 = int_result;
        float_var2 = float_result;
    }
    
    /* BLOCK F: High Register Pressure with Many Clobbers */
    {
        int a = int_var1, b = int_var2, c = int_var3;
        int d = long_var1, e = long_var2;
        
        /* Clobber many registers to force spills and reloads */
        asm volatile (
            "movl %0, %%eax\n\t"
            "movl %1, %%ebx\n\t"
            "movl %2, %%ecx\n\t"
            "movl %3, %%edx\n\t"
            "movl %4, %%esi\n\t"
            "addl %%ebx, %%eax\n\t"
            "addl %%ecx, %%eax\n\t"
            "addl %%edx, %%eax\n\t"
            "addl %%esi, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (a)
            : "r" (b), "r" (c), "r" (d), "r" (e)
            : "%eax", "%ebx", "%ecx", "%edx", "%esi", "cc", "memory"
        );
        
        int_var1 = a;
    }
    
    /* BLOCK G: Vector/SIMD Reloads */
    {
        __m128i vec_temp;
        
        /* Vector operation that might need reload */
        asm volatile (
            "paddd %1, %0\n\t"
            "pslld $2, %0\n\t"
            : "=x" (vec_temp)
            : "x" (vec_var1), "0" (vec_var2)
            : "cc"
        );
        
        /* Store vector to complex memory address */
        int vec_idx = int_var1 % 100;
        asm volatile (
            "movdqu %1, (%0)\n\t"
            :
            : "r" (&dbl_array[vec_idx][0]), "x" (vec_temp)
            : "memory"
        );
        
        vec_var1 = vec_temp;
    }
    
    /* BLOCK H: Pointer Chain with Offset */
    {
        struct Node {
            int value;
            struct Node* next;
        };
        
        /* Simulate pointer chasing that needs address reloads */
        volatile int* ptr1 = (volatile int*)&int_var1;
        volatile int* ptr2 = (volatile int*)&int_var2;
        volatile int* ptr3 = (volatile int*)&int_var3;
        
        int chain_result;
        asm volatile (
            "movl (%1), %%eax\n\t"
            "addl (%2), %%eax\n\t"
            "addl (%3), %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (chain_result)
            : "r" (ptr1), "r" (ptr2), "r" (ptr3)
            : "%eax", "memory"
        );
        
        /* Complex pointer arithmetic */
        volatile int* complex_ptr = ptr1 + (int_var1 % 10);
        asm volatile (
            "movl $999, (%0)\n\t"
            :
            : "r" (complex_ptr)
            : "memory"
        );
        
        long_var2 = chain_result;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    int checksum = compute_checksum(
        int_var1, int_var2, int_var3, (int)long_var1,
        double_var1, double_var2,
        vec_var1, vec_var2
    );
    
    /* Use all variables one more time */
    global_counter += checksum;
    volatile_ptr = (volatile int*)&checksum;
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
