/* test_resource_coverage.c
 * Designed to trigger mark_referenced_resources for:
 * - ZERO_EXTRACT (volatile bit-field assignments)
 * - STRICT_LOW_PART (inline assembly with byte operations)
 * - SUBREG (type punning with different-sized accesses)
 * - MEM (complex addressing modes)
 */

#include <stddef.h>

/* Force no optimization on specific functions */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_BITFIELD volatile

/* Pattern 1: ZERO_EXTRACT + MEM */
NOINLINE static void pattern_zero_extract_mem(int i, int j) {
    /* Struct with volatile bit-fields */
    struct BitFieldStruct {
        VOLATILE_BITFIELD unsigned int f1 : 5;
        VOLATILE_BITFIELD unsigned int f2 : 7;
        VOLATILE_BITFIELD unsigned int f3 : 3;
    };
    
    /* Array of structs to create MEM with addressing */
    static struct BitFieldStruct bfs[16];
    
    /* Complex addressing: base + index + scaled offset */
    struct BitFieldStruct *ptr = &bfs[(i * 3 + j * 2) & 0xF];
    
    /* Multiple volatile bit-field assignments -> ZERO_EXTRACT */
    ptr->f1 = (i & 0x1F);
    ptr->f2 = (j & 0x7F);
    ptr->f3 = ((i + j) & 0x07);
    
    /* Additional MEM with pointer arithmetic */
    volatile int *mem_ptr = (volatile int *)ptr;
    mem_ptr[(i + j) & 0x3] = i * j;
}

/* Pattern 2: STRICT_LOW_PART + SUBREG */
NOINLINE static void pattern_strict_low_part_subreg(volatile int *counter) {
    /* Use char/short types for STRICT_LOW_PART */
    volatile char c = *counter & 0xFF;
    volatile short s = *counter & 0xFFFF;
    
    /* Inline assembly with "=q" constraint for STRICT_LOW_PART */
    /* Byte operation on char */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)
        : "0"(c)
        : "cc"
    );
    
    /* Word operation on short */
    asm volatile (
        "addw $2, %0\n\t"
        : "=r"(s)
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different-sized accesses */
    int combined = 0;
    short *ps = (short *)&combined;
    char *pc = (char *)&combined;
    
    /* Mixed-size accesses to the same memory -> SUBREG in RTL */
    *ps = s;
    pc[2] = c;
    pc[3] = *counter & 0xFF;
    
    /* More SUBREG: access 64-bit as 32-bit on 32-bit target */
    long long big_val = 0x123456789ABCDEF0LL;
    int *p32 = (int *)&big_val;
    p32[0] = *counter;
    p32[1] = *counter + 1;
    
    /* Update counter through pointer */
    *counter = combined + p32[0];
}

/* Pattern 3: Complex expression mixing patterns */
NOINLINE static void pattern_complex_mix(int i, int j, volatile int *out) {
    /* Array with multi-dimensional indexing -> complex MEM */
    volatile int arr[8][8];
    
    /* Ternary operator selecting different addressing modes */
    volatile int *selected_ptr = (i > j) ? 
        (volatile int *)&arr[i & 7][j & 7] : 
        (volatile int *)&arr[j & 7][i & 7];
    
    /* Bit-field in union for potential ZERO_EXTRACT */
    union {
        struct {
            VOLATILE_BITFIELD unsigned int low : 10;
            VOLATILE_BITFIELD unsigned int high : 6;
        } bits;
        volatile unsigned int full;
    } u;
    
    u.full = i * 100 + j;
    
    /* Assignment that could generate multiple RTL patterns */
    *selected_ptr = u.bits.low + (u.bits.high << 10);
    
    /* Additional SUBREG access */
    short *sp = (short *)selected_ptr;
    sp[1] = (short)(*selected_ptr >> 16);
    
    /* Store result */
    *out = *selected_ptr + u.full;
}

/* Pattern 4: Loop-based pattern generator */
NOINLINE static void pattern_loop_based(volatile int iterations) {
    /* Mixed types for SUBREG generation */
    volatile long long ll_data = 0;
    volatile int int_data = 0;
    volatile short short_data = 0;
    volatile char char_data = 0;
    
    for (volatile int k = 0; k < iterations; k++) {
        /* STRICT_LOW_PART via inline assembly on different types */
        if (k & 1) {
            asm volatile (
                "incb %0\n\t"
                : "=q"(char_data)
                : "0"(char_data)
                : "cc"
            );
        } else {
            asm volatile (
                "incw %0\n\t"
                : "=r"(short_data)
                : "0"(short_data)
                : "cc"
            );
        }
        
        /* SUBREG: access long long as smaller types */
        int *p_int = (int *)&ll_data;
        p_int[k & 1] = int_data + k;
        
        /* ZERO_EXTRACT via bit-field in struct */
        struct {
            VOLATILE_BITFIELD unsigned int field : 9;
        } bf;
        bf.field = (int_data + char_data) & 0x1FF;
        
        /* MEM with complex addressing */
        volatile int mem_buffer[16];
        int index = (k * 7) & 0xF;
        mem_buffer[index] = bf.field + p_int[0];
        
        /* Update variables to prevent optimization */
        int_data = mem_buffer[index];
        ll_data += int_data;
    }
}

/* Main driver that calls all patterns */
int main(int argc, char *argv[]) {
    /* Use argc to bound loops, preventing infinite loops in analysis */
    volatile int iterations = (argc > 1) ? 10 : 5;
    volatile int result = 0;
    
    /* Initialize some volatile data */
    volatile int counter = 0;
    volatile int array[8] = {0};
    
    /* Main loop calling pattern functions */
    for (volatile int i = 0; i < iterations; i++) {
        for (volatile int j = 0; j < iterations; j++) {
            /* Call each pattern function */
            pattern_zero_extract_mem(i, j);
            
            pattern_strict_low_part_subreg(&counter);
            
            pattern_complex_mix(i, j, &result);
            
            /* Update array with complex addressing -> MEM */
            array[(i * 3 + j * 7) & 0x7] = result + counter;
        }
        
        /* Call loop-based pattern */
        pattern_loop_based(i + 1);
        
        /* Prevent dead code elimination */
        result += counter + array[i & 0x7];
    }
    
    /* Final dummy operation using all results */
    volatile int final = result + counter;
    
    /* The program doesn't need correct runtime semantics,
     * but we return something to make it valid */
    return (final > 0) ? 0 : 1;
}
