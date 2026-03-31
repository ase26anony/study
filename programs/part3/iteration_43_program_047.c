/* test_resource_coverage.c
 * Generates RTL patterns to trigger uncovered lines in resource.cc:
 * ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM patterns
 */

#include <stddef.h>
#include <string.h>

/* Force compiler to generate specific RTL patterns */

/* Function A: ZERO_EXTRACT and MEM patterns */
static void __attribute__((noinline)) 
pattern_zero_extract_mem(volatile int *counter) {
    /* Struct with volatile bit-fields for ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int f1 : 5;
        volatile unsigned int f2 : 3;
        volatile unsigned int f3 : 8;
    } bs;
    
    /* Array with complex addressing for MEM patterns */
    static volatile int arr[16][16];
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bs.f1 = (*counter) & 0x1F;
    bs.f2 = ((*counter) >> 5) & 0x7;
    bs.f3 = ((*counter) >> 8) & 0xFF;
    
    /* MEM pattern with complex addressing */
    int idx1 = (*counter) & 0xF;
    int idx2 = ((*counter) >> 4) & 0xF;
    
    /* Force MEM reference with addressing calculation */
    volatile int val = arr[idx1][idx2];
    
    /* Use the value to prevent elimination */
    bs.f1 = (bs.f1 + val) & 0x1F;
}

/* Function B: STRICT_LOW_PART and SUBREG patterns */
static void __attribute__((noinline))
pattern_strict_low_part_subreg(volatile int *counter) {
    volatile unsigned short hs = *counter & 0xFFFF;
    volatile unsigned char hb = *counter & 0xFF;
    
    /* STRICT_LOW_PART pattern: inline asm modifying low part */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(hb)   /* =q constraint for byte-addressable register */
        : "0"(hb)    /* matching input constraint */
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with different operation */
    asm volatile (
        "orb $0x10, %0\n\t"
        : "=q"(hb)
        : "0"(hb)
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different sizes */
    volatile int i = *counter;
    
    /* Cast to smaller type pointer - may generate SUBREG */
    volatile short *ps = (volatile short *)&i;
    *ps = hs;  /* Assignment through SUBREG */
    
    /* Another SUBREG pattern with char */
    volatile char *pc = (volatile char *)&i;
    pc[1] = hb;  /* Modify middle byte */
    
    /* Use the modified value */
    *counter = i + hs;
}

/* Function C: Mixed patterns with ternary and complex expressions */
static void __attribute__((noinline))
pattern_mixed_complex(volatile int *counter, volatile int selector) {
    /* Struct with bit-fields at different positions */
    struct mixed_struct {
        volatile unsigned int a : 3;
        volatile unsigned int b : 7;
        volatile unsigned int c : 10;
        volatile unsigned int padding : 12;
    } ms;
    
    /* Array with pointer arithmetic */
    static volatile int buffer[256];
    volatile int *ptr = buffer;
    
    /* Complex addressing calculation */
    int offset = (*counter) & 0xFF;
    
    /* Ternary operator selecting different addressing modes */
    volatile int *target_ptr = (selector & 1) ? 
                               (ptr + offset) : 
                               (buffer + (offset ^ 0x55));
    
    /* MEM pattern with computed address */
    volatile int fetched = *target_ptr;
    
    /* ZERO_EXTRACT based on selector */
    if (selector & 2) {
        ms.a = fetched & 0x7;
    } else {
        ms.b = (fetched >> 3) & 0x7F;
    }
    
    /* Another MEM reference with scaled index */
    volatile int idx = (offset * 3) & 0xFF;
    ms.c = buffer[idx] & 0x3FF;
    
    /* Update counter with mixed values */
    *counter = ms.a + ms.b + ms.c + fetched;
}

/* Helper function with loop containing patterns */
static void __attribute__((noinline))
pattern_loop_helper(volatile int iterations) {
    volatile int counter = 0;
    volatile int selector = 0x55AA;
    
    /* Loop to generate repeated RTL patterns */
    for (volatile int i = 0; i < iterations; i++) {
        /* Alternate between different pattern functions */
        if (i & 1) {
            pattern_zero_extract_mem(&counter);
        } else {
            pattern_strict_low_part_subreg(&counter);
        }
        
        /* Periodically call mixed pattern */
        if ((i & 3) == 0) {
            pattern_mixed_complex(&counter, selector);
        }
        
        /* Modify selector to vary patterns */
        selector ^= (i << 1);
        
        /* Prevent loop elimination */
        asm volatile("" : "+r"(counter) : : "memory");
    }
}

/* Main function that drives pattern generation */
int main(int argc, char *argv[]) {
    /* Use argc to bound iterations for safety */
    volatile int iterations = (argc > 1) ? 10 : 5;
    
    /* Initialize some volatile data */
    volatile int data_array[32];
    for (volatile int i = 0; i < 32; i++) {
        data_array[i] = i * i;
    }
    
    /* Call pattern functions multiple times */
    pattern_loop_helper(iterations);
    
    /* Additional direct calls with different parameters */
    volatile int counter1 = 0x12345678;
    volatile int counter2 = 0x87654321;
    
    pattern_zero_extract_mem(&counter1);
    pattern_strict_low_part_subreg(&counter2);
    pattern_mixed_complex(&counter1, counter2);
    
    /* One more complex pattern in main */
    {
        /* SUBREG pattern with 64-bit type on 32-bit target */
        volatile long long big_val = 0x1122334455667788LL;
        volatile int *p32 = (volatile int *)&big_val;
        
        /* Access different parts - generates SUBREG */
        volatile int low_part = p32[0];
        volatile int high_part = p32[1];
        
        /* Use in bit-field assignment (ZERO_EXTRACT) */
        struct {
            volatile unsigned int low_bits : 12;
            volatile unsigned int high_bits : 12;
        } parts;
        
        parts.low_bits = low_part & 0xFFF;
        parts.high_bits = (high_part >> 4) & 0xFFF;
        
        /* MEM pattern with pointer arithmetic */
        volatile int *arr_ptr = data_array;
        arr_ptr += (parts.low_bits & 0x1F);
        volatile int final_val = *arr_ptr + parts.high_bits;
        
        /* Use final_val to prevent elimination */
        asm volatile("" : "+r"(final_val) : : "memory");
    }
    
    return 0;
}
