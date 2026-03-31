/* test_resource.c - Test program for GCC resource.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* ========== ZERO_EXTRACT Patterns ========== */

/* Pattern 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Multiple extractions with different widths and positions */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 (8 bits) */
        unsigned int extract1 = (source >> 4) & 0xFF;
        
        /* Extract bits 16-23 (8 bits) */
        unsigned int extract2 = (source >> 16) & 0xFF;
        
        /* Extract bits 8-19 (12 bits) */
        unsigned int extract3 = (source >> 8) & 0xFFF;
        
        /* Combine extractions to create dependency */
        result = extract1 + extract2 - extract3;
        
        /* Modify source to create varying patterns */
        source = (source * 1103515245U + 12345U) & 0xFFFFFFFFU;
        
        COMPILER_BARRIER();
    }
    
    /* Prevent dead code elimination */
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Pattern 2: Packed struct with bitfields */
struct packed_bitfields {
    unsigned int field_a : 5;
    unsigned int field_b : 11;
    unsigned int field_c : 7;
    unsigned int field_d : 9;
} __attribute__((packed));

void test_zero_extract_struct(void) {
    volatile struct packed_bitfields s = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    s.field_a = 0x1F;
    s.field_b = 0x7FF;
    s.field_c = 0x7F;
    s.field_d = 0x1FF;
    
    for (int i = 0; i < 100; ++i) {
        /* Read bitfields - these should generate ZERO_EXTRACT */
        unsigned int val_a = s.field_a;
        unsigned int val_b = s.field_b;
        unsigned int val_c = s.field_c;
        unsigned int val_d = s.field_d;
        
        /* Write bitfields - may also generate ZERO_EXTRACT in SET_DEST */
        s.field_a = (val_b + i) & 0x1F;
        s.field_b = (val_c * 3) & 0x7FF;
        s.field_c = (val_d >> 2) & 0x7F;
        s.field_d = (val_a ^ val_b) & 0x1FF;
        
        /* Complex bitfield operation */
        s.field_a = ((s.field_b & 0x1F) + (s.field_c & 0x0F)) & 0x1F;
        
        accumulator += s.field_a + s.field_b + s.field_c + s.field_d;
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = accumulator;
}

/* Pattern 3: Bitfield extraction with arithmetic */
void test_zero_extract_arithmetic(void) {
    volatile unsigned int data[4] = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};
    volatile unsigned int results[4] = {0};
    
    for (int iter = 0; iter < 50; ++iter) {
        for (int i = 0; i < 4; ++i) {
            /* Extract varying bit ranges */
            unsigned int val = data[i];
            
            /* Extract and combine multiple fields */
            unsigned int low5 = val & 0x1F;
            unsigned int mid8 = (val >> 5) & 0xFF;
            unsigned int high6 = (val >> 20) & 0x3F;
            
            /* Create dependency chain */
            unsigned int temp = (low5 * mid8) & 0x1FF;
            temp = (temp + high6) & 0x3FF;
            
            /* Conditional extraction */
            if (temp & 1) {
                unsigned int alt_extract = (val >> 10) & 0x3FF;
                temp ^= alt_extract;
            }
            
            results[i] = temp;
            
            /* Rotate data */
            data[i] = (val >> 1) | (val << 31);
        }
        COMPILER_BARRIER();
    }
    
    /* Use results to prevent elimination */
    volatile unsigned int sum = 0;
    for (int i = 0; i < 4; ++i) {
        sum += results[i];
    }
    volatile unsigned int sink __attribute__((unused)) = sum;
}

/* ========== STRICT_LOW_PART Patterns ========== */

/* Pattern 4: Conditional narrow store using bit operations */
void test_strict_low_part_conditional(void) {
    volatile unsigned int registers[8] = {0};
    volatile unsigned int updates[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22};
    
    for (int i = 0; i < 100; ++i) {
        for (int reg = 0; reg < 8; reg++) {
            unsigned int current = registers[reg];
            unsigned int update = updates[reg];
            
            /* Conditional update of low byte only */
            if ((i + reg) & 1) {
                /* Update only low 8 bits, preserve high bits */
                registers[reg] = (current & ~0xFFU) | (update & 0xFFU);
            }
            
            /* Conditional update of low 16 bits */
            if ((i + reg) & 2) {
                registers[reg] = (current & ~0xFFFFU) | ((update * 3) & 0xFFFFU);
            }
            
            /* Update low 4 bits based on condition */
            if (current & 0x100) {
                registers[reg] = (current & ~0xFU) | ((update >> 4) & 0xFU);
            }
            
            updates[reg] = (update * 1664525U + 1013904223U) & 0xFFFFFFFFU;
        }
        COMPILER_BARRIER();
    }
    
    /* Verify something changed */
    volatile unsigned int total = 0;
    for (int reg = 0; reg < 8; reg++) {
        total ^= registers[reg];
    }
    volatile unsigned int sink __attribute__((unused)) = total;
}

/* Pattern 5: Inline assembly for partial register updates */
void test_strict_low_part_asm(void) {
    volatile unsigned int values[4] = {0x12345678, 0x9ABCDEF0, 0x13579BDF, 0x2468ACE0};
    
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 4; j++) {
            unsigned int val = values[j];
            
            /* Inline assembly that operates on partial register */
            unsigned int result;
            asm volatile (
                /* Clear low byte, keep rest */
                "and %0, %1, %2\n\t"
                : "=r" (result)
                : "r" (val), "i" (~0xFFU)
            );
            
            /* Another partial operation */
            unsigned int low_part;
            asm volatile (
                /* Extract and zero-extend low 16 bits */
                "and %0, %1, %2\n\t"
                : "=r" (low_part)
                : "r" (val), "i" (0xFFFFU)
            );
            
            /* Combine results */
            values[j] = result | (low_part << 8);
            
            COMPILER_BARRIER();
        }
    }
    
    /* Use final values */
    volatile unsigned int check = 0;
    for (int j = 0; j < 4; j++) {
        check += values[j];
    }
    volatile unsigned int sink __attribute__((unused)) = check;
}

/* Pattern 6: Mixed operations with memory references */
void test_mixed_operations(void) {
    /* Array to force memory operations */
    volatile unsigned int mem_buffer[128];
    for (int i = 0; i < 128; i++) {
        mem_buffer[i] = i * 0x01010101U;
    }
    
    /* Packed struct for bitfields */
    struct {
        unsigned int header : 8;
        unsigned int data : 20;
        unsigned int footer : 4;
    } __attribute__((packed)) packet;
    
    volatile unsigned int total = 0;
    
    for (int i = 0; i < 100; ++i) {
        /* Memory load (may generate MEM_P) */
        unsigned int base = mem_buffer[i & 127];
        
        /* Bitfield extraction from memory value */
        packet.header = (base >> 24) & 0xFF;
        packet.data = (base >> 4) & 0xFFFFF;
        packet.footer = base & 0xF;
        
        /* Partial store to memory */
        unsigned int idx = (i * 13) & 127;
        unsigned int old_val = mem_buffer[idx];
        
        /* Update only low 3 bytes */
        unsigned int new_low = (packet.data * i) & 0xFFFFFF;
        mem_buffer[idx] = (old_val & 0xFF000000U) | new_low;
        
        /* Switch based on bitfield */
        switch (packet.header & 0x7) {
            case 0: total += packet.data; break;
            case 1: total += packet.footer * 2; break;
            case 2: total ^= packet.data; break;
            case 3: total |= packet.data; break;
            case 4: total &= packet.data; break;
            case 5: total -= packet.data; break;
            case 6: total = (total << 3) | (packet.data & 0x7); break;
            case 7: total = (total >> 2) ^ packet.data; break;
        }
        
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = total;
}

/* ========== Main Driver ========== */

int main(int argc, char *argv[]) {
    unsigned int test_mask = 0xFF; /* Run all tests by default */
    
    /* Parse command line arguments */
    if (argc > 1) {
        test_mask = 0;
        for (int i = 1; i < argc; i++) {
            int test_num = atoi(argv[i]);
            if (test_num >= 1 && test_num <= 6) {
                test_mask |= (1U << (test_num - 1));
            }
        }
    }
    
    volatile unsigned int final_result = 0;
    
    /* Execute selected tests */
    if (test_mask & 0x01) {
        test_zero_extract_volatile();
        final_result += 1;
    }
    if (test_mask & 0x02) {
        test_zero_extract_struct();
        final_result += 2;
    }
    if (test_mask & 0x04) {
        test_zero_extract_arithmetic();
        final_result += 4;
    }
    if (test_mask & 0x08) {
        test_strict_low_part_conditional();
        final_result += 8;
    }
    if (test_mask & 0x10) {
        test_strict_low_part_asm();
        final_result += 16;
    }
    if (test_mask & 0x20) {
        test_mixed_operations();
        final_result += 32;
    }
    
    /* Print result to prevent optimization */
    printf("Test result: %u\n", final_result);
    
    return 0;
}
