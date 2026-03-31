/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Force variables to be in memory to increase reload opportunities */
volatile int vi = 42;
volatile long long vll = 9876543210LL;
volatile float vf = 3.14159f;
volatile double vd = 2.718281828459045;
volatile __m128i v128;

/* Complex array structures to force address reloads */
int multi_array[10][20][30];
struct nested {
    int a;
    long b;
    float c;
    double d;
    struct nested *next;
} nested_array[100];

/* Function to prevent optimization */
static int use_result(int x) {
    volatile int sink = x;
    return sink;
}

int main(void) {
    int result = 0;
    
    /* 1. REGISTER CLASS CONFLICT RELOADS */
    {
        int int_var = vi;
        float float_var = vf;
        double double_var = vd;
        
        /* Force integer to float register reload */
        asm volatile (
            "mov %1, %%eax\n\t"
            "cvtsi2ss %%eax, %0\n\t"
            : "=f" (float_var)      /* Output in floating-point register */
            : "r" (int_var)         /* Input in general-purpose register */
            : "%eax", "memory"
        );
        
        /* Force float to integer register reload */
        asm volatile (
            "cvtss2si %1, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "=r" (int_var)        /* Output in general-purpose register */
            : "f" (float_var)       /* Input in floating-point register */
            : "%eax"
        );
        
        result += int_var + (int)float_var;
    }
    
    /* 2. COMPLEX ADDRESS RELOADS */
    {
        int i = vi % 10;
        int j = (vi * 3) % 20;
        int k = (vi * 7) % 30;
        int *complex_addr;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "lea (%[base], %[idx1], 4), %[addr]\n\t"
            "add %[idx2], %[addr]\n\t"
            : [addr] "=r" (complex_addr)
            : [base] "r" (&multi_array[0][0][0]),
              [idx1] "r" (i * 20 * 30 + j * 30),
              [idx2] "r" (k * sizeof(int))
            : "cc"
        );
        
        /* Use the computed address with memory constraint */
        int loaded_value;
        asm volatile (
            "movl (%1), %0\n\t"
            : "=r" (loaded_value)
            : "r" (complex_addr)
            : "memory"
        );
        
        /* Even more complex: struct pointer chain */
        struct nested *ptr = &nested_array[i];
        for (int n = 0; n < 5; n++) {
            ptr->next = &nested_array[(i + n) % 100];
            ptr = ptr->next;
        }
        
        /* Access through pointer chain - may need address reload */
        double chain_value;
        asm volatile (
            "movq 24(%1), %0\n\t"  /* Access d field at offset 24 */
            : "=r" (chain_value)
            : "r" (ptr->next->next)
            : "memory"
        );
        
        result += loaded_value + (int)chain_value;
    }
    
    /* 3. EARLY-CLOBBER MULTIPLE OUTPUT RELOADS */
    {
        int in1 = vi;
        int in2 = vi * 2;
        int in3 = vi * 3;
        int out1, out2, out3;
        
        /* Multiple outputs with early clobber on one */
        asm volatile (
            "imull %2, %0\n\t"      /* out1 = in1 * in2 */
            "addl %3, %1\n\t"       /* out2 = in1 + in3 - EARLY CLOBBER! */
            "subl %1, %0\n\t"       /* out1 -= out2 (uses early-clobbered reg) */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)
            : "r" (in1), "r" (in2), "r" (in3),
              "0" (in1), "1" (in1)  /* Same as outputs for initialization */
            : "cc"
        );
        
        /* Another early-clobber with overlapping constraints */
        long long ll_in = vll;
        int hi_part, lo_part;
        asm volatile (
            "movq %2, %%rax\n\t"
            "movl %%eax, %0\n\t"    /* lo_part = low 32 bits */
            "shrl $32, %%rax\n\t"
            "movl %%eax, %1\n\t"    /* hi_part = high 32 bits */
            : "=&r" (lo_part), "=&r" (hi_part)
            : "r" (ll_in)
            : "%rax", "cc"
        );
        
        result += out1 + out2 + out3 + hi_part + lo_part;
    }
    
    /* 4. SECONDARY RELOAD PATTERNS */
    {
        /* Large immediate that might need secondary reload on some arches */
        uint64_t large_const = 0x123456789ABCDEF0ULL;
        uint64_t shifted;
        
        asm volatile (
            "mov %1, %0\n\t"
            "shr $32, %0\n\t"
            : "=r" (shifted)
            : "r" (large_const)    /* Might need temp register for 64-bit const */
            : "cc"
        );
        
        /* Vector register reload pattern */
        __m128i vec1 = _mm_set_epi32(vi, vi*2, vi*3, vi*4);
        __m128i vec2 = _mm_set_epi32(vi*5, vi*6, vi*7, vi*8);
        __m128i vec_result;
        
        /* Vector operation that might need general-purpose reg as intermediate */
        int shuffle_mask = 0x1B;  /* Complex shuffle pattern */
        asm volatile (
            "movd %2, %%xmm2\n\t"
            "pshufd %%xmm2, %1, %0\n\t"
            : "=x" (vec_result)
            : "x" (vec1), "r" (shuffle_mask)
            : "%xmm2"
        );
        
        /* Extract to general-purpose register - may need reload */
        int extracted;
        asm volatile (
            "pextrd $0, %1, %0\n\t"
            : "=r" (extracted)
            : "x" (vec_result)
        );
        
        result += (int)shifted + extracted;
    }
    
    /* 5. MIXED MODE RELOADS */
    {
        /* Different machine modes in same asm */
        char c = (char)vi;
        short s = (short)(vi * 100);
        int i = vi;
        long long ll = vll;
        
        asm volatile (
            "movsbl %1, %0\n\t"
            : "=r" (i)
            : "r" (c)              /* Byte to int - mode change */
            : "cc"
        );
        
        asm volatile (
            "movswl %1, %%eax\n\t"
            "cltq\n\t"
            "movq %%rax, %0\n\t"
            : "=r" (ll)
            : "r" (s)              /* Short to long long - two mode changes */
            : "%rax", "cc"
        );
        
        /* Floating point with different precisions */
        float f = vf;
        double d = vd;
        long double ld = (long double)vd * 2.0L;
        
        asm volatile (
            "cvtss2sd %1, %0\n\t"
            : "=x" (d)
            : "x" (f)              /* Single to double precision */
        );
        
        result += i + (int)ll + (int)d;
    }
    
    /* 6. REGISTER PRESSURE TO FORCE SPILL/RELOAD */
    {
        /* Many live variables to increase register pressure */
        int r0 = vi, r1 = vi+1, r2 = vi+2, r3 = vi+3, r4 = vi+4;
        int r5 = vi+5, r6 = vi+6, r7 = vi+7, r8 = vi+8, r9 = vi+9;
        
        /* Long asm that uses many registers */
        asm volatile (
            "addl %1, %0\n\t"
            "addl %2, %0\n\t"
            "addl %3, %0\n\t"
            "addl %4, %0\n\t"
            "addl %5, %0\n\t"
            "addl %6, %0\n\t"
            "addl %7, %0\n\t"
            "addl %8, %0\n\t"
            "addl %9, %0\n\t"
            : "+r" (r0)
            : "r" (r1), "r" (r2), "r" (r3), "r" (r4),
              "r" (r5), "r" (r6), "r" (r7), "r" (r8), "r" (r9)
            : "cc"
        );
        
        /* Force some to memory and reload */
        volatile int *mem_ptr = &vi;
        asm volatile (
            "movl %1, (%0)\n\t"
            :
            : "r" (mem_ptr), "r" (r0)
            : "memory"
        );
        
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
    }
    
    /* Final result to prevent optimization */
    result = use_result(result);
    
    printf("Result: %d\n", result);
    return result;
}
