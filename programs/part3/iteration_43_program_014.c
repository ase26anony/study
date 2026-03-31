/* resource_patterns.c
 * 
 * This program is designed to generate specific RTL patterns that trigger
 * the uncovered lines in GCC's resource.cc (lines 282-290) during compilation.
 * The code uses volatile variables, inline assembly, and type punning to
 * force generation of ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM
 * RTL expressions.
 */

#include <stddef.h>

/* Prevent inlining to ensure functions remain separate in RTL */
#define NOINLINE __attribute__((noinline))

/* Pattern 1: ZERO_EXTRACT and MEM
 * Uses volatile bit-fields and pointer arithmetic to generate
 * ZERO_EXTRACT for bit-field assignment and MEM with addressing modes.
 */
struct bitfield_struct {
    volatile unsigned int f1 : 5;
    volatile unsigned int f2 : 3;
    volatile unsigned int f3 : 8;
    volatile unsigned int padding : 16;
};

static NOINLINE void pattern_zextract_mem(struct bitfield_struct *s, 
                                          volatile int *arr, 
                                          int i, int j) {
    /* Complex addressing mode for MEM */
    volatile int *ptr = &arr[i * 10 + j];
    
    /* ZERO_EXTRACT from volatile bit-field assignment */
    s->f1 = (*ptr) & 0x1F;
    s->f2 = (*ptr >> 5) & 0x7;
    
    /* More complex MEM addressing with multiple indices */
    volatile int val = arr[(i + j) * 3 % 10];
    s->f3 = val & 0xFF;
}

/* Pattern 2: STRICT_LOW_PART and SUBREG
 * Uses inline assembly with byte constraints and type punning
 * to generate STRICT_LOW_PART and SUBREG RTL.
 */
static NOINLINE void pattern_strictlow_subreg(volatile int *result) {
    volatile char c = 42;
    volatile short s = 1024;
    volatile int i = 0x12345678;
    
    /* STRICT_LOW_PART: inline assembly modifying only low byte */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)   /* =q constraint for byte-addressable register */
        : "0"(c)
        : "cc"
    );
    
    /* SUBREG: type punning through pointer casts */
    short *ps = (short *)&i;
    *ps = s;  /* This generates SUBREG store */
    
    /* Another SUBREG pattern with char access */
    char *pc = (char *)&i;
    pc[1] = c;
    
    /* Complex expression mixing sizes */
    *result = (int)(*ps) + (int)(pc[2]);
}

/* Pattern 3: Mixed patterns with ternary and complex addressing
 * Combines array indexing, conditional selection, and bit-field operations
 * to trigger multiple RTL transformations.
 */
static NOINLINE void pattern_mixed(volatile int *arr, 
                                   int idx1, int idx2, 
                                   struct bitfield_struct *bf) {
    /* Ternary operator selecting different addressing modes */
    volatile int *ptr = (idx1 > idx2) ? &arr[idx1 * 2] : &arr[idx2 * 3 + 1];
    
    /* MEM with complex addressing */
    volatile int val = ptr[(idx1 + idx2) % 5];
    
    /* Conditional bit-field assignment (potential ZERO_EXTRACT) */
    if (val > 0) {
        bf->f1 = val & 0x1F;
        /* SUBREG-like access through byte pointer */
        char *byte_ptr = (char *)&val;
        bf->f3 = byte_ptr[0] + byte_ptr[1];
    } else {
        bf->f2 = (-val) & 0x7;
    }
    
    /* Additional MEM pattern with double indirection */
    volatile int **pptr = &ptr;
    volatile int val2 = **pptr;
    bf->f3 = (bf->f3 + val2) & 0xFF;
}

/* Helper function to create more complex call graphs */
static NOINLINE void helper_complex_mem(volatile int *arr, int n) {
    for (int i = 0; i < n; i++) {
        /* Multi-dimensional array-like addressing */
        volatile int val = arr[i] + arr[i * 2 % n] + arr[(i * 3 + 1) % n];
        
        /* Force MEM usage with computation */
        arr[i] = val % 100;
    }
}

/* Main function that drives all patterns */
int main(int argc, char **argv) {
    /* Use argc to bound loops - prevents infinite loops in analysis */
    int iterations = (argc > 1) ? 10 : 5;
    
    /* Volatile counters to prevent optimization */
    volatile int counter = 0;
    volatile int sum = 0;
    
    /* Arrays and structs with volatile elements */
    volatile int array[100];
    struct bitfield_struct bf = {0};
    
    /* Initialize array with volatile writes */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    /* Main loop calling pattern functions */
    for (volatile int i = 0; i < iterations; i++) {
        int idx1 = (i * 2) % 50;
        int idx2 = (i * 3 + 1) % 50;
        
        /* Call pattern 1: ZERO_EXTRACT and MEM */
        pattern_zextract_mem(&bf, (int *)array, idx1, idx2);
        
        /* Call pattern 2: STRICT_LOW_PART and SUBREG */
        pattern_strictlow_subreg(&counter);
        
        /* Call pattern 3: Mixed patterns */
        pattern_mixed((int *)array, idx1, idx2, &bf);
        
        /* Call helper for additional MEM complexity */
        helper_complex_mem((int *)array, 20);
        
        /* Accumulate results to prevent dead code elimination */
        sum += counter + bf.f1 + bf.f2 + bf.f3;
        
        /* Modify array to create new addressing patterns */
        array[idx1] = sum;
        array[idx2] = counter;
        
        counter++;
    }
    
    /* Final dummy operation using all results */
    volatile int final_result = sum + counter + bf.f1;
    
    /* The program doesn't need correct runtime semantics,
     * but we return something to make compilation happy */
    return (final_result > 0) ? 0 : 1;
}

/* Additional global variables to create more RTL contexts */
volatile unsigned int global_bitfield = 0;

/* Another function using global volatile bit-field */
void NOINLINE use_global_bitfield(void) {
    /* Cast to struct with bit-fields for ZERO_EXTRACT */
    struct local_bf {
        volatile unsigned int a : 4;
        volatile unsigned int b : 4;
        volatile unsigned int c : 8;
    } *p = (struct local_bf *)&global_bitfield;
    
    p->a = 3;
    p->b = 7;
    p->c = (global_bitfield >> 4) & 0xFF;
    
    /* STRICT_LOW_PART on global */
    volatile char *cptr = (volatile char *)&global_bitfield;
    asm volatile (
        "orb $0x10, %0"
        : "+q"(*cptr)
        :
        : "cc"
    );
}
