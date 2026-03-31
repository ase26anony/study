/* resource_patterns.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdint.h>
#include <string.h>

/* Force no inlining to preserve patterns */
#define NOINLINE __attribute__((noinline))

/* Pattern 1: ZERO_EXTRACT + MEM */
NOINLINE static void pattern_zero_extract_mem(volatile int *counter) {
    /* Struct with volatile bit-field for ZERO_EXTRACT */
    struct bitfield_struct {
        volatile unsigned int field1 : 5;
        volatile unsigned int field2 : 3;
        volatile unsigned int field3 : 8;
    } bs;
    
    /* Array with complex addressing for MEM */
    volatile int arr[16][8];
    volatile int *ptr;
    
    /* ZERO_EXTRACT pattern: assignment to volatile bit-field */
    bs.field1 = (*counter) & 0x1F;
    bs.field2 = (*counter >> 5) & 0x7;
    bs.field3 = (*counter >> 8) & 0xFF;
    
    /* MEM pattern with complex addressing */
    int idx = *counter;
    ptr = &arr[idx & 0xF][idx & 0x7];
    
    /* Combine: use bit-field value in memory access */
    *ptr = bs.field1 + bs.field2;
    
    /* Additional MEM with pointer arithmetic */
    volatile int *p = (volatile int*)&bs;
    p += (idx & 0x3);
    *p = idx;  /* This may generate MEM with displacement */
}

/* Pattern 2: STRICT_LOW_PART + SUBREG */
NOINLINE static void pattern_strict_low_part_subreg(volatile int *counter) {
    volatile char c = *counter & 0xFF;
    volatile short s = *counter & 0xFFFF;
    volatile int i = *counter;
    
    /* STRICT_LOW_PART: inline assembly modifying only low part */
    asm volatile (
        "addb $1, %0\n\t"
        : "=q"(c)  /* =q constraint for byte-addressable register */
        : "0"(c)
        : "cc"
    );
    
    /* Another STRICT_LOW_PART pattern with different size */
    asm volatile (
        "incw %0\n\t"
        : "=r"(s)
        : "0"(s)
        : "cc"
    );
    
    /* SUBREG pattern: type punning with different sizes */
    int32_t val32 = i;
    int16_t *p16 = (int16_t*)&val32;  /* Type punning - may generate SUBREG */
    *p16 = (int16_t)(s + 1);          /* Write to low 16 bits */
    
    /* More SUBREG: access different parts of larger type */
    int64_t big_val = (int64_t)i * 1000;
    int32_t *p32 = (int32_t*)&big_val;
    p32[0] = c;      /* Low 32 bits */
    p32[1] = s;      /* High 32 bits (on 64-bit) or wraps (on 32-bit) */
    
    /* Use results to prevent elimination */
    *counter = val32 + p32[0];
}

/* Pattern 3: Mixed patterns with ternary and complex expressions */
NOINLINE static void pattern_mixed(volatile int *counter, volatile int *sum) {
    /* Volatile struct with bit-fields */
    struct mixed_struct {
        volatile unsigned int flag : 1;
        volatile unsigned int value : 15;
        volatile unsigned int pad : 16;
    } ms;
    
    /* Array for MEM patterns */
    static volatile int data[256];
    
    /* Initialize with counter */
    ms.flag = (*counter) & 0x1;
    ms.value = (*counter >> 1) & 0x7FFF;
    
    /* Complex addressing with ternary */
    volatile int *addr;
    int idx = *counter;
    
    /* Ternary selects different addressing modes */
    addr = (idx & 0x1) ? 
           &data[idx & 0xFF] :           /* Direct index */
           &data[(idx * 3) & 0xFF];      /* Computed index */
    
    /* Assignment that could involve multiple transformations */
    *addr = ms.value;
    
    /* Additional MEM with scaled index */
    volatile short *short_ptr = (volatile short*)&data[0];
    short_ptr[idx & 0x7F] = (short)ms.value;
    
    /* Update sum to prevent elimination */
    *sum += *addr + ms.flag;
}

/* Pattern 4: Complex loop with all patterns */
NOINLINE static void pattern_complex(volatile int iterations) {
    volatile int temp = 0;
    volatile char byte_var = 0;
    volatile short short_var = 0;
    
    /* Mixed-size array for SUBREG patterns */
    volatile uint32_t mixed_array[32];
    volatile uint16_t *p16 = (volatile uint16_t*)mixed_array;
    volatile uint8_t *p8 = (volatile uint8_t*)mixed_array;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* ZERO_EXTRACT: bit-field in struct */
        struct {
            volatile uint32_t low : 12;
            volatile uint32_t high : 12;
        } bf;
        bf.low = i & 0xFFF;
        bf.high = (i >> 12) & 0xFFF;
        
        /* STRICT_LOW_PART: inline asm on byte */
        asm volatile (
            "orb $0x1, %0\n\t"
            : "+q"(byte_var)
            :
            : "cc"
        );
        
        /* SUBREG: access 32-bit array as 16-bit */
        int idx = i & 0x1F;
        p16[idx] = (uint16_t)(bf.low + byte_var);
        
        /* MEM: complex addressing with multiple indices */
        mixed_array[(idx * 3) & 0x1F] = bf.high + p16[idx];
        
        /* More SUBREG: access as 8-bit */
        p8[(i * 5) & 0x7F] = (uint8_t)temp;
        
        /* Update temp to prevent elimination */
        temp += mixed_array[idx] + byte_var;
    }
}

/* Main function that exercises all patterns */
int main(int argc, char **argv) {
    volatile int counter = 0;
    volatile int sum = 0;
    volatile int iterations = 10;  /* Prevent infinite loops */
    
    /* Use argc to vary patterns slightly */
    if (argc > 1) {
        iterations = 20;
    }
    
    /* Loop to generate repeated RTL patterns */
    for (counter = 0; counter < iterations; counter++) {
        /* Call each pattern function */
        pattern_zero_extract_mem(&counter);
        pattern_strict_low_part_subreg(&counter);
        pattern_mixed(&counter, &sum);
        
        /* Periodically call complex pattern */
        if ((counter & 0x3) == 0) {
            pattern_complex(5);
        }
        
        /* Additional volatile operations to force resource tracking */
        volatile int *ptr = &sum;
        *ptr += counter;
        
        /* MEM with auto-increment pattern */
        volatile int arr[8];
        volatile int *p = arr;
        for (volatile int j = 0; j < 4; j++) {
            *p++ = counter + j;  /* MEM with post-increment */
        }
    }
    
    /* Final dummy use to prevent elimination */
    asm volatile ("" : : "r"(sum));
    
    return sum != 0;  /* Non-deterministic return to prevent optimization */
}

/* Additional helper to ensure patterns aren't optimized away */
NOINLINE static void dummy_use(volatile void *ptr) {
    asm volatile ("" : : "r"(ptr));
}
