/* test_resource.c - Test program to cover ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* ========== Test 1: Bitfield extraction from volatile integer ========== */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns that should generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int extracted = (source >> 4) & ((1U << 8) - 1);
        result ^= extracted;  /* XOR to combine results */
        
        /* Extract bits 16-23 with different width */
        extracted = (source >> 16) & 0xFF;
        result += extracted;
        
        /* Extract bits 0-3 */
        extracted = source & 0x0F;
        result |= extracted;
        
        /* Modify source slightly each iteration */
        source = (source * 1103515245U + 12345U) & 0xFFFFFFFFU;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* ========== Test 2: Packed struct with bitfields ========== */
struct __attribute__((packed)) BitfieldStruct {
    unsigned int header : 4;
    unsigned int data   : 20;
    unsigned int footer : 8;
};

struct __attribute__((packed)) MultiBitfield {
    unsigned short field1 : 3;
    unsigned short field2 : 5;
    unsigned short field3 : 8;
    unsigned int   field4 : 17;
    unsigned short field5 : 1;
};

void test_packed_struct_bitfields(void) {
    volatile struct BitfieldStruct bs = {0};
    volatile struct MultiBitfield mb = {0};
    volatile unsigned int accumulator = 0;
    
    bs.header = 0xA;
    bs.data = 0x12345;
    bs.footer = 0xCD;
    
    mb.field1 = 0x5;
    mb.field2 = 0x12;
    mb.field3 = 0xAB;
    mb.field4 = 0x1FFFF;
    mb.field5 = 0x1;
    
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int val1 = bs.data;
        unsigned int val2 = bs.footer;
        
        /* Complex bitfield operation */
        mb.field3 = (mb.field2 + mb.field1) & 0xFF;
        
        /* Extract and combine */
        accumulator += val1;
        accumulator ^= val2;
        accumulator |= mb.field4;
        
        /* Write back to bitfields */
        bs.data = accumulator & 0xFFFFF;
        mb.field2 = (accumulator >> 8) & 0x1F;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = accumulator;
}

/* ========== Test 3: Inline assembly for partial register updates ========== */
void test_asm_partial_store(void) {
    volatile unsigned int reg_var = 0x12345678;
    volatile unsigned int temp;
    
    for (int i = 0; i < 75; ++i) {
        /* Inline asm that operates on partial register - may generate STRICT_LOW_PART */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (reg_var), "i" (0x0000FFFF)
        );
        
        /* Another partial operation */
        asm volatile (
            "and %0, %1, %2\n\t"
            "orr %0, %0, %3\n\t"
            : "=r" (temp)
            : "r" (reg_var), "i" (0xFFFF0000), "r" (0x0000ABCD)
        );
        
        /* Update low 16 bits only */
        unsigned int new_low = (i * 137) & 0xFFFF;
        reg_var = (reg_var & 0xFFFF0000) | new_low;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = reg_var + temp;
}

/* ========== Test 4: Conditional merge operations ========== */
void test_conditional_merge(void) {
    volatile unsigned int value = 0x87654321;
    volatile unsigned int mask;
    volatile int condition = 1;
    
    for (int i = 0; i < 60; ++i) {
        /* Varying mask patterns */
        mask = (i & 1) ? 0xFF : 0xFFFF;
        
        /* Conditional partial update - may generate STRICT_LOW_PART */
        if (condition) {
            unsigned int new_val = (i * 7919) & mask;  /* Prime multiplier */
            value = (value & ~mask) | (new_val & mask);
        }
        
        /* Nested conditional with different mask */
        if ((i % 3) == 0) {
            unsigned int byte_mask = 0xFF00;
            unsigned int update = (i * 65537) & byte_mask;
            value = (value & ~byte_mask) | (update & byte_mask);
        }
        
        /* Toggle condition */
        condition = !condition;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = value;
}

/* ========== Test 5: Mixed operations with memory and control flow ========== */
void test_mixed_operations(void) {
    volatile unsigned int array[16];
    volatile struct BitfieldStruct bs_array[4];
    volatile unsigned int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; ++i) {
        array[i] = i * 0x11111111;
    }
    for (int i = 0; i < 4; ++i) {
        bs_array[i].header = i;
        bs_array[i].data = i * 0xCCCCC;
        bs_array[i].footer = i * 0x33;
    }
    
    /* Register variable to encourage register allocation */
    register unsigned int reg_acc asm ("r12") = 0;
    
    for (int i = 0; i < 40; ++i) {
        /* Memory access combined with bitfield extraction */
        unsigned int idx = i & 0xF;
        unsigned int mem_val = array[idx];
        
        /* Extract bits from memory value */
        unsigned int extracted = (mem_val >> 8) & 0xFFF;  /* 12 bits */
        
        /* Bitfield struct access */
        unsigned int bf_val = bs_array[idx & 0x3].data;
        
        /* Switch based on extracted bits - creates control flow */
        switch (extracted & 0x7) {  /* Use low 3 bits */
            case 0:
                reg_acc = (reg_acc & 0xFFFFFF00) | (bf_val & 0xFF);
                break;
            case 1:
                reg_acc = (reg_acc & 0xFFFF00FF) | ((bf_val & 0xFF) << 8);
                break;
            case 2:
                reg_acc = (reg_acc & 0xFF00FFFF) | ((bf_val & 0xFF) << 16);
                break;
            default:
                reg_acc = (reg_acc & 0x00FFFFFF) | ((bf_val & 0xFF) << 24);
                break;
        }
        
        /* Partial store to memory */
        array[idx] = (array[idx] & 0xFFFF0000) | (reg_acc & 0xFFFF);
        
        result += reg_acc;
        COMPILER_BARRIER();
    }
    
    /* Use result to prevent optimization */
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* ========== Test 6: Pointer-based partial updates ========== */
void test_pointer_partial_updates(void) {
    volatile unsigned int data = 0x89ABCDEF;
    volatile unsigned char *byte_ptr = (volatile unsigned char *)&data;
    volatile unsigned short *short_ptr = (volatile unsigned short *)&data;
    
    for (int i = 0; i < 30; ++i) {
        /* Update through byte pointer - partial store */
        byte_ptr[1] = (i * 13) & 0xFF;
        
        /* Update through short pointer */
        short_ptr[1] = (i * 257) & 0xFFFF;
        
        /* Combine with bit extraction */
        unsigned int low_byte = data & 0xFF;
        unsigned int high_byte = (data >> 24) & 0xFF;
        
        /* Conditional swap of bytes */
        if ((i & 1) == 0) {
            data = (data & 0x00FFFF00) | (low_byte << 24) | high_byte;
        }
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = data;
}

/* ========== Main driver ========== */
int main(int argc, char *argv[]) {
    unsigned int test_mask = 0x3F;  /* Run all tests by default */
    
    /* Parse command line arguments */
    if (argc > 1) {
        test_mask = 0;
        for (int i = 1; i < argc; ++i) {
            int test_num = atoi(argv[i]);
            if (test_num >= 1 && test_num <= 6) {
                test_mask |= (1U << (test_num - 1));
            }
        }
    }
    
    volatile unsigned int final_result = 0;
    
    printf("Running tests with mask: 0x%02X\n", test_mask & 0x3F);
    
    if (test_mask & 0x01) {
        printf("Test 1: Zero extract from volatile\n");
        test_zero_extract_volatile();
        final_result += 1;
    }
    
    if (test_mask & 0x02) {
        printf("Test 2: Packed struct bitfields\n");
        test_packed_struct_bitfields();
        final_result += 2;
    }
    
    if (test_mask & 0x04) {
        printf("Test 3: Inline assembly partial stores\n");
        test_asm_partial_store();
        final_result += 4;
    }
    
    if (test_mask & 0x08) {
        printf("Test 4: Conditional merge operations\n");
        test_conditional_merge();
        final_result += 8;
    }
    
    if (test_mask & 0x10) {
        printf("Test 5: Mixed operations with control flow\n");
        test_mixed_operations();
        final_result += 16;
    }
    
    if (test_mask & 0x20) {
        printf("Test 6: Pointer-based partial updates\n");
        test_pointer_partial_updates();
        final_result += 32;
    }
    
    /* Print final result to prevent dead code elimination */
    printf("Final marker: %u\n", final_result);
    
    return 0;
}
