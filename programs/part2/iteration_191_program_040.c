/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass during optimization. It creates operations that
   generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and complex MEM_P
   expressions in the RTL intermediate representation. */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create unpredictable branches */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global variables for memory access patterns */
volatile unsigned int global_bitfield = 0xDEADBEEF;
int global_array[256];
struct ComplexStruct {
    int32_t full;
    int16_t parts[4];
    unsigned int bits : 8;
    unsigned int more_bits : 8;
} global_struct;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Pattern 1: Direct bitfield extraction using shift and mask */
int extract_byte(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT: extract bits 8-15 */
    return (*p >> 8) & 0xFF;
}

/* Pattern 2: Bitfield structure member access */
int extract_bitfield(struct ComplexStruct *s) {
    /* Accessing bitfield members often creates ZERO_EXTRACT */
    return s->bits + s->more_bits;
}

/* Pattern 3: Multiple extractions with different ranges */
unsigned int extract_multiple(volatile unsigned int *p) {
    unsigned int result = 0;
    /* Extract 4-bit nibbles from different positions */
    result |= (*p & 0xF);           /* bits 0-3 */
    result |= ((*p >> 4) & 0xF) << 4;  /* bits 4-7 */
    result |= ((*p >> 16) & 0xFF) << 8; /* bits 16-23 */
    return result;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Pattern 1: Writing to low part of a larger variable */
void set_low_byte(volatile unsigned int *p, unsigned char v) {
    /* This pattern often generates STRICT_LOW_PART */
    *p = (*p & ~0xFF) | v;
}

/* Pattern 2: Cast to smaller type assignment */
void set_low_half(int32_t *p, int16_t v) {
    /* Writing to half of a 32-bit value */
    *(int16_t*)p = v;
}

/* Pattern 3: Inline assembly that modifies partial register (x86-specific) */
void set_low_asm(volatile uint32_t *p, uint16_t v) {
    /* This inline asm writes only to AX (low 16 bits of EAX/RAX) */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (*p)
        : "r" (v)
        : "eax"
    );
}

/* ==================== SUBREG patterns ==================== */

/* Pattern 1: Union for type punning */
union TypePun {
    int64_t full;
    int32_t halves[2];
    int16_t quarters[4];
    int8_t bytes[8];
};

int32_t access_via_subreg(union TypePun *u) {
    /* Accessing parts through different views creates SUBREG */
    u->quarters[1] = 0x1234;
    u->bytes[3] = 0xAB;
    return u->halves[0];
}

/* Pattern 2: Pointer casting between different sizes */
int32_t cast_access(int64_t *large) {
    /* Casting pointer to different size access */
    int32_t result = *(int32_t*)large;
    *(int16_t*)((char*)large + 2) = 0x5678;
    return result;
}

/* Pattern 3: Array access with mixed types */
int mixed_array_access(void) {
    int64_t big_array[4] = {0};
    /* Access 64-bit array as 32-bit elements */
    ((int32_t*)big_array)[1] = 0xDEAD;
    ((int32_t*)big_array)[3] = 0xBEEF;
    return big_array[0] + big_array[1];
}

/* ==================== Complex MEM_P patterns ==================== */

/* Pattern 1: Complex array indexing with multiple dimensions */
int complex_array_index(int *base, int i, int j, int k) {
    /* Multi-dimensional access with non-trivial addressing */
    return base[(i * 64 + j * 8 + k) & 0xFF];
}

/* Pattern 2: Struct with array member and computed offset */
int struct_array_access(struct ComplexStruct *s, int index) {
    /* Access array within struct with bounds checking */
    if (index >= 0 && index < 4) {
        return s->parts[index] + s->full;
    }
    return 0;
}

/* Pattern 3: Pointer arithmetic with multiple operations */
int pointer_arithmetic(int *base, int offset1, int offset2) {
    /* Complex address calculation */
    int *ptr = base + offset1;
    ptr += offset2 * 2;
    ptr -= offset1 / 2;
    return *ptr + ptr[offset2];
}

/* ==================== Combined function with control flow ==================== */

/* Main test function that combines all patterns */
int combined_test(int iterations) {
    union TypePun pun;
    int result = 0;
    volatile int i;
    
    /* Initialize */
    pun.full = 0x0123456789ABCDEFull;
    global_struct.full = 0xCAFEBABE;
    global_struct.bits = 0x42;
    global_struct.more_bits = 0x24;
    
    for (i = 0; i < iterations; i++) {
        /* Unpredictable control flow based on volatile */
        if (v_flag1) {
            /* ZERO_EXTRACT patterns */
            result ^= extract_byte(&global_bitfield);
            result += extract_bitfield(&global_struct);
            result ^= extract_multiple(&global_bitfield);
            
            /* Modify global_bitfield to change future extractions */
            global_bitfield = (global_bitfield * 1103515245 + 12345) & 0xFFFFFFFF;
        }
        
        if (v_flag2 || (i % 3 == 0)) {
            /* STRICT_LOW_PART patterns */
            set_low_byte((volatile unsigned int*)&global_struct.full, i & 0xFF);
            set_low_half(&global_struct.full, (i * 7) & 0xFFFF);
            
            #ifdef __x86_64__
            set_low_asm((volatile uint32_t*)&global_struct.parts[1], (i * 13) & 0xFFFF);
            #endif
        }
        
        if (i % 2 == 0) {
            /* SUBREG patterns */
            result += access_via_subreg(&pun);
            result ^= cast_access(&pun.full);
            result += mixed_array_access();
            
            /* Rotate the union value */
            pun.full = (pun.full << 13) | (pun.full >> (64 - 13));
        }
        
        /* Complex MEM_P patterns */
        result += complex_array_index(global_array, i & 0x3F, (i >> 6) & 0x3, (i >> 8) & 0x1);
        result += struct_array_access(&global_struct, i & 0x3);
        result += pointer_arithmetic(global_array, i & 0x7F, (i * 3) & 0x7F);
        
        /* Update array for next iteration */
        global_array[i & 0xFF] = result;
        
        /* Occasionally flip volatile flags */
        if (i % 7 == 0) {
            v_flag1 = !v_flag1;
        }
        if (i % 11 == 0) {
            v_flag2 = !v_flag2;
        }
    }
    
    return result;
}

/* ==================== Helper functions ==================== */

void initialize_globals(void) {
    /* Initialize array with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Initialize struct */
    global_struct.full = 0xDEADBEEF;
    for (int i = 0; i < 4; i++) {
        global_struct.parts[i] = (i * 0x1111) & 0xFFFF;
    }
    global_struct.bits = 0xAB;
    global_struct.more_bits = 0xCD;
}

/* ==================== Main function ==================== */

int main(void) {
    int final_result;
    
    /* Initialize all global data */
    initialize_globals();
    
    /* Run the combined test multiple times with different iteration counts */
    final_result = combined_test(100);
    final_result ^= combined_test(50);
    final_result += combined_test(75);
    
    /* Use volatile counter to affect control flow */
    v_counter = final_result & 0xFF;
    
    /* One more pass with volatile-dependent iterations */
    final_result += combined_test(v_counter + 10);
    
    /* Ensure result is used */
    printf("Final result: %d\n", final_result);
    
    return final_result & 0xFF;
}
