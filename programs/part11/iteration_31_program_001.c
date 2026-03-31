/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdlib.h>

/* Structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
struct bitfield_pack {
    unsigned int a : 1;
    unsigned int b : 3;
    unsigned int c : 12;
    unsigned int d : 16;
} __attribute__((packed));

/* Union for accessing same memory as different types */
union mixed_access {
    struct bitfield_pack bits;
    unsigned int full;
    unsigned short halves[2];
    unsigned char bytes[4];
};

/* Volatile variables to prevent optimization */
volatile struct bitfield_pack bf_var;
volatile union mixed_access mix_var;
volatile long long ll_var;
volatile double dbl_var;
volatile int array[256];

/* Function with inline assembly to force specific patterns */
static inline unsigned int asm_low_part(unsigned long long val) {
    unsigned int result;
    /* Inline asm that operates on low 32 bits, potentially creating STRICT_LOW_PART */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $0x1234, %0"
        : "=r" (result)
        : "r" ((unsigned int)val)
        : "cc"
    );
    return result;
}

static inline unsigned long long asm_combine(unsigned int lo, unsigned int hi) {
    unsigned long long result;
    /* Combine two 32-bit values into 64-bit, generating SUBREG patterns */
    __asm__ volatile (
        "movl %1, %k0\n\t"
        "shlq $32, %0\n\t"
        "orq %2, %0"
        : "=r" (result)
        : "r" (hi), "r" ((unsigned long long)lo)
        : "cc"
    );
    return result;
}

/* Function to create complex memory addressing */
static void complex_mem_access(volatile int *arr, int idx, int stride) {
    /* Complex addressing that may generate MEM with complex XEXP */
    arr[idx * stride + 7] = arr[idx * stride + 3] + arr[idx * stride + 11];
    
    /* Access with bit-field derived offset */
    int offset = (bf_var.c & 0xF) * 4;
    arr[offset] ^= 0x55AA55AA;
}

int main(int argc, char **argv) {
    int i, j;
    volatile int limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int temp;
    unsigned long long accum = 0;
    
    /* Initialize variables */
    bf_var.a = 1;
    bf_var.b = 5;
    bf_var.c = 1234;
    bf_var.d = 5678;
    
    mix_var.full = 0xDEADBEEF;
    
    ll_var = 0x123456789ABCDEF0ULL;
    dbl_var = 3.141592653589793;
    
    for (i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Main loop with mixed operations */
    for (i = 0; i < limit; i++) {
        /* 1. Bit-field operations for ZERO_EXTRACT/STRICT_LOW_PART */
        temp = bf_var.c;  /* May generate ZERO_EXTRACT */
        bf_var.b = (temp >> 2) & 0x7;  /* Bit-field assignment */
        
        /* Extract and manipulate specific bits */
        unsigned int extracted = (mix_var.full >> 8) & 0xFFF;  /* 12-bit extract */
        bf_var.c = extracted ^ 0x888;
        
        /* 2. Multi-word operations for SUBREG generation */
        ll_var = ll_var + 0x100000001ULL;  /* 64-bit add on 32-bit target */
        
        /* Split 64-bit operations */
        unsigned int lo_part = (unsigned int)ll_var;
        unsigned int hi_part = (unsigned int)(ll_var >> 32);
        
        /* Operations that may generate SUBREG */
        hi_part = hi_part + lo_part;
        lo_part = lo_part ^ 0xAAAAAAAA;
        
        /* Recombine - may create SUBREG patterns */
        ll_var = ((unsigned long long)hi_part << 32) | lo_part;
        
        /* Double precision operations */
        dbl_var = dbl_var * 1.0001;
        unsigned long long as_int = *(unsigned long long*)&dbl_var;
        as_int = as_int ^ 0x8000000000000000ULL;  /* Flip sign bit */
        dbl_var = *(double*)&as_int;
        
        /* 3. Complex memory addressing */
        int stride = (bf_var.b + 1) * 2;
        complex_mem_access(array, i & 0x3F, stride);
        
        /* Array access with complex index calculation */
        int idx = ((i * 7) + (bf_var.c & 0x1F)) & 0xFF;
        array[idx] = array[idx] + (temp & 0xFF);
        
        /* 4. Control flow based on bit-field results */
        if (bf_var.a ^ (bf_var.c & 1)) {
            /* Branch 1: More bit-field manipulation */
            mix_var.bits.a = !mix_var.bits.a;
            mix_var.bits.d = mix_var.bits.d + bf_var.b;
            
            /* Use inline assembly functions */
            unsigned int asm_result = asm_low_part(ll_var);
            bf_var.c = (bf_var.c + asm_result) & 0xFFF;
        } else {
            /* Branch 2: Different operations */
            ll_var = asm_combine(bf_var.d, bf_var.c);
            
            /* Access memory through pointer with offset */
            volatile int *ptr = &array[bf_var.c % 240];
            for (j = 0; j < 4; j++) {
                ptr[j] = ptr[j] * 2 + 1;
            }
        }
        
        /* Conditional based on high vs low word comparison */
        if (hi_part > lo_part) {
            /* Swap operations */
            unsigned int tmp = hi_part;
            hi_part = lo_part;
            lo_part = tmp;
            
            /* More complex memory pattern */
            array[(i * 13) & 0xFF] = array[(i * 17) & 0xFF] ^ array[(i * 19) & 0xFF];
        }
        
        /* Accumulate results to prevent elimination */
        accum += bf_var.c + (lo_part & 0xFFF) + array[i & 0xFF];
    }
    
    /* Final aggregation and output */
    unsigned long long final_result = accum + ll_var + (unsigned long long)(dbl_var * 1000);
    
    /* Mix in array contents */
    for (i = 0; i < 256; i += 8) {
        final_result ^= (unsigned long long)array[i] << (i & 0x38);
    }
    
    printf("Result: 0x%llx\n", final_result);
    return (int)(final_result & 0x7FFFFFFF);
}
