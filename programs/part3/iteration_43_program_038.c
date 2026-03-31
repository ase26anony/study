/* resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic (mark_referenced_resources in resource.cc).
 * The target lines handle ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM.
 * 
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -c resource_coverage.c
 * Additional flags for debugging: -fdump-rtl-all -dP -da
 */

#include <stddef.h>

/* Prevent inlining to ensure separate RTL generation for each function */
#define NOINLINE __attribute__((noinline))

/* ===== Pattern 1: ZERO_EXTRACT + MEM ===== */
struct BitFieldStruct {
    volatile unsigned int field1 : 5;
    volatile unsigned int field2 : 7;
    volatile unsigned int field3 : 3;
};

static NOINLINE void pattern_zero_extract_mem(struct BitFieldStruct *s, int idx) {
    /* Complex addressing mode for MEM */
    volatile int *arr = (volatile int*)s;
    
    /* ZERO_EXTRACT from volatile bit-field assignment */
    s->field1 = idx & 0x1F;
    
    /* MEM with complex addressing: pointer arithmetic with multiple indices */
    volatile int val = arr[(idx * 3 + 7) % 16];
    
    /* Another ZERO_EXTRACT */
    s->field2 = val & 0x7F;
    
    /* MEM with scaled index */
    volatile int *ptr = arr + (idx << 2);
    s->field3 = *ptr & 0x7;
}

/* ===== Pattern 2: STRICT_LOW_PART + SUBREG ===== */
static NOINLINE void pattern_strict_low_part_subreg(volatile int *base, int offset) {
    /* Use char/short types to encourage QI/HI modes */
    volatile char c;
    volatile short s;
    volatile int i;
    
    /* Type punning for SUBREG: access int as smaller types */
    i = *base + offset;
    
    /* STRICT_LOW_PART via inline assembly modifying byte-sized part */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)      /* =q constraint for byte-addressable register */
        : "0"(c)       /* matching input constraint */
        : "cc"
    );
    
    /* SUBREG: cast pointer to larger type to smaller type */
    short *ps = (short*)&i;
    *ps = (short)(i + 1);  /* Generates SUBREG access */
    
    /* Another STRICT_LOW_PART on short */
    asm volatile (
        "incw %0\n\t"
        : "=r"(s)
        : "0"(s)
        : "cc"
    );
    
    /* More SUBREG: access different parts of the same memory */
    char *pc = (char*)&i;
    pc[1] = c;
    pc[3] = s >> 8;
}

/* ===== Pattern 3: Mixed patterns with ternary operator ===== */
struct Mixed {
    volatile unsigned int bits : 4;
    volatile int array[8];
    volatile long long large;
};

static NOINLINE void pattern_mixed(struct Mixed *m, int idx1, int idx2) {
    /* Complex expression with ternary selecting address */
    volatile int *addr = (idx1 > idx2) ? 
                         &m->array[idx1 % 8] : 
                         (int*)&m->large;
    
    /* ZERO_EXTRACT on bit-field */
    m->bits = (*addr & 0xF);
    
    /* MEM with complex addressing mode */
    volatile int val = m->array[(idx1 + idx2 * 3) & 7];
    
    /* Type punning for SUBREG */
    short *half = (short*)addr;
    *half = (short)val;
    
    /* Another ZERO_EXTRACT */
    m->bits = (val >> 4) & 0xF;
}

/* ===== Pattern 4: Complex loop with all patterns ===== */
static NOINLINE void pattern_complex(volatile int *arr, int size) {
    struct BitFieldStruct bfs = {0};
    struct Mixed m = {0};
    
    for (volatile int i = 0; i < size; i++) {
        /* Alternate between patterns based on i */
        if (i & 1) {
            pattern_zero_extract_mem(&bfs, i);
        } else {
            pattern_strict_low_part_subreg(arr, i);
        }
        
        /* Every 4 iterations, use mixed pattern */
        if ((i & 3) == 0) {
            pattern_mixed(&m, i, i >> 1);
        }
        
        /* Complex MEM addressing with multiple indices */
        volatile int x = arr[(i * 7 + 3) % size];
        volatile int y = arr[(i * 5 + 1) % size];
        
        /* ZERO_EXTRACT via bit-field in struct */
        bfs.field2 = (x + y) & 0x7F;
        
        /* SUBREG via type punning */
        char *byte_ptr = (char*)&x;
        byte_ptr[2] = y & 0xFF;
    }
}

/* ===== Main function to drive everything ===== */
int main(int argc, char **argv) {
    /* Use argc to bound loops (prevents infinite loops in analysis) */
    int iterations = (argc > 1) ? 10 : 5;
    volatile int result = 0;
    
    /* Initialize data structures with volatile members */
    volatile int array[32];
    struct BitFieldStruct bfs = {0};
    struct Mixed m = {0};
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 32; i++) {
        array[i] = i * 3 + 1;
    }
    
    /* Main loop that calls pattern functions */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call each pattern function with arguments derived from loop */
        pattern_zero_extract_mem((struct BitFieldStruct*)&bfs, i);
        pattern_strict_low_part_subreg((int*)array, i);
        pattern_mixed((struct Mixed*)&m, i, i + 1);
        
        /* Every few iterations, call complex pattern */
        if (i % 3 == 0) {
            pattern_complex((int*)array, 32);
        }
        
        /* Use results to prevent dead code elimination */
        result += bfs.field1 + bfs.field2 + m.bits;
        
        /* More complex addressing to encourage MEM patterns */
        volatile int *ptr = (int*)&m;
        ptr[i % 8] = result;
        
        /* SUBREG through pointer casting */
        short *sptr = (short*)&result;
        sptr[0] = sptr[1] + 1;
    }
    
    /* Final dummy operation using all results */
    volatile int final = result + bfs.field3 + m.array[0];
    
    /* Return something based on computation to avoid removal */
    return (final > 0) ? 0 : 1;
}
