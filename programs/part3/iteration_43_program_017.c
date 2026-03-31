/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic (mark_referenced_resources), particularly
 * targeting the uncovered lines handling ZERO_EXTRACT, STRICT_LOW_PART,
 * SUBREG, and MEM expressions.
 *
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -c test_resource_coverage.c
 * Additional flags for debugging: -fdump-rtl-all -dP -da
 */

#include <stddef.h>

/* Force functions to be considered for optimization passes */
#define NOINLINE __attribute__((noinline))

/* ========== Pattern A: ZERO_EXTRACT and MEM ========== */
struct bitfield_struct {
    volatile unsigned int f1 : 5;
    volatile unsigned int f2 : 7;
    volatile unsigned int f3 : 3;
    volatile unsigned int padding : 17;
};

static NOINLINE void pattern_a_zf_mem(struct bitfield_struct *s, int idx) {
    /* Complex addressing with MEM */
    volatile int *arr = (volatile int*)s;
    
    /* Multiple ZERO_EXTRACT patterns through volatile bit-field assignments */
    s->f1 = (idx & 0x1F);           /* Likely ZERO_EXTRACT */
    s->f2 = (idx >> 5) & 0x7F;      /* Another ZERO_EXTRACT */
    
    /* MEM with complex addressing */
    arr[(idx & 3) + 1] = arr[idx & 3] + 1;
    
    /* Nested MEM access through pointer */
    volatile int *ptr = &arr[2];
    *ptr = s->f3 | 0x10;
}

/* ========== Pattern B: STRICT_LOW_PART and SUBREG ========== */
static NOINLINE void pattern_b_slp_subreg(volatile short *ps, volatile char *pc) {
    int combined;
    short temp_short;
    char temp_char;
    
    /* STRICT_LOW_PART via inline assembly modifying only low byte */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(temp_char)   /* =q constraint for byte-addressable register */
        : "0"(temp_char)
        : "cc"
    );
    
    /* SUBREG pattern through type punning */
    combined = 0x12345678;
    short *subreg_ptr = (short*)&combined;
    *subreg_ptr = *ps + 1;          /* SUBREG access to 32-bit integer */
    
    /* Mixed-size access causing SUBREG */
    *pc = (char)(combined >> 8);
    
    /* Another SUBREG pattern */
    temp_short = *ps;
    int widened = (int)temp_short;  /* Potential SUBREG in RTL */
    *pc = (char)widened;
}

/* ========== Pattern C: Mixed patterns with ternary ========== */
static NOINLINE void pattern_c_mixed(struct bitfield_struct *s1, 
                                     struct bitfield_struct *s2,
                                     int selector) {
    volatile int *target;
    
    /* Ternary selecting different MEM addresses */
    target = (selector & 1) ? 
             (volatile int*)&s1->f1 : 
             (volatile int*)&s2->f2;
    
    /* Complex expression with MEM */
    *target = (selector << 3) & 0xFF;
    
    /* Additional ZERO_EXTRACT possibility */
    if (selector > 100) {
        s1->f3 = selector & 0x07;
    }
    
    /* Pointer arithmetic creating complex MEM addresses */
    volatile char *byte_ptr = (volatile char*)target;
    byte_ptr[selector & 3] = byte_ptr[(selector + 1) & 3] + 1;
}

/* ========== Pattern D: Complex loop with all patterns ========== */
static NOINLINE void pattern_d_complex(volatile int *arr, int size) {
    struct bitfield_struct bs = {0};
    volatile short hs = 0;
    volatile char hc = 0;
    
    for (volatile int i = 0; i < (size & 3); i++) {
        /* Mix patterns within loop */
        bs.f1 = arr[i] & 0x1F;                     /* ZERO_EXTRACT */
        
        /* STRICT_LOW_PART via assembly in loop */
        asm volatile (
            "incb %0\n\t"
            : "+q"(hc)
            :
            : "cc"
        );
        
        /* SUBREG access */
        short *sp = (short*)&arr[i];
        hs = *sp + hc;
        
        /* Complex MEM addressing */
        arr[(i + 1) % size] = arr[i] + bs.f1;
    }
}

/* ========== Main driver ========== */
int main(int argc, char **argv) {
    volatile int iterations = (argc > 1) ? 10 : 5;  /* Prevent infinite loops */
    struct bitfield_struct bs1 = {0}, bs2 = {0};
    volatile short vs = 1000;
    volatile char vc = 50;
    volatile int sum = 0;
    
    /* Array for MEM patterns */
    volatile int mem_array[16];
    for (int i = 0; i < 16; i++) {
        mem_array[i] = i * 2;
    }
    
    /* Main loop to trigger resource tracking across passes */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call each pattern function with volatile args */
        pattern_a_zf_mem(&bs1, i);
        pattern_b_slp_subreg(&vs, &vc);
        pattern_c_mixed(&bs1, &bs2, i);
        pattern_d_complex(mem_array, 16);
        
        /* Prevent dead code elimination */
        sum += bs1.f1 + bs2.f2 + vs + vc + mem_array[i & 0xF];
    }
    
    /* Use sum to avoid being optimized out */
    if (sum > 1000000) {
        return 1;  /* Never happens, but compiler doesn't know */
    }
    
    return 0;
}

/* Additional global variables to increase complexity */
volatile unsigned long global_var = 0xDEADBEEF;
volatile struct bitfield_struct global_bs;

/* One more function called from main to increase RTL diversity */
static NOINLINE void extra_pattern(void) {
    /* SUBREG with 64-bit to 32-bit on 32-bit target */
    volatile unsigned long long big = 0x123456789ABCDEF0ULL;
    volatile unsigned int *small = (volatile unsigned int*)&big;
    
    /* ZERO_EXTRACT from 64-bit */
    global_bs.f1 = (big >> 10) & 0x1F;
    
    /* MEM with displacement */
    small[1] = small[0] + global_bs.f2;
    
    /* STRICT_LOW_PART on char from int */
    volatile char cc = (char)big;
    asm volatile (
        "orb $0x0F, %0\n\t"
        : "+q"(cc)
        :
        : "cc"
    );
}
