/* test_resource.c - Program to trigger ZERO_EXTRACT, STRICT_LOW_PART, and SUBREG RTL patterns */

#include <stdio.h>
#include <stdint.h>

/* Structure with bit-fields to generate ZERO_EXTRACT and STRICT_LOW_PART */
struct bitfield_pack {
    volatile unsigned int a : 1;
    volatile unsigned int b : 3;
    volatile unsigned int c : 12;
    volatile unsigned int d : 8;
    volatile unsigned int e : 8;
} __attribute__((packed));

/* Union for mixed-type access */
union mixed_access {
    volatile uint32_t full;
    volatile struct {
        uint32_t low : 16;
        uint32_t high : 16;
    } parts;
};

/* Function to force complex addressing modes */
static int complex_index(int i, int stride) {
    return (i * stride + 7) & 0x3F;  /* Non-trivial indexing */
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile struct bitfield_pack bf = {0};
    volatile union mixed_access mix = {0};
    volatile long long ll_var = 0x123456789ABCDEF0LL;
    volatile double dbl_var = 3.141592653589793;
    volatile int array[64] = {0};
    volatile int limit = (argc > 1) ? 10 : 5;  /* External control */
    int result = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 64; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Main loop with data-dependent control flow */
    for (volatile int i = 0; i < limit; i++) {
        /* 1. Bit-field manipulations for ZERO_EXTRACT/STRICT_LOW_PART */
        bf.a = (i & 1);                     /* Single bit assignment */
        bf.b = (i & 0x7);                   /* 3-bit field */
        bf.c = (bf.c + i) & 0xFFF;          /* 12-bit field with wrap */
        
        /* Extract using masks and shifts (may generate ZERO_EXTRACT) */
        unsigned int extracted = (bf.full >> 4) & 0xF;  /* Extract bits 4-7 */
        
        /* Combined bit-field operations */
        bf.d = (bf.b << 2) | (bf.a ? 1 : 0);
        bf.e = bf.c >> 4;
        
        /* 2. Multi-word operations for SUBREG generation */
        /* Operations on long long (multiple registers on 32-bit) */
        ll_var = ll_var + (long long)(i * 0x10001LL);
        ll_var = ll_var ^ (0xF0F0F0F0F0F0F0F0LL >> (i & 0x3F));
        
        /* Split access to force SUBREG handling */
        uint32_t low_part = (uint32_t)(ll_var & 0xFFFFFFFF);
        uint32_t high_part = (uint32_t)(ll_var >> 32);
        mix.parts.low = low_part;
        mix.parts.high = high_part ^ 0x5555;
        
        /* Double operations (may use multiple FP registers) */
        dbl_var = dbl_var * 1.5 + (double)i;
        dbl_var = dbl_var - (double)(low_part) / 1000.0;
        
        /* 3. Complex memory addressing with bit-field derived index */
        int idx = complex_index(i, bf.c & 0x1F);
        array[idx] = array[idx] + (bf.d << 1);
        
        /* Another complex access pattern */
        int idx2 = ((i * 13 + bf.e) & 0x3F);
        array[idx2] ^= (mix.full >> (i & 0xF));
        
        /* 4. Control flow based on bit-field and multi-word results */
        if (bf.a) {
            /* Branch 1: More bit-field manipulations */
            bf.c = (bf.c << 1) | (bf.c >> 11);  /* Rotate 12-bit field */
            ll_var = ll_var | ((long long)bf.c << 20);
        } else {
            /* Branch 2: Different operations */
            bf.d = bf.d + bf.b;
            ll_var = ll_var & ~((long long)0xFF << 24);
        }
        
        /* Condition based on high vs low word comparison */
        if (high_part > low_part) {
            array[i & 0x3F] += high_part - low_part;
        } else if (high_part < low_part) {
            array[i & 0x3F] += (low_part - high_part) & 0xFF;
        }
        
        /* Switch based on extracted bits */
        switch (extracted & 0x7) {
            case 0:
                dbl_var += 0.1;
                break;
            case 1:
                dbl_var -= 0.1;
                break;
            case 2:
                bf.e = (bf.e + 1) & 0xFF;
                break;
            case 3:
                ll_var = ll_var >> 1;
                break;
            default:
                array[extracted & 0x3F] = i;
                break;
        }
        
        /* Prevent optimization of loop variable */
        asm volatile("" : "+r" (i));
    }
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < 64; i++) {
        result += array[i];
    }
    result += bf.c + bf.d + bf.e;
    result += (int)(ll_var & 0xFFFFFFFF) + (int)(ll_var >> 32);
    result += (int)(dbl_var * 1000);
    
    /* Use result to affect return value */
    return (result > 0) ? 0 : 1;
}

/* Additional function with inline assembly for specific patterns */
void asm_helpers(void) {
    volatile long long ll_input = 0x123456789ABCDEF0LL;
    volatile uint32_t output;
    
    /* Inline assembly that might generate STRICT_LOW_PART-like behavior */
    asm volatile (
        "movl %1, %0\n\t"
        "addl $0x1234, %0"
        : "=r" (output)
        : "r" ((uint32_t)ll_input)
        : "cc"
    );
    
    /* Another asm that operates on partial register */
    volatile uint64_t big_val = 0xFEDCBA9876543210ULL;
    volatile uint32_t low_part, high_part;
    
    asm volatile (
        "movl %2, %0\n\t"
        "movl %3, %1"
        : "=&r" (low_part), "=r" (high_part)
        : "r" ((uint32_t)big_val), "r" ((uint32_t)(big_val >> 32))
        :
    );
}
