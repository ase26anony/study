/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass (resource.cc lines 282-290). It creates code
   that should generate ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and
   complex MEM expressions during compilation. */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int v_flag = 1;
volatile int v_index = 0;
volatile int v_mask = 0xFF;

/* Global variables for memory pattern generation */
int global_array[256];
struct BitFieldStruct {
    unsigned int low8 : 8;
    unsigned int mid8 : 8;
    unsigned int high16 : 16;
} bf_struct;

/* Union for SUBREG pattern generation */
union MixedSize {
    int64_t full;
    int32_t half[2];
    int16_t quarter[4];
    int8_t eighth[8];
} data_union;

/* ========== ZERO_EXTRACT Patterns ========== */
/* Bit-field extraction using struct member access */
unsigned int extract_bitfield_volatile(volatile unsigned int *ptr) {
    /* This should generate ZERO_EXTRACT for the bit-field access */
    unsigned int val = *ptr;
    return (val >> 8) & 0xFF;  /* Potential ZERO_EXTRACT */
}

/* Direct bit-field struct access */
unsigned int extract_from_struct(void) {
    bf_struct.low8 = 0xAB;
    bf_struct.mid8 = 0xCD;
    bf_struct.high16 = 0x1234;
    
    /* Taking address and accessing bit-field may create ZERO_EXTRACT */
    unsigned int *ptr = (unsigned int*)&bf_struct;
    return (*ptr >> 16) & 0xFFFF;  /* Another potential ZERO_EXTRACT */
}

/* ========== STRICT_LOW_PART Patterns ========== */
/* Writing only low part of a variable */
void write_low_part_32(volatile uint32_t *dest, uint8_t value) {
    /* This pattern may generate STRICT_LOW_PART */
    *dest = (*dest & ~0xFF) | value;
}

/* Using smaller type assignment through pointer cast */
void write_low_part_cast(uint32_t *dest, uint16_t value) {
    /* Cast to smaller pointer type may create STRICT_LOW_PART */
    *(uint16_t*)dest = value;
}

/* ========== SUBREG Patterns ========== */
/* Accessing parts of larger types through unions */
int32_t subreg_via_union(void) {
    data_union.full = 0x0123456789ABCDEFLL;
    
    /* Access different sized parts - may generate SUBREG */
    int32_t result = data_union.half[0] + data_union.half[1];
    result += data_union.quarter[0] + data_union.quarter[2];
    
    return result;
}

/* Mixed-size operations */
int64_t mixed_size_ops(int64_t a, int32_t b) {
    /* Operations mixing sizes may create SUBREG */
    int64_t temp = a + b;  /* b may need SUBREG promotion */
    return temp * 2;
}

/* ========== Complex MEM Patterns ========== */
/* Memory access with complex addressing */
int complex_mem_access(int *base, int idx1, int idx2, int idx3) {
    /* Complex addressing calculation */
    return base[(idx1 * 3 + idx2 * 7 + idx3 * 11) % 256];
}

/* Struct with array and pointer arithmetic */
struct Container {
    int data[100];
    int extra;
};

int struct_mem_access(struct Container *c, int offset) {
    /* Multiple memory operations with addressing */
    int sum = 0;
    sum += c->data[offset];
    sum += c->data[offset + 1];
    sum += c->data[offset + 2];
    
    /* Additional memory reference */
    c->extra = sum;
    
    return sum;
}

/* ========== Combined Function ========== */
/* Function that combines multiple patterns with control flow */
int combined_patterns(int iterations) {
    int result = 0;
    volatile int counter = 0;
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    data_union.full = 0;
    bf_struct.low8 = 0;
    
    struct Container container;
    for (int i = 0; i < 100; i++) {
        container.data[i] = i * 2;
    }
    container.extra = 0;
    
    /* Loop with conditional execution of different patterns */
    for (int i = 0; i < iterations; i++) {
        counter++;
        
        if (v_flag & 0x1) {
            /* ZERO_EXTRACT pattern */
            volatile unsigned int extract_src = 0x12345678;
            result ^= extract_bitfield_volatile(&extract_src);
            result ^= extract_from_struct();
        }
        
        if (v_flag & 0x2) {
            /* STRICT_LOW_PART pattern */
            volatile uint32_t low_part_target = 0x87654321;
            write_low_part_32(&low_part_target, i & 0xFF);
            result += low_part_target;
            
            uint32_t cast_target = 0xDEADBEEF;
            write_low_part_cast(&cast_target, i & 0xFFFF);
            result += cast_target;
        }
        
        if (v_flag & 0x4) {
            /* SUBREG pattern */
            result += subreg_via_union();
            result += mixed_size_ops(i, i * 2);
        }
        
        if (v_flag & 0x8) {
            /* Complex MEM pattern */
            int idx = (i * 17) % 256;
            result += complex_mem_access(global_array, idx, idx+1, idx+2);
            result += struct_mem_access(&container, i % 50);
        }
        
        /* Change v_flag periodically to alter control flow */
        if (i % 7 == 0) {
            v_flag = (v_flag << 1) | ((v_flag >> 3) & 1);
        }
    }
    
    return result;
}

/* ========== Main Function ========== */
int main(void) {
    printf("Starting RTL pattern generation test...\n");
    
    /* Initialize volatile variables */
    v_flag = 0x5;  /* Start with patterns 1 and 3 enabled */
    v_index = 42;
    v_mask = 0x3F;
    
    /* Run combined patterns multiple times */
    int final_result = 0;
    
    /* Multiple calls with different parameters */
    final_result += combined_patterns(10);
    v_flag = 0xA;  /* Switch to patterns 2 and 4 */
    final_result += combined_patterns(15);
    v_flag = 0xF;  /* Enable all patterns */
    final_result += combined_patterns(20);
    
    /* Additional individual pattern calls */
    volatile unsigned int extract_test = 0x89ABCDEF;
    final_result += extract_bitfield_volatile(&extract_test);
    
    uint32_t low_test = 0x11223344;
    write_low_part_32(&low_test, 0x99);
    final_result += low_test;
    
    final_result += subreg_via_union();
    
    /* Complex memory access */
    final_result += complex_mem_access(global_array, 10, 20, 30);
    
    printf("Final result: %d\n", final_result);
    
    /* Return result to prevent optimization */
    return final_result & 0xFF;
}
