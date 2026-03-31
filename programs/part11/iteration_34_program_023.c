/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* ========== Test 1: Bitfield extraction from volatile integer ========== */
volatile unsigned int test1_result = 0;

void test_bitfield_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int mask = 0x1F;  /* 5 bits */
    volatile unsigned int start = 4;    /* start at bit 4 */
    
    for (int i = 0; i < 100; ++i) {
        /* This should generate ZERO_EXTRACT: extract bits 4-8 */
        unsigned int extracted = (source >> start) & mask;
        test1_result += extracted;
        
        /* Modify source to create variation */
        source = (source << 1) | (source >> 31);
        COMPILER_BARRIER();
    }
}

/* ========== Test 2: Packed struct with bitfields ========== */
struct packed_bitfields {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
} __attribute__((packed));

volatile int test2_result = 0;

void test_packed_struct_bitfields(void) {
    volatile struct packed_bitfields s = {0};
    s.field_a = 0x1F;
    s.field_b = 0x7FF;
    s.field_c = 0x7F;
    s.field_d = 0x1FF;
    
    for (int i = 0; i < 50; ++i) {
        /* Multiple bitfield reads/writes - should generate ZERO_EXTRACT */
        int temp = s.field_b;           /* Read from bitfield */
        s.field_a = temp & 0x1F;        /* Write to bitfield */
        temp = s.field_c;
        s.field_d = (temp + i) & 0x1FF;
        
        test2_result += s.field_a + s.field_b;
        COMPILER_BARRIER();
    }
}

/* ========== Test 3: Complex bitfield arithmetic ========== */
struct control_reg {
    unsigned int enable : 1;
    unsigned int mode : 3;
    unsigned int count : 10;
    unsigned int reserved : 18;
} __attribute__((packed));

volatile unsigned int test3_result = 0;

void test_complex_bitfield_ops(void) {
    volatile struct control_reg reg = {0};
    volatile unsigned int external = 0xABCD;
    
    for (int i = 0; i < 75; ++i) {
        /* Complex operations on bitfields */
        reg.enable = (i & 1);
        reg.mode = (external >> (i % 8)) & 0x7;
        reg.count = (reg.count + reg.mode) & 0x3FF;
        
        /* Switch based on bitfield value */
        switch (reg.mode) {
            case 0: test3_result += 1; break;
            case 1: test3_result += 2; break;
            case 2: test3_result += 3; break;
            default: test3_result += 4; break;
        }
        
        external = (external * 1103515245 + 12345) & 0xFFFF;
        COMPILER_BARRIER();
    }
}

/* ========== Test 4: Conditional partial store (STRICT_LOW_PART pattern) ========== */
volatile unsigned int test4_result = 0;

void test_conditional_partial_store(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned int mask = 0xFF;
    volatile int condition = 1;
    
    for (int i = 0; i < 100; ++i) {
        /* Pattern that may generate STRICT_LOW_PART: conditional update of low byte */
        if (condition) {
            /* Update only low 8 bits, preserve high bits */
            data = (data & ~mask) | ((i & 0xFF) & mask);
        }
        
        /* Alternate pattern using pointer to char */
        volatile unsigned char *byte_ptr = (volatile unsigned char*)&data;
        if (i % 3 == 0) {
            byte_ptr[1] = (i >> 8) & 0xFF;  /* Modify second byte */
        }
        
        test4_result += data;
        condition = !condition;
        COMPILER_BARRIER();
    }
}

/* ========== Test 5: Inline assembly for partial register update ========== */
volatile unsigned int test5_result = 0;

void test_inline_asm_partial(void) {
    volatile unsigned int value = 0x87654321;
    
    for (int i = 0; i < 60; ++i) {
        unsigned int old_value = value;
        
        /* Inline assembly that operates on partial register */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (value)
            : "r" (old_value), "i" (0xFFFF)  /* Keep only low 16 bits */
            : /* No clobbers */
        );
        
        /* Another pattern: clear high bits while preserving low */
        asm volatile (
            "bic %0, %1, %2\n\t"
            : "=r" (value)
            : "r" (value), "i" (0xFFFF0000)
        );
        
        test5_result += value;
        value = (value * 1103515245 + 12345);
        COMPILER_BARRIER();
    }
}

/* ========== Test 6: Mixed operations with memory references ========== */
volatile unsigned int test6_result = 0;

void test_mixed_with_memory(void) {
    volatile unsigned int array[16];
    volatile struct packed_bitfields struct_array[4];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    for (int i = 0; i < 4; ++i) {
        struct_array[i].field_a = i;
        struct_array[i].field_b = i * 64;
    }
    
    register unsigned int reg_var asm("r12") = 0;  /* Encourage register use */
    
    for (int i = 0; i < 80; ++i) {
        /* Mix memory access with bitfield operations */
        unsigned int idx = i & 0xF;
        array[idx] = (array[idx] >> 4) & 0x0F0F0F0F;  /* ZERO_EXTRACT pattern */
        
        /* Bitfield from struct array */
        struct_array[idx & 0x3].field_c = array[idx] & 0x7F;
        
        /* Conditional partial update */
        if (i % 7 == 0) {
            reg_var = (reg_var & ~0xFF) | (i & 0xFF);
        }
        
        test6_result += array[idx] + struct_array[idx & 0x3].field_b;
        COMPILER_BARRIER();
    }
}

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    int run_all = 0;
    int test_to_run = 0;
    
    /* Parse command line arguments */
    if (argc > 1) {
        if (strcmp(argv[1], "all") == 0) {
            run_all = 1;
        } else {
            test_to_run = atoi(argv[1]);
        }
    } else {
        run_all = 1;  /* Default: run all tests */
    }
    
    /* Initialize volatile results */
    test1_result = test2_result = test3_result = 0;
    test4_result = test5_result = test6_result = 0;
    
    /* Run selected tests */
    if (run_all || test_to_run == 1) {
        test_bitfield_extract_volatile();
        printf("Test 1 completed: result = %u\n", test1_result);
    }
    
    if (run_all || test_to_run == 2) {
        test_packed_struct_bitfields();
        printf("Test 2 completed: result = %d\n", test2_result);
    }
    
    if (run_all || test_to_run == 3) {
        test_complex_bitfield_ops();
        printf("Test 3 completed: result = %u\n", test3_result);
    }
    
    if (run_all || test_to_run == 4) {
        test_conditional_partial_store();
        printf("Test 4 completed: result = %u\n", test4_result);
    }
    
    if (run_all || test_to_run == 5) {
        test_inline_asm_partial();
        printf("Test 5 completed: result = %u\n", test5_result);
    }
    
    if (run_all || test_to_run == 6) {
        test_mixed_with_memory();
        printf("Test 6 completed: result = %u\n", test6_result);
    }
    
    /* Aggregate and print final result to prevent dead code elimination */
    unsigned int final_result = 
        test1_result + test2_result + test3_result + 
        test4_result + test5_result + test6_result;
    
    printf("Final aggregated result: %u\n", final_result);
    
    return 0;
}
