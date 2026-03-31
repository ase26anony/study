/* test_resource_coverage.c
 * Designed to trigger mark_referenced_resources() logic for:
 * - ZERO_EXTRACT (volatile bit-field assignments)
 * - STRICT_LOW_PART (inline assembly with byte constraints)
 * - SUBREG (type punning with mixed-size accesses)
 * - MEM (complex addressing modes)
 */

#include <stddef.h>

/* Force no optimization on specific functions */
#define NOINLINE __attribute__((noinline))

/* ========== Pattern A: ZERO_EXTRACT + MEM ========== */
struct bitfield_struct {
    volatile unsigned int f1 : 5;
    volatile unsigned int f2 : 3;
    volatile unsigned int f3 : 8;
    int padding;
};

static NOINLINE void helper_with_ptr_arithmetic(struct bitfield_struct *p, int idx) {
    /* Complex addressing mode for MEM */
    volatile int *ptr = (volatile int*)p + idx * 2;
    volatile int val = *ptr;  /* MEM access with offset */
    (void)val;
}

NOINLINE void pattern_a(void) {
    struct bitfield_struct s = {0};
    volatile int counter = 0;
    
    /* ZERO_EXTRACT pattern: volatile bit-field assignment */
    s.f1 = 1;
    s.f2 = counter & 0x7;
    s.f3 = (counter >> 3) & 0xFF;
    
    /* MEM pattern with complex addressing */
    for (counter = 0; counter < 4; counter++) {
        helper_with_ptr_arithmetic(&s, counter);
    }
}

/* ========== Pattern B: STRICT_LOW_PART + SUBREG ========== */
NOINLINE void pattern_b(void) {
    volatile short vs = 0x1234;
    volatile int vi = 0xABCDEF;
    volatile char vc = 0;
    
    /* STRICT_LOW_PART pattern: inline assembly modifying low byte */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(vc)      /* =q constraint for byte-addressable register */
        : "0"(vc)       /* matching input constraint */
        : "cc"
    );
    
    /* SUBREG pattern: type punning with mixed-size access */
    short *ps = (short*)&vi;  /* Cast int* to short* */
    *ps = vs;                 /* SUBREG store */
    
    /* Another SUBREG pattern */
    char *pc = (char*)&vi + 1;
    *pc = vc;                /* SUBREG byte access */
}

/* ========== Pattern C: Complex mixed patterns ========== */
struct container {
    volatile unsigned int flags : 4;
    volatile unsigned int value : 12;
    int data[8];
};

NOINLINE void pattern_c(int idx1, int idx2) {
    static struct container containers[4];
    volatile int *volatile ptr;  /* Extra volatile to prevent optimization */
    
    /* Ternary selecting address with bit-field access */
    ptr = (idx1 & 1) ? 
          (volatile int*)&containers[0].flags : 
          (volatile int*)&containers[1].value;
    
    /* ZERO_EXTRACT through pointer */
    if (ptr == (volatile int*)&containers[0].flags) {
        containers[0].flags = idx2 & 0xF;  /* ZERO_EXTRACT */
    }
    
    /* Complex MEM addressing with multiple indices */
    volatile int v = containers[idx1 & 3].data[idx2 & 7];
    
    /* SUBREG access through char pointer */
    char *byte_ptr = (char*)&v;
    byte_ptr[(idx1 + idx2) & 3] = idx1 & 0xFF;
    
    (void)v;  /* Prevent unused variable warning */
}

/* ========== Pattern D: Loop with all patterns ========== */
NOINLINE void pattern_d(volatile int iterations) {
    struct {
        volatile unsigned int bf : 7;
        volatile int array[16];
    } ctx;
    
    volatile int i, j;
    volatile int temp = 0;
    
    for (i = 0; i < iterations && i < 8; i++) {
        /* ZERO_EXTRACT in loop */
        ctx.bf = (i * 3) & 0x7F;
        
        /* MEM with complex addressing */
        for (j = 0; j < 4; j++) {
            temp += ctx.array[i * 2 + j];  /* MEM with scaled index */
        }
        
        /* STRICT_LOW_PART via assembly every 3rd iteration */
        if ((i & 3) == 0) {
            volatile char c = temp & 0xFF;
            asm volatile (
                "orb $0x10, %0\n\t"
                : "=q"(c)
                : "0"(c)
                : "cc"
            );
            temp = (temp & ~0xFF) | c;
        }
    }
    
    /* Final SUBREG pattern */
    short *sp = (short*)&temp;
    *sp = (*sp + 1) & 0x7FFF;
}

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    volatile int iterations = 3;  /* Prevent loop unrolling */
    volatile int i, result = 0;
    
    /* Use argc to make iterations non-constant for compiler */
    if (argc > 1) {
        iterations = 4;
    }
    
    /* Initialize some data */
    int dummy_array[10][10];
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            dummy_array[i][j] = i * 10 + j;
        }
    }
    
    /* Call pattern functions in loop */
    for (i = 0; i < iterations; i++) {
        pattern_a();
        pattern_b();
        pattern_c(i, i * 2);
        pattern_d(i + 1);
        
        /* MEM pattern with 2D array indexing */
        volatile int v = dummy_array[i % 10][(i * 3) % 10];
        result += v;  /* Prevent dead code elimination */
    }
    
    /* Additional SUBREG pattern in main */
    {
        volatile long long big = 0x123456789ABCDEF0LL;
        int *ip = (int*)&big;
        *ip = result;  /* SUBREG access to 64-bit value */
        
        /* ZERO_EXTRACT via bit-field in local struct */
        struct { volatile unsigned int local_bf : 9; } ls;
        ls.local_bf = result & 0x1FF;
    }
    
    return result & 0xFF;  /* Ensure program has non-constant return */
}
