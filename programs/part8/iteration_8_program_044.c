/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For vector types */

/* Force no optimization on specific variables */
#define VOL(var) (*(volatile __typeof__(var)*)&(var))

/* Complex addressing structure */
struct nested {
    int data[8][8];
    double fp[4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_array[256];
double global_fp[128];
struct nested global_struct;

int main(void) {
    /* Diverse variable declarations to create register pressure */
    int i = 42, j = 17, k = 99, l = 123;
    long long big1 = 0x123456789ABCDEF0LL, big2 = 0xFEDCBA9876543210LL;
    float f1 = 3.14159f, f2 = 2.71828f;
    double d1 = 1.41421356, d2 = 1.73205080;
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* Pointer variables for complex addressing */
    int *ptr1 = &i, *ptr2 = &j;
    double *dptr1 = &d1, *dptr2 = &d2;
    struct nested stack_struct;
    struct nested *nptr = &stack_struct;
    
    /* Initialize structure with complex pattern */
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            stack_struct.data[x][y] = x * 100 + y;
        }
    }
    stack_struct.next = &global_struct;
    
    /* Array for complex indexing */
    int multi_array[16][16];
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            multi_array[x][y] = x * 16 + y;
        }
    }
    
    /* ============================================
       BLOCK A: Register Class Conflict Reload
       ============================================ */
    {
        int int_var = 0xDEADBEEF;
        double fp_var = 3.14159265358979;
        
        /* Force integer into floating-point register */
        asm volatile (
            /* Input: integer in general register, output: floating register */
            "mov %1, %%eax\n\t"          /* Load integer */
            "cvtsi2sd %%eax, %0\n\t"     /* Convert to double, needs FP reg */
            : "=f" (fp_var)              /* Output constraint: floating reg */
            : "r" (int_var)              /* Input constraint: general reg */
            : "%eax", "memory"
        );
        
        /* Use both to prevent optimization */
        VOL(fp_var) = fp_var;
        VOL(int_var) = int_var;
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       ============================================ */
    {
        /* Complex addressing mode that may need reloading */
        int idx1 = i % 16, idx2 = j % 16;
        int result;
        
        /* Address calculation: base + idx1*64 + idx2*4 (non-trivial) */
        asm volatile (
            "movl %c[offset](%%rbx, %%rcx, 4), %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+r" (result)
            : [offset] "i" (idx1 * 64),   /* Large offset */
              "b" (multi_array),          /* Base in rbx */
              "c" (idx2)                  /* Index in rcx */
            : "%eax", "memory"
        );
        
        /* More complex: pointer chain with multiple dereferences */
        int chain_result;
        asm volatile (
            "movq (%1), %%rax\n\t"        /* Load next pointer */
            "movq 16(%%rax), %%rbx\n\t"   /* Load data pointer with offset */
            "movl (%%rbx, %2, 4), %%ecx\n\t" /* Indexed load */
            "movl %%ecx, %0\n\t"
            : "=r" (chain_result)
            : "r" (nptr), "r" (k)
            : "%rax", "%rbx", "%rcx", "memory"
        );
        
        VOL(result) = result;
        VOL(chain_result) = chain_result;
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       ============================================ */
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int out1, out2, out3;
        
        /* Early-clobber on out2 (&) means it's written before all inputs read */
        asm volatile (
            "movl %3, %0\n\t"            /* out1 = in1 */
            "imull %4, %0\n\t"           /* out1 *= in2 (uses out1 as temp) */
            "movl %0, %1\n\t"            /* out2 = out1 (early clobber!) */
            "addl %5, %1\n\t"            /* out2 += in3 */
            "movl %1, %2\n\t"            /* out3 = out2 */
            "subl %3, %2\n\t"            /* out3 -= in1 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        /* Use outputs to prevent optimization */
        VOL(out1) = out1;
        VOL(out2) = out2;
        VOL(out3) = out3;
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Patterns
       ============================================ */
    {
        /* Pattern 1: Large immediate to vector register (may need GP register) */
        __m128i vec_result;
        long long large_imm = 0x1234567890ABCDEFLL;
        
        asm volatile (
            "movq %1, %%rax\n\t"         /* Load 64-bit immediate to GP reg */
            "movq %%rax, %0\n\t"         /* Move to low 64 bits of vector */
            "pslldq $8, %0\n\t"          /* Shift to high 64 bits */
            "movq %%rax, %%xmm1\n\t"     /* Another copy to temp vector */
            "por %%xmm1, %0\n\t"         /* Combine both halves */
            : "=x" (vec_result)
            : "r" (large_imm)
            : "%rax", "%xmm1", "cc"
        );
        
        /* Pattern 2: Memory operand requiring index register reload */
        double fp_result;
        int index = 7;
        
        asm volatile (
            "movslq %2, %%rcx\n\t"       /* Sign extend index to 64-bit */
            "movsd (%%rbx, %%rcx, 8), %%xmm0\n\t" /* Load with scaled index */
            "addsd %1, %%xmm0\n\t"       /* Add scalar */
            "movsd %%xmm0, %0\n\t"       /* Store result */
            : "=m" (fp_result)
            : "x" (d1),                  /* Input in XMM register */
              "r" (index),               /* Index in general register */
              "b" (global_fp)            /* Base in rbx */
            : "%rcx", "%xmm0", "memory"
        );
        
        VOL(vec_result) = vec_result;
        VOL(fp_result) = fp_result;
    }
    
    /* ============================================
       BLOCK E: Mixed Mode Reloads
       ============================================ */
    {
        /* Different machine modes in same asm */
        char c1 = 'A', c2 = 'B';
        short s1 = 1000, s2 = 2000;
        int i1 = 1000000, i2 = 2000000;
        long long ll1 = 0x1111111122222222LL;
        
        asm volatile (
            /* Mix 8-bit, 16-bit, 32-bit, and 64-bit operations */
            "addb %4, %0\n\t"            /* 8-bit add */
            "addw %5, %1\n\t"            /* 16-bit add */
            "addl %6, %2\n\t"            /* 32-bit add */
            "addq %7, %3\n\t"            /* 64-bit add */
            : "+r" (c1), "+r" (s1), "+r" (i1), "+r" (ll1)
            : "r" (c2), "r" (s2), "r" (i2), "r" (big1)
            : "cc"
        );
        
        VOL(c1) = c1;
        VOL(s1) = s1;
        VOL(i1) = i1;
        VOL(ll1) = ll1;
    }
    
    /* ============================================
       BLOCK F: High Register Pressure
       ============================================ */
    {
        /* Force many live values across asm statements */
        int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5, r6 = 6, r7 = 7, r8 = 8;
        int r9 = 9, r10 = 10, r11 = 11, r12 = 12, r13 = 13, r14 = 14, r15 = 15;
        
        /* Chain of asm statements that use many different registers */
        asm volatile ("addl %1, %0" : "+r" (r1) : "r" (r2) : "cc");
        asm volatile ("subl %1, %0" : "+r" (r3) : "r" (r4) : "cc");
        asm volatile ("imull %1, %0" : "+r" (r5) : "r" (r6) : "cc");
        asm volatile ("andl %1, %0" : "+r" (r7) : "r" (r8) : "cc");
        asm volatile ("orl %1, %0" : "+r" (r9) : "r" (r10) : "cc");
        asm volatile ("xorl %1, %0" : "+r" (r11) : "r" (r12) : "cc");
        asm volatile ("shll $2, %0" : "+r" (r13) :: "cc");
        asm volatile ("shrl $1, %0" : "+r" (r14) :: "cc");
        
        /* Use all results to keep them live */
        int sum = r1 + r3 + r5 + r7 + r9 + r11 + r13 + r14;
        VOL(sum) = sum;
    }
    
    /* ============================================
       Compute checksum to prevent optimization
       ============================================ */
    unsigned long long checksum = 0;
    
    /* Mix all variables into checksum */
    checksum += i ^ j ^ k ^ l;
    checksum += big1 ^ big2;
    checksum += *(unsigned*)&f1 ^ *(unsigned*)&f2;
    checksum += *(unsigned long long*)&d1 ^ *(unsigned long long*)&d2;
    checksum += ((unsigned long long*)&v1)[0] ^ ((unsigned long long*)&v1)[1];
    checksum += ((unsigned long long*)&v2)[0] ^ ((unsigned long long*)&v2)[1];
    
    /* Use volatile to ensure computation isn't optimized away */
    VOL(checksum) = checksum;
    
    printf("Checksum: %llx\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
