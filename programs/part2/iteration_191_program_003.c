/* test_resource_patterns.c
 * 
 * This program is designed to generate RTL patterns that will trigger
 * the uncovered lines in resource.cc (lines 282-290) during compiler
 * optimization passes. It creates:
 * 1. ZERO_EXTRACT patterns via bit-field operations
 * 2. STRICT_LOW_PART patterns via partial register writes
 * 3. SUBREG patterns via type punning and mixed-size accesses
 * 4. Complex MEM patterns via addressing modes with arithmetic
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create control flow */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_counter = 0;

/* Global variables for memory operations */
unsigned int global_bitfield = 0x12345678;
uint64_t global_large = 0xFFFFFFFFFFFFFFFF;
int global_array[256];
int global_index = 0;

/* Structs for bit-field and union access */
struct bitfield_struct {
    unsigned int low8 : 8;
    unsigned int mid16 : 16;
    unsigned int high8 : 8;
} bf_var;

union mixed_union {
    uint64_t full;
    uint32_t halves[2];
    uint16_t words[4];
    uint8_t bytes[8];
} mu_var;

/* 1. ZERO_EXTRACT patterns */
int zero_extract_pattern_1(volatile unsigned int *p) {
    /* Multiple extract patterns to increase chances */
    int a = (*p >> 0) & 0xFF;      /* Extract bits 0-7 */
    int b = (*p >> 8) & 0xFFFF;    /* Extract bits 8-23 */
    int c = (*p >> 24) & 0xF;      /* Extract bits 24-27 */
    return a + b + c;
}

int zero_extract_pattern_2(struct bitfield_struct *bf) {
    /* Access bit-fields - may generate ZERO_EXTRACT */
    unsigned int val = bf->low8;
    val += bf->mid16;
    val += bf->high8;
    return val;
}

/* 2. STRICT_LOW_PART patterns */
void strict_low_part_pattern_1(volatile uint32_t *p, uint8_t v) {
    /* Write only low 8 bits */
    *p = (*p & ~0xFF) | v;
}

void strict_lOW_part_pattern_2(uint64_t *p) {
    /* Write to low 16 bits via pointer cast */
    *(uint16_t *)p = 0xABCD;
}

/* 3. SUBREG patterns */
int32_t subreg_pattern_1(union mixed_union *u) {
    /* Access different views of the same data */
    u->words[0] = 0x1234;
    u->words[1] = 0x5678;
    u->words[2] = 0x9ABC;
    u->words[3] = 0xDEF0;
    
    /* Mixed-size accesses */
    uint32_t val = u->halves[0] + u->halves[1];
    return (int32_t)(val + u->bytes[7]);
}

void subreg_pattern_2(uint64_t *data) {
    /* Type punning through pointer casts */
    uint32_t *as_32 = (uint32_t *)data;
    uint16_t *as_16 = (uint16_t *)data;
    uint8_t *as_8 = (uint8_t *)data;
    
    as_32[0] = 0xDEADBEEF;
    as_16[2] = 0xCAFE;
    as_8[6] = 0x42;
}

/* 4. Complex MEM patterns */
int complex_mem_pattern_1(int *base, int idx1, int idx2, int idx3) {
    /* Complex addressing with multiple indices */
    return base[idx1 + idx2 * 4 + idx3 * 16];
}

int complex_mem_pattern_2(struct wrapper {
    int arr[64];
    int pad[16];
} *wp, int a, int b, int c) {
    /* Nested addressing with struct */
    return wp->arr[a + b * 2 + c * 8];
}

/* Main test function that combines all patterns */
int test_all_patterns(int iterations) {
    int result = 0;
    
    /* Initialize data */
    bf_var.low8 = 0xAA;
    bf_var.mid16 = 0xBBBB;
    bf_var.high8 = 0xCC;
    
    mu_var.full = 0;
    
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    struct wrapper w;
    for (int i = 0; i < 64; i++) {
        w.arr[i] = i * 5;
    }
    
    /* Main loop with volatile-controlled flow */
    for (int i = 0; i < iterations; i++) {
        v_counter++;
        
        /* Use volatile flags to create unpredictable control flow */
        if (v_flag1) {
            /* ZERO_EXTRACT patterns */
            result += zero_extract_pattern_1(&global_bitfield);
            result += zero_extract_pattern_2(&bf_var);
            
            /* Modify bitfield for next iteration */
            bf_var.low8 = (bf_var.low8 + 1) & 0xFF;
        }
        
        if (v_flag2 || (v_counter % 3 == 0)) {
            /* STRICT_LOW_PART patterns */
            strict_low_part_pattern_1((volatile uint32_t *)&global_large, 
                                     (uint8_t)(result & 0xFF));
            strict_lOW_part_pattern_2(&global_large);
            
            /* SUBREG patterns */
            result += subreg_pattern_1(&mu_var);
            subreg_pattern_2(&global_large);
        }
        
        /* Always execute MEM patterns */
        int idx1 = (result + i) & 0xF;
        int idx2 = (result * 2) & 0x3;
        int idx3 = (i) & 0x1;
        
        result += complex_mem_pattern_1(global_array, idx1, idx2, idx3);
        result += complex_mem_pattern_2(&w, idx1, idx2, idx3);
        
        /* Toggle volatile flags occasionally */
        if (i % 7 == 0) {
            v_flag1 = !v_flag1;
        }
        if (i % 11 == 0) {
            v_flag2 = !v_flag2;
        }
        
        /* Prevent overflow */
        result &= 0xFFFF;
    }
    
    return result;
}

/* Additional helper functions to increase pass activity */
void helper_function_1(void) {
    /* More bit-field operations */
    volatile struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 10;
        unsigned int d : 14;
    } s;
    
    s.a = 5;
    s.b = 20;
    s.c = 512;
    s.d = 12345;
    
    /* Force use of all fields */
    global_index += s.a + s.b + s.c + s.d;
}

void helper_function_2(void) {
    /* Mixed pointer accesses */
    uint64_t buffer[4];
    uint32_t *p32 = (uint32_t *)buffer;
    uint16_t *p16 = (uint16_t *)buffer;
    
    for (int i = 0; i < 8; i++) {
        p32[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 16; i++) {
        p16[i] = (p16[i] & 0xFF) | 0x100;
    }
}

int main(void) {
    int final_result = 0;
    
    printf("Starting resource pattern test...\n");
    
    /* Call helpers to increase compiler pass activity */
    helper_function_1();
    helper_function_2();
    
    /* Main test with multiple iterations */
    final_result = test_all_patterns(100);
    
    /* Additional test with different parameters */
    v_flag1 = 0;
    v_flag2 = 1;
    final_result += test_all_patterns(50);
    
    printf("Final result: %d\n", final_result);
    
    /* Use result to prevent dead code elimination */
    return final_result & 0xFF;
}
