/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force variables to be in memory to increase reload opportunities */
#define NO_INLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Complex structure to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to force memory operations */
int global_array[256];
double global_doubles[128];
struct nested nested_array[16];
__m128i global_vector[8];

NO_INLINE void init_data(void) {
    for (int i = 0; i < 256; i++) global_array[i] = i;
    for (int i = 0; i < 128; i++) global_doubles[i] = i * 1.5;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) nested_array[i].a[j] = i * 10 + j;
        for (int j = 0; j < 4; j++) nested_array[i].b[j] = i * 20.0 + j;
        nested_array[i].next = &nested_array[(i + 1) % 16];
    }
    for (int i = 0; i < 8; i++) {
        global_vector[i] = _mm_set_epi32(i*4+3, i*4+2, i*4+1, i*4);
    }
}

int main(void) {
    /* Diverse variable declarations with different types and storage */
    volatile int vi = 42;
    volatile long long vll = 0x123456789ABCDEFLL;
    volatile float vf = 3.14159f;
    volatile double vd = 2.718281828459045;
    volatile __m128i vv1, vv2;
    volatile int *vp = &vi;
    volatile int result = 0;
    
    /* Initialize data */
    init_data();
    
    /* Block A: Register class conflict - force integer to FP register */
    {
        int input = vi * 2;
        double output;
        /* Request floating-point register for integer computation */
        asm volatile (
            "mov %1, %%eax\n\t"
            "cvtsi2sd %%eax, %0\n\t"
            : "=f" (output)          /* FP register output */
            : "r" (input)            /* General register input */
            : "%eax", "memory"
        );
        vd = output;
    }
    
    /* Block B: Complex address reload with multi-dimensional access */
    {
        int i = vi % 8;
        int j = (vi * 3) % 4;
        int k = (vi * 5) % 8;
        int load_result;
        
        /* Complex addressing: nested_array[i].next->a[j + k] */
        asm volatile (
            "mov (%[addr]), %0\n\t"
            : "=r" (load_result)
            : [addr] "m" (nested_array[i].next->a[j + k])
            : "memory"
        );
        result += load_result;
    }
    
    /* Block C: Early-clobber multiple outputs */
    {
        int in1 = vi;
        int in2 = vi * 2;
        int in3 = vi * 3;
        int out1, out2, out3;
        
        /* Early-clobber on out2 forces separate register allocation */
        asm volatile (
            "mov %2, %0\n\t"         /* out1 = in1 */
            "imul %3, %0\n\t"        /* out1 *= in2 */
            "mov %4, %1\n\t"         /* out2 = in3 (early clobbered) */
            "add %0, %1\n\t"         /* out2 += out1 */
            "lea (%0,%1,2), %3\n\t"  /* Use input as temporary */
            "mov %3, %2\n\t"         /* out3 = computed value */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)
            : "r" (in1), "r" (in2), "1" (in3)  /* Note: in3 in same reg as out2 */
            : "cc"
        );
        result += out1 + out2 + out3;
    }
    
    /* Block D: Secondary reload pattern - vector to integer transfer */
    {
        __m128i vec = global_vector[vi % 8];
        int64_t low, high;
        
        /* Extract low and high 64-bit parts - may need secondary reloads */
        asm volatile (
            "movq %2, %0\n\t"        /* Extract low 64 bits */
            "pextrq $1, %2, %1\n\t"  /* Extract high 64 bits */
            : "=r" (low), "=r" (high)
            : "x" (vec)              /* XMM register constraint */
            : "memory"
        );
        result += (int)(low + high);
    }
    
    /* Block E: Memory operand with complex displacement */
    {
        double sum = 0.0;
        int idx = vi;
        
        /* Force complex addressing with scale and displacement */
        asm volatile (
            "addsd %1, %0\n\t"
            "addsd 16(%1), %0\n\t"
            "addsd 32(%1,%2,8), %0\n\t"
            : "+x" (sum)
            : "r" (&global_doubles[0]), "r" (idx)
            : "memory"
        );
        vd += sum;
    }
    
    /* Block F: Multiple conflicting constraints */
    {
        int a = vi;
        int b = vi + 1;
        int c = vi + 2;
        int d;
        
        /* Force reloads by using same variable in multiple constraints */
        asm volatile (
            "lea (%1,%2,1), %0\n\t"
            "imul %3, %0\n\t"
            : "=r" (d)
            : "0" (a), "r" (b), "r" (c)  /* 'a' in same reg as output */
            : "cc"
        );
        result += d;
    }
    
    /* Block G: Large immediate that may need secondary reload */
    {
        uint64_t big_constant = 0xFFFFFFFF00000000ULL;
        uint64_t shifted;
        
        /* Large constant may need reloading through GPR */
        asm volatile (
            "mov %1, %0\n\t"
            "shr $32, %0\n\t"
            : "=r" (shifted)
            : "i" (big_constant)     /* Immediate constraint */
            : "cc"
        );
        result += (int)shifted;
    }
    
    /* Block H: Mixed register classes in one asm */
    {
        float f1 = vf;
        double d1 = vd;
        int i1 = vi;
        float f_out;
        double d_out;
        
        asm volatile (
            "cvtss2sd %2, %1\n\t"    /* float to double */
            "cvtsi2sd %3, %0\n\t"    /* int to double */
            "addsd %1, %0\n\t"       /* add doubles */
            "cvtsd2ss %0, %2\n\t"    /* double back to float */
            : "=x" (d_out), "=x" (f_out)
            : "x" (f1), "r" (i1), "0" (d1)
            : "memory"
        );
        vf = f_out;
        vd = d_out;
    }
    
    /* Final computation to prevent elimination */
    result += (int)vf + (int)vd + vi + (int)(vll >> 32);
    
    /* Use computed result */
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}
