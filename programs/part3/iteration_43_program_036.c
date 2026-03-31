/* test_resource_coverage.c
 * 
 * This program is designed to generate specific RTL patterns that trigger
 * the uncovered lines in resource.cc (lines 282-290) during GCC compilation.
 * The patterns target ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM RTL expressions.
 * 
 * Compile with: gcc -O2 -m32 -fno-strict-aliasing -fdump-rtl-all -c test_resource_coverage.c
 * Or for more aggressive optimization: gcc -O3 -m32 -funroll-loops -fno-strict-aliasing -c test_resource_coverage.c
 */

#include <stddef.h>
#include <string.h>

/* ========== PATTERN 1: ZERO_EXTRACT + MEM ========== */
/* ZERO_EXTRACT: volatile bit-field assignments
 * MEM: complex addressing modes with pointer arithmetic
 */
struct bitfield_struct {
    volatile unsigned int f1 : 5;
    volatile unsigned int f2 : 7;
    volatile unsigned int f3 : 3;
    volatile unsigned int f4 : 17;
};

static void __attribute__((noinline)) pattern_zero_extract_mem(struct bitfield_struct *arr, int idx1, int idx2) {
    /* Complex MEM addressing: arr[idx1 + idx2].f1 */
    volatile struct bitfield_struct *ptr = arr + idx1 + idx2;
    
    /* ZERO_EXTRACT: assignment to volatile bit-field */
    ptr->f1 = (idx1 & 0x1F);
    
    /* More complex addressing with multiple indices */
    ptr = &arr[idx1 * 2 - idx2];
    ptr->f2 = (idx2 & 0x7F);
    
    /* Nested pointer arithmetic */
    volatile struct bitfield_struct **pptr = &ptr;
    (*pptr)->f3 = (idx1 ^ idx2) & 0x07;
}

/* ========== PATTERN 2: STRICT_LOW_PART + SUBREG ========== */
/* STRICT_LOW_PART: inline assembly with byte operations
 * SUBREG: type punning between different-sized types
 */
static void __attribute__((noinline)) pattern_strict_low_part_subreg(volatile int *base, int offset) {
    /* SUBREG pattern: type punning between int and short */
    int value = *base + offset;
    short *ps = (short *)&value;
    
    /* Access through different-sized type generates SUBREG */
    *ps = (short)(value & 0xFFFF);
    
    /* Another SUBREG pattern: char access */
    char *pc = (char *)&value;
    pc[1] = (char)(offset & 0xFF);
    
    /* STRICT_LOW_PART: inline assembly modifying only low byte */
    char byte_var = (char)value;
    asm volatile (
        "addb $1, %0\n\t"
        "subb $1, %0"
        : "=q"(byte_var) 
        : "0"(byte_var)
        : "cc"
    );
    
    /* Mixed-size operations to encourage more SUBREGs */
    short half_var = (short)value;
    asm volatile (
        "addw $1, %0\n\t"
        "rorw $2, %0"
        : "+r"(half_var)
        :
        : "cc"
    );
    
    /* Store back through SUBREG */
    ps = (short *)((char *)base + offset);
    *ps = half_var;
}

/* ========== PATTERN 3: COMPLEX MIXED PATTERNS ========== */
/* Combines all patterns in a single complex expression */
static void __attribute__((noinline)) pattern_mixed(volatile int *mem, struct bitfield_struct *bf, int selector) {
    /* Ternary operator with MEM addressing */
    volatile int *addr = (selector & 1) ? 
                         (mem + (selector & 0xF)) : 
                         (mem - (selector & 0x7));
    
    /* MEM with complex addressing */
    int val = addr[(selector >> 4) & 0x3];
    
    /* SUBREG through type punning */
    unsigned char *bytes = (unsigned char *)&val;
    bytes[2] = selector & 0xFF;
    
    /* ZERO_EXTRACT through bit-field in struct */
    volatile struct bitfield_struct *bf_ptr = bf + (selector & 0x3);
    bf_ptr->f4 = val & 0x1FFFF;
    
    /* Another STRICT_LOW_PART asm */
    unsigned short word;
    asm volatile (
        "movw %1, %0\n\t"
        "incb %0\n\t"
        "decb %0"
        : "=q"(word)
        : "r"((unsigned short)val)
        : "cc"
    );
    
    /* Store back through SUBREG */
    *(unsigned short *)addr = word;
}

/* ========== HELPER FUNCTIONS FOR COMPLEX ADDRESSING ========== */
static volatile int* __attribute__((noinline)) get_complex_addr(volatile int *base, int i, int j) {
    /* Complex addressing calculation that won't be optimized away */
    return base + (i * 3 + j * 7) % 16;
}

static void __attribute__((noinline)) modify_through_subreg(volatile long long *big, int idx) {
    /* Access 64-bit value as 32-bit parts on 32-bit target */
    int *half = (int *)big;
    half[idx & 1] = idx;
    
    /* Further break down to 16-bit */
    short *quarter = (short *)big;
    quarter[(idx & 3) + 1] = (short)idx;
}

/* ========== MAIN DRIVER ========== */
int main(int argc, char **argv) {
    volatile int iteration_counter = 0;
    volatile int dummy_sum = 0;
    
    /* Initialize data structures */
    struct bitfield_struct bf_array[16];
    volatile int int_array[32];
    volatile long long big_vals[8];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 16; i++) {
        bf_array[i].f1 = i & 0x1F;
        bf_array[i].f2 = (i * 3) & 0x7F;
        bf_array[i].f3 = (i >> 2) & 0x07;
        bf_array[i].f4 = i * 100;
    }
    
    for (int i = 0; i < 32; i++) {
        int_array[i] = i * 2;
    }
    
    for (int i = 0; i < 8; i++) {
        big_vals[i] = (long long)i << 32 | i;
    }
    
    /* Use argc to bound loops (prevents infinite loops in analysis) */
    int loop_bound = (argc > 1) ? 8 : 4;
    
    /* Main loop that exercises all patterns */
    for (int i = 0; i < loop_bound; i++) {
        iteration_counter++;
        
        /* Pattern 1: ZERO_EXTRACT + MEM */
        pattern_zero_extract_mem(bf_array, i, iteration_counter & 0x7);
        
        /* Pattern 2: STRICT_LOW_PART + SUBREG */
        volatile int *addr1 = get_complex_addr(int_array, i, iteration_counter);
        pattern_strict_low_part_subreg(addr1, iteration_counter);
        
        /* Pattern 3: Mixed patterns */
        pattern_mixed(int_array, bf_array, iteration_counter);
        
        /* Additional SUBREG patterns with 64-bit values */
        modify_through_subreg(big_vals, iteration_counter);
        
        /* Complex MEM addressing in loop */
        for (int j = 0; j < 2; j++) {
            volatile int *addr2 = int_array + (i * 5 + j * 11) % 32;
            volatile int *addr3 = addr2 + (iteration_counter & 0x3);
            
            /* Force MEM references with volatile */
            dummy_sum += *addr2;
            dummy_sum -= *addr3;
            
            /* More type punning for SUBREG */
            short *short_ptr = (short *)addr3;
            *short_ptr = (short)(dummy_sum & 0xFFFF);
        }
        
        /* Prevent optimization of bit-fields */
        dummy_sum += bf_array[i & 0xF].f1;
        dummy_sum += bf_array[i & 0xF].f2;
    }
    
    /* Final dummy operation to prevent dead code elimination */
    volatile int result = dummy_sum + iteration_counter;
    
    /* The program doesn't need correct runtime semantics,
     * but we return something to make the compiler happy */
    return (result > 0) ? 0 : 1;
}
