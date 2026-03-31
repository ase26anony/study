/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
volatile unsigned int test1_result = 0;
void test_bitfield_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int output = 0;
    
    /* Multiple extraction patterns that may generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 (8 bits) */
        unsigned int extract1 = (source >> 4) & ((1U << 8) - 1);
        
        /* Extract bits 16-23 with variable width */
        unsigned int width = 8;
        unsigned int extract2 = (source >> 16) & ((1U << width) - 1);
        
        /* Combine extractions */
        output = extract1 + extract2;
        
        /* Modify source to create variation */
        source = source * 1103515245 + 12345;
        
        COMPILER_BARRIER();
    }
    
    test1_result = output;
    COMPILER_BARRIER();
}

/* Test 2: Packed struct with bitfields */
struct __attribute__((packed)) packed_bitfields {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
};

volatile unsigned int test2_result = 0;
void test_packed_struct_bitfields(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int accumulator = 0;
    
    s.field_a = 0x1F;
    s.field_b = 0x7FF;
    s.field_c = 0x7F;
    s.field_d = 0x1FF;
    
    /* Complex bitfield operations in loop */
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - may generate ZERO_EXTRACT */
        unsigned int val_a = s.field_a;
        unsigned int val_b = s.field_b;
        unsigned int val_c = s.field_c;
        unsigned int val_d = s.field_d;
        
        /* Write back with transformations - may generate both ZERO_EXTRACT and STRICT_LOW_PART */
        s.field_a = (val_b + i) & 0x1F;          /* 5 bits */
        s.field_b = (val_c ^ val_d) & 0x7FF;     /* 11 bits */
        s.field_c = (val_a * 3) & 0x7F;          /* 7 bits */
        s.field_d = (val_b - val_c) & 0x1FF;     /* 9 bits */
        
        accumulator += s.field_a + s.field_b + s.field_c + s.field_d;
        
        COMPILER_BARRIER();
    }
    
    test2_result = accumulator;
    COMPILER_BARRIER();
}

/* Test 3: Conditional partial store operations */
volatile unsigned int test3_result = 0;
void test_conditional_partial_store(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned int temp = 0;
    
    for (int i = 0; i < 100; ++i) {
        unsigned int condition = i & 1;
        
        if (condition) {
            /* Update only low 8 bits - may generate STRICT_LOW_PART */
            data = (data & ~0xFF) | ((i + 0x55) & 0xFF);
        } else {
            /* Update only bits 8-15 */
            data = (data & ~0xFF00) | (((i * 3) & 0xFF) << 8);
        }
        
        /* Another pattern: conditional merge of low 16 bits */
        if (i % 3 == 0) {
            unsigned short low_part = (i * 7) & 0xFFFF;
            data = (data & 0xFFFF0000) | low_part;
        }
        
        temp += data;
        COMPILER_BARRIER();
    }
    
    test3_result = temp;
    COMPILER_BARRIER();
}

/* Test 4: Inline assembly for partial register updates */
volatile unsigned int test4_result = 0;
void test_inline_asm_partial(void) {
    volatile unsigned int reg_var = 0x87654321;
    volatile unsigned int asm_out = 0;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline asm that operates on partial register */
        unsigned int input = reg_var + i;
        unsigned int output;
        
        /* Assembly that might be optimized to STRICT_LOW_PART */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3"
            : "=r" (output)
            : "r" (input), "i" (0x0000FFFF), "r" (i << 16)
            : /* no clobber */
        );
        
        /* Another pattern with byte extraction */
        unsigned char byte_val;
        asm volatile (
            "ubfx %0, %1, #8, #8"
            : "=r" (byte_val)
            : "r" (reg_var)
            : /* no clobber */
        );
        
        reg_var = output ^ (byte_val << 24);
        asm_out += reg_var;
        
        COMPILER_BARRIER();
    }
    
    test4_result = asm_out;
    COMPILER_BARRIER();
}

/* Test 5: Mixed operations with memory and bitfields */
volatile unsigned int test5_result = 0;
void test_mixed_memory_bitfields(void) {
    /* Array with volatile elements to force memory operations */
    volatile unsigned int mem_array[64];
    for (int i = 0; i < 64; ++i) {
        mem_array[i] = i * 0x01010101;
    }
    
    /* Register variable to encourage register allocation */
    register unsigned int reg_acc asm("r12") = 0;
    
    /* Packed struct on stack */
    struct __attribute__((packed)) {
        unsigned int head : 12;
        unsigned int middle : 16;
        unsigned int tail : 4;
    } local_bitfield = {0};
    
    /* Complex loop mixing operations */
    for (int i = 0; i < 32; ++i) {
        /* Memory read with bitfield extraction */
        unsigned int mem_val = mem_array[i];
        local_bitfield.head = (mem_val >> 4) & 0xFFF;
        local_bitfield.middle = (mem_val >> 16) & 0xFFFF;
        
        /* Conditional partial update */
        if (i % 4 == 0) {
            /* Update only tail field */
            local_bitfield.tail = (mem_val + i) & 0xF;
        }
        
        /* Switch based on bitfield value */
        switch (local_bitfield.head & 0x7) {
            case 0: reg_acc += mem_array[i + 1]; break;
            case 1: reg_acc += local_bitfield.middle; break;
            case 2: reg_acc -= local_bitfield.tail; break;
            default: reg_acc ^= mem_val; break;
        }
        
        /* Write back to memory with partial update */
        mem_array[i] = (mem_array[i] & 0xFF000000) | 
                      (local_bitfield.head << 12) | 
                      local_bitfield.middle;
        
        COMPILER_BARRIER();
    }
    
    test5_result = reg_acc;
    COMPILER_BARRIER();
}

/* Main driver function */
int main(int argc, char *argv[]) {
    unsigned int final_result = 0;
    
    /* Run tests based on command line arguments */
    int run_all = (argc == 1); /* Run all if no arguments */
    
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && strcmp(argv[1], "1") == 0)) {
        test_bitfield_extract_volatile();
        final_result += test1_result;
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && strcmp(argv[1], "2") == 0)) {
        test_packed_struct_bitfields();
        final_result += test2_result;
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && strcmp(argv[1], "3") == 0)) {
        test_conditional_partial_store();
        final_result += test3_result;
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && strcmp(argv[1], "4") == 0)) {
        test_inline_asm_partial();
        final_result += test4_result;
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && strcmp(argv[1], "5") == 0)) {
        test_mixed_memory_bitfields();
        final_result += test5_result;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: 0x%08X\n", final_result);
    
    return 0;
}
