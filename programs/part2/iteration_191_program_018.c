/* This program is designed to trigger specific RTL patterns in GCC's
   resource tracking pass (resource.cc lines 282-290) by generating
   code that compiles to ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and
   complex MEM expressions. */

#include <stdio.h>
#include <stdint.h>

/* Volatile variables to prevent optimization and create unpredictable control flow */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global arrays and structs for memory operand patterns */
int global_array[256];
struct BitFieldStruct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 12;
    unsigned int field4 : 8;
} bf_struct;

/* Union for SUBREG patterns */
union MixedSizeUnion {
    int64_t large;
    int32_t medium[2];
    int16_t small[4];
    int8_t tiny[8];
} data_union;

/* ========== ZERO_EXTRACT Patterns ========== */
/* Bit-field extraction using struct member access */
unsigned int extract_bitfield_1(void) {
    /* Taking address of bit-field member may generate ZERO_EXTRACT */
    unsigned int val = bf_struct.field2;
    return val * 2;
}

/* Bit-field extraction using shift/mask on volatile pointer */
unsigned int extract_bitfield_2(volatile unsigned int *p) {
    /* Complex shift/mask pattern that might generate ZERO_EXTRACT */
    unsigned int temp = *p;
    return ((temp >> 4) & 0xF) | ((temp >> 12) & 0xF0);
}

/* ========== STRICT_LOW_PART Patterns ========== */
/* Writing only low part of a variable */
void write_low_part_1(volatile unsigned int *p, unsigned char v) {
    /* Pattern that writes only low 8 bits */
    *p = (*p & ~0xFF) | v;
}

/* Using smaller type assignment to create partial write */
void write_low_part_2(void) {
    /* Cast to smaller type might generate STRICT_LOW_PART */
    int32_t x = 0x12345678;
    *(int16_t*)&x = 0xABCD;
    global_array[0] = x; /* Use result to prevent dead code elimination */
}

/* ========== SUBREG Patterns ========== */
/* Accessing parts of larger type through union */
int32_t subreg_via_union(void) {
    data_union.large = 0x1122334455667788ULL;
    /* Access smaller part of the union */
    int32_t result = data_union.medium[1] + data_union.small[0];
    return result;
}

/* Mixed-size operations */
int64_t mixed_size_ops(int64_t a, int32_t b) {
    /* Operations mixing different sizes might generate SUBREG */
    int64_t temp = a + b;  /* b gets extended, might involve SUBREG */
    int32_t part = (int32_t)(temp >> 16);
    return temp * part;
}

/* ========== Complex MEM Patterns ========== */
/* Memory access with complex addressing mode */
int complex_mem_access_1(int *base, int idx1, int idx2, int idx3) {
    /* Complex addressing calculation */
    return base[(idx1 * idx2 + idx3) & 0xFF];
}

/* Struct with array and pointer arithmetic */
struct Container {
    int data[128];
    int extra;
};

int complex_mem_access_2(struct Container *c, volatile int idx) {
    /* Multiple memory accesses with addressing modes */
    int sum = 0;
    sum += c->data[idx % 64];
    sum += c->data[(idx * 3) % 64 + 64];
    return sum + c->extra;
}

/* ========== Combined Function ========== */
/* Function that combines multiple patterns in control flow */
int combined_patterns(int seed) {
    int result = seed;
    
    /* Use volatile flags to create unpredictable control flow */
    if (v_flag1) {
        /* ZERO_EXTRACT pattern */
        volatile unsigned int *vol_ptr = (volatile unsigned int*)&result;
        result += extract_bitfield_2(vol_ptr);
        
        /* STRICT_LOW_PART pattern */
        write_low_part_1((volatile unsigned int*)&result, (seed >> 8) & 0xFF);
    }
    
    if (v_flag2 || (v_counter % 3 == 0)) {
        /* SUBREG pattern */
        result ^= subreg_via_union() & 0xFFFF;
        
        /* Complex MEM pattern */
        result += complex_mem_access_1(global_array, 
                                      result & 0xF, 
                                      (result >> 4) & 0xF,
                                      (result >> 8) & 0xF);
    }
    
    /* More mixed operations */
    result = mixed_size_ops(result, seed * 2) & 0x7FFFFFFF;
    
    return result;
}

/* ========== Main Function ========== */
int main(void) {
    int i, final_result = 0;
    
    /* Initialize data */
    for (i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    bf_struct.field1 = 5;
    bf_struct.field2 = 0xAB;
    bf_struct.field3 = 0x7FF;
    bf_struct.field4 = 0xCD;
    
    data_union.large = 0x8877665544332211ULL;
    
    struct Container container;
    for (i = 0; i < 128; i++) {
        container.data[i] = i * 5 - 3;
    }
    container.extra = 999;
    
    /* Main loop with multiple pattern usages */
    for (v_counter = 0; v_counter < 100; v_counter++) {
        /* Toggle volatile flags */
        v_flag1 = (v_counter % 7) < 4;
        v_flag2 = (v_counter % 5) == 0;
        
        /* Call individual pattern functions */
        int temp = extract_bitfield_1();
        write_low_part_2();
        temp += complex_mem_access_2(&container, v_counter);
        
        /* Use combined function */
        final_result += combined_patterns(temp + v_counter);
        
        /* Ensure all operations have observable effect */
        if (final_result > 1000000) {
            final_result >>= 1;
        }
    }
    
    /* Additional pattern variations */
    volatile unsigned int mem_test = 0xDEADBEEF;
    for (i = 0; i < 10; i++) {
        /* More ZERO_EXTRACT patterns */
        unsigned int bits = (mem_test >> (i * 2)) & 0x3;
        final_result ^= bits << (i * 3);
        
        /* More STRICT_LOW_PART patterns */
        write_low_part_1(&mem_test, final_result & 0xFF);
    }
    
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;
}
