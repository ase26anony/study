/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource tracking logic (mark_referenced_resources in resource.cc).
 * The goal is to cover lines 282-290 that handle ZERO_EXTRACT,
 * STRICT_LOW_PART, SUBREG, and MEM expressions.
 *
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -fdump-rtl-all -c test_resource_coverage.c
 * Or for scheduling passes: gcc -O3 -funroll-loops -m32 -fno-strict-aliasing -c test_resource_coverage.c
 */

#include <stddef.h>

/* Prevent inlining to ensure separate RTL generation for each function */
#define NOINLINE __attribute__((noinline))

/* ========== Pattern A: ZERO_EXTRACT and MEM ========== */
struct bitfield_struct {
    volatile unsigned int f1 : 5;
    volatile unsigned int f2 : 7;
    volatile unsigned int f3 : 10;
    volatile unsigned int padding : 10;
};

static NOINLINE void pattern_a_zxt_mem(struct bitfield_struct *s, int idx) {
    /* Complex addressing with MEM */
    volatile int *arr = (volatile int*)s;
    
    /* Multiple MEM accesses with addressing modes */
    int val1 = arr[idx];
    int val2 = arr[idx + 1];
    int val3 = arr[idx + 2];
    
    /* ZERO_EXTRACT through volatile bit-field assignment */
    s->f1 = (val1 & 0x1F);        /* 5-bit field */
    s->f2 = (val2 & 0x7F);        /* 7-bit field */
    s->f3 = (val3 & 0x3FF);       /* 10-bit field */
    
    /* More complex MEM with pointer arithmetic */
    volatile int *ptr = arr + idx;
    ptr += (val1 & 0x3);
    *ptr = val2;
}

/* ========== Pattern B: STRICT_LOW_PART and SUBREG ========== */
static NOINLINE void pattern_b_slp_subreg(int base) {
    /* Use char/short types to encourage QI/HI modes */
    volatile char c = (char)base;
    volatile short s = (short)base;
    
    /* STRICT_LOW_PART via inline assembly modifying byte part */
    asm volatile (
        "addb $1, %0\n\t"
        "subb $2, %0"
        : "=q"(c)   /* =q constraint for byte-addressable register */
        : "0"(c)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART for short */
    asm volatile (
        "addw $3, %0\n\t"
        "subw $1, %0"
        : "=r"(s)   /* Word operation that may use STRICT_LOW_PART */
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG through type punning */
    int i = base;
    short *ps = (short*)&i;  /* Cast to different-sized pointer */
    *ps = (short)s;          /* SUBREG store */
    
    /* More SUBREG: access different parts of larger type */
    char *pc = (char*)&i;
    pc[1] = (char)c;         /* Another SUBREG access */
    
    /* Mixed-size operations */
    long long ll = (long long)base;
    int *pi = (int*)&ll;
    pi[0] = i;               /* 32-bit SUBREG of 64-bit value on 32-bit target */
}

/* ========== Pattern C: Mixed patterns with ternary ========== */
static NOINLINE void pattern_c_mixed(struct bitfield_struct *s1, 
                                     struct bitfield_struct *s2,
                                     int selector) {
    /* Complex addressing with ternary operator */
    volatile struct bitfield_struct *target = 
        (selector & 1) ? s1 : s2;
    
    /* Array-like access through pointer */
    volatile unsigned int *base_ptr = 
        (selector & 2) ? (volatile unsigned int*)s1 : 
                         (volatile unsigned int*)s2;
    
    /* MEM with complex addressing mode */
    int idx = selector & 0x3;
    volatile unsigned int val = base_ptr[idx * 2];
    
    /* ZERO_EXTRACT assignment based on condition */
    if (selector & 4) {
        target->f1 = (val & 0x1F);
    } else {
        target->f2 = (val & 0x7F);
    }
    
    /* More SUBREG through type punning in the same function */
    int temp = val;
    short *ps = (short*)&temp;
    *ps = (short)(selector & 0xFFFF);
    
    /* Additional MEM reference */
    volatile int *mem_ptr = (volatile int*)target;
    mem_ptr[1] = temp;
}

/* ========== Pattern D: Complex loop with all patterns ========== */
static NOINLINE void pattern_d_combo(volatile int *arr, int size) {
    struct bitfield_struct bs = {0};
    
    for (volatile int i = 0; i < size; i++) {
        /* MEM with complex index calculation */
        int idx1 = (i * 3) % size;
        int idx2 = (i * 5) % size;
        
        /* ZERO_EXTRACT from array values */
        bs.f1 = (arr[idx1] & 0x1F);
        bs.f3 = (arr[idx2] & 0x3FF);
        
        /* STRICT_LOW_PART via inline asm on byte from array */
        char byte_val = (char)arr[idx1];
        asm volatile (
            "incb %0"
            : "=q"(byte_val)
            : "0"(byte_val)
            : "cc"
        );
        
        /* SUBREG store back to array */
        short *ps = (short*)&arr[idx2];
        *ps = (short)byte_val;
        
        /* More MEM with pointer arithmetic */
        volatile int *ptr = arr + (i & 0x3);
        *ptr = *ptr + 1;
    }
}

/* ========== Main driver ========== */
int main(int argc, char **argv) {
    /* Use argc to bound loops, preventing infinite loops in analysis */
    int iterations = (argc > 1) ? (argc & 0x7) : 3;
    
    /* Volatile to force resource tracking */
    volatile int counter = 0;
    volatile int sum = 0;
    
    /* Initialize data structures */
    struct bitfield_struct bs1 = {0}, bs2 = {0};
    volatile int array[16];
    
    for (int i = 0; i < 16; i++) {
        array[i] = i * 7;
    }
    
    /* Main loop calling pattern functions */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call each pattern function with volatile-derived arguments */
        pattern_a_zxt_mem(&bs1, i & 0x3);
        
        pattern_b_slp_subreg(i + counter);
        
        pattern_c_mixed(&bs1, &bs2, i + argc);
        
        pattern_d_combo(array, 16);
        
        /* Update volatile variables to prevent dead code elimination */
        counter++;
        sum += bs1.f1 + bs2.f2 + array[i & 0xF];
    }
    
    /* Final dummy use of results */
    asm volatile ("" : : "r"(sum), "r"(counter));
    
    return 0;
}
