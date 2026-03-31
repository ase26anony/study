/* Target RTL patterns for resource.cc coverage:
   - ZERO_EXTRACT: volatile bit-field assignments
   - STRICT_LOW_PART: inline assembly with byte operations
   - SUBREG: type punning with different-sized accesses
   - MEM: complex addressing modes with arrays
*/

#include <stddef.h>

/* Prevent optimization from removing our patterns */
static volatile int global_counter = 0;

/* Function A: ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_zero_extract_mem(volatile int iter) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 7;
        volatile unsigned int field3 : 3;
    } bfs;
    
    /* Array with complex addressing for MEM patterns */
    static volatile int mem_array[32][16];
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bfs.field1 = iter & 0x1F;
    bfs.field2 = (iter >> 5) & 0x7F;
    bfs.field3 = (iter >> 12) & 0x7;
    
    /* MEM pattern with complex addressing */
    int idx1 = iter % 32;
    int idx2 = (iter * 7) % 16;
    
    /* Multiple MEM accesses with addressing calculations */
    volatile int val = mem_array[idx1][idx2];
    mem_array[(idx1 + 1) % 32][(idx2 + 3) % 16] = val + bfs.field1;
    
    /* More complex MEM addressing */
    volatile int *ptr = &mem_array[0][0];
    ptr += idx1 * 16 + idx2;
    volatile int val2 = *ptr;
    *(ptr + 8) = val2 + bfs.field2;
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_strict_low_part_subreg(volatile int iter) {
    /* Variables for STRICT_LOW_PART via inline assembly */
    volatile unsigned char byte_var = iter & 0xFF;
    volatile unsigned short short_var = iter & 0xFFFF;
    
    /* STRICT_LOW_PART pattern: inline assembly modifying only part of register */
    /* Byte operation on char variable */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(byte_var) 
        : "0"(byte_var)
        : "cc"
    );
    
    /* Another byte operation with different constraint */
    asm volatile (
        "subb $2, %0\n\t"
        : "=r"(byte_var)
        : "0"(byte_var)
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different-sized accesses */
    volatile int int_var = iter;
    
    /* Access int as short (HI mode) - generates SUBREG */
    volatile short *short_ptr = (volatile short*)&int_var;
    *short_ptr = (short)(iter + 0x1234);
    
    /* Access int as char (QI mode) - generates SUBREG */
    volatile char *char_ptr = (volatile char*)&int_var;
    char_ptr[1] = (char)(iter + 0x56);
    
    /* Mixed-size operations to encourage SUBREG usage */
    volatile long long large_var = iter;
    volatile int *int_from_ll = (volatile int*)&large_var;
    int_from_ll[0] = byte_var;
    int_from_ll[1] = short_var;
}

/* Function C: Mixed patterns with ternary operator */
static void __attribute__((noinline))
pattern_mixed_complex(volatile int iter) {
    /* Complex struct with bit-fields at different offsets */
    struct mixed_struct {
        volatile unsigned int header : 8;
        volatile unsigned int data : 20;
        volatile unsigned int trailer : 4;
        volatile int array[8];
    } ms;
    
    /* Initialize with ternary affecting addressing */
    volatile int use_bitfield = iter & 1;
    
    /* ZERO_EXTRACT with conditional addressing */
    if (use_bitfield) {
        ms.header = iter & 0xFF;
        ms.data = (iter >> 8) & 0xFFFFF;
    } else {
        ms.trailer = iter & 0xF;
    }
    
    /* MEM with complex index calculation using ternary */
    int array_idx = (iter & 1) ? (iter % 4) : ((iter * 3) % 8);
    ms.array[array_idx] = iter;
    
    /* Pointer arithmetic with type punning (SUBREG potential) */
    volatile char *byte_ptr = (volatile char*)&ms.array[0];
    byte_ptr[array_idx * sizeof(int) + 1] = iter & 0xFF;
    
    /* Additional MEM pattern with pointer chasing */
    volatile int *ptr1 = &ms.array[0];
    volatile int *ptr2 = ptr1 + array_idx;
    volatile int *ptr3 = ptr2 + (iter & 3);
    volatile int final_val = *ptr3;
    
    /* Use the value to prevent elimination */
    global_counter += final_val;
}

/* Helper with loop to increase RTL complexity */
static void __attribute__((noinline))
pattern_loop_helper(volatile int start, volatile int end) {
    volatile int local_sum = 0;
    
    /* Loop creates multiple instances of patterns */
    for (volatile int i = start; i < end; i = i + 1) {
        /* Alternate between patterns */
        if (i & 1) {
            pattern_zero_extract_mem(i);
        } else if (i & 2) {
            pattern_strict_low_part_subreg(i);
        } else {
            pattern_mixed_complex(i);
        }
        
        /* Force resource tracking across iterations */
        local_sum += i;
    }
    
    global_counter += local_sum;
}

/* Main function with volatile counters and calls */
int main(int argc, char *argv[]) {
    /* Use argc to bound loops (prevents infinite loops in analysis) */
    volatile int iterations = (argc > 1) ? 10 : 5;
    
    /* Initialize some volatile data */
    volatile int data_array[64];
    for (volatile int i = 0; i < 64; i = i + 1) {
        data_array[i] = i * 3;
    }
    
    /* Call pattern functions in sequence */
    pattern_loop_helper(0, iterations);
    
    /* More direct calls with different parameters */
    for (volatile int j = 0; j < iterations; j = j + 1) {
        pattern_zero_extract_mem(j + data_array[j % 64]);
        pattern_strict_low_part_subreg(j * 2);
        pattern_mixed_complex(j * 3);
    }
    
    /* Final operation using results */
    volatile int final_result = global_counter;
    
    /* The following would cause UB if run, but is valid for compilation */
    /* volatile int *uninit_ptr = (volatile int*)(final_result & 0xFFF); */
    /* volatile int dummy = *uninit_ptr; */  /* Commented: would crash if executed */
    
    return final_result & 1;  /* Ensure return value depends on patterns */
}
