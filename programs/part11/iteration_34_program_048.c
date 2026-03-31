/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer (ZERO_EXTRACT) */
volatile unsigned int test1_bitfield_extract(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns to increase chance of ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int extracted = (source >> 4) & 0xFF;
        COMPILER_BARRIER();
        
        /* Extract bits 8-15 with different width */
        extracted |= ((source >> 8) & 0x3F) << 8;
        COMPILER_BARRIER();
        
        /* Extract bits 16-23 */
        extracted |= ((source >> 16) & 0x7F) << 16;
        COMPILER_BARRIER();
        
        /* Extract bits 0-3 */
        extracted |= (source & 0x0F) << 24;
        COMPILER_BARRIER();
        
        result += extracted;
        source = (source * 1103515245 + 12345) & 0xFFFFFFFF; /* Change source */
    }
    
    return result;
}

/* Test 2: Packed struct with bitfields (ZERO_EXTRACT) */
volatile unsigned int test2_packed_struct(void) {
    /* Packed struct with various bitfield widths */
    struct __attribute__((packed)) BitFields {
        unsigned int a : 5;
        unsigned int b : 11;
        unsigned int c : 7;
        unsigned int d : 9;
    };
    
    volatile struct BitFields s = {0};
    volatile unsigned int accumulator = 0;
    
    /* Initialize with pattern */
    s.a = 0x1F;
    s.b = 0x7FF;
    s.c = 0x7F;
    s.d = 0x1FF;
    
    for (int i = 0; i < 100; ++i) {
        /* Read bitfields - these often generate ZERO_EXTRACT */
        unsigned int val1 = s.a;
        unsigned int val2 = s.b;
        unsigned int val3 = s.c;
        unsigned int val4 = s.d;
        
        COMPILER_BARRIER();
        
        /* Write back with transformations */
        s.a = (val2 + i) & 0x1F;
        s.b = (val3 ^ val4) & 0x7FF;
        s.c = (val1 * 3) & 0x7F;
        s.d = (val2 - val3) & 0x1FF;
        
        accumulator += val1 + val2 + val3 + val4;
        COMPILER_BARRIER();
    }
    
    return accumulator;
}

/* Test 3: Complex bitfield operations with arrays (ZERO_EXTRACT) */
volatile unsigned int test3_bitfield_array(void) {
    /* Array of packed structs */
    struct __attribute__((packed)) Element {
        unsigned int low : 6;
        unsigned int mid : 10;
        unsigned int high : 16;
    };
    
    volatile struct Element arr[32];
    volatile unsigned int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 32; ++i) {
        arr[i].low = i & 0x3F;
        arr[i].mid = (i * 7) & 0x3FF;
        arr[i].high = (i * 13) & 0xFFFF;
    }
    
    /* Process array with bitfield operations */
    for (int iter = 0; iter < 50; ++iter) {
        for (int i = 0; i < 31; ++i) {
            /* Complex bitfield manipulation */
            unsigned int temp_low = arr[i].low;
            unsigned int temp_mid = arr[i+1].mid;
            unsigned int temp_high = arr[i].high;
            
            COMPILER_BARRIER();
            
            /* Operations that may generate ZERO_EXTRACT */
            arr[i].low = (temp_mid >> 3) & 0x3F;
            arr[i].mid = (temp_high ^ temp_low) & 0x3FF;
            arr[i].high = ((temp_low << 8) | (temp_mid & 0xFF)) & 0xFFFF;
            
            sum += temp_low + temp_mid + temp_high;
            COMPILER_BARRIER();
        }
    }
    
    return sum;
}

/* Test 4: Conditional partial store (STRICT_LOW_PART) */
volatile unsigned int test4_strict_low_part(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned int mask = 0;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 200; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (i & 1) {
            /* Update only low 8 bits */
            data = (data & ~0xFF) | ((i * 3) & 0xFF);
        } else {
            /* Update only low 16 bits */
            data = (data & ~0xFFFF) | ((i * 5) & 0xFFFF);
        }
        
        COMPILER_BARRIER();
        
        /* Another pattern for partial update */
        mask = (i < 100) ? 0xFF : 0xFFFF;
        data = (data & ~mask) | ((data * 7) & mask);
        
        COMPILER_BARRIER();
        
        result += data;
    }
    
    return result;
}

/* Test 5: Inline assembly for partial register updates (STRICT_LOW_PART) */
volatile unsigned int test5_inline_asm_partial(void) {
    volatile unsigned int reg = 0x87654321;
    volatile unsigned int temp;
    
    for (int i = 0; i < 100; ++i) {
        /* Inline assembly that hints at partial register updates */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (reg), "i" (0xFF)
        );
        
        COMPILER_BARRIER();
        
        /* Another partial update pattern */
        asm volatile (
            "and %0, %1, %2\n\t"
            : "=r" (temp)
            : "r" (reg), "i" (0xFFFF)
        );
        
        COMPILER_BARRIER();
        
        /* Manual partial update that may generate STRICT_LOW_PART */
        unsigned int low_part = temp & 0xFF;
        reg = (reg & ~0xFF) | low_part;
        
        COMPILER_BARRIER();
        
        /* Update with shifting */
        reg = (reg & ~0xFF00) | ((temp << 8) & 0xFF00);
        
        COMPILER_BARRIER();
    }
    
    return reg;
}

/* Test 6: Mixed operations with memory references */
volatile unsigned int test6_mixed_operations(void) {
    volatile unsigned int array[64];
    volatile unsigned int registers[8] = {0};
    volatile unsigned int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 64; ++i) {
        array[i] = i * 0x01010101;
    }
    
    for (int i = 0; i < 8; ++i) {
        registers[i] = i * 0x11111111;
    }
    
    /* Mixed operations to trigger various RTL patterns */
    for (int iter = 0; iter < 50; ++iter) {
        for (int i = 0; i < 63; ++i) {
            /* Memory operation */
            unsigned int mem_val = array[i];
            
            /* Bitfield extraction (ZERO_EXTRACT) */
            unsigned int extracted = (mem_val >> (i % 24)) & ((1U << 8) - 1);
            
            /* Partial store to register (STRICT_LOW_PART potential) */
            registers[i % 8] = (registers[i % 8] & ~0xFF) | (extracted & 0xFF);
            
            /* Another memory operation */
            array[i+1] = registers[i % 8] + extracted;
            
            COMPILER_BARRIER();
            
            result += mem_val + extracted;
        }
    }
    
    return result;
}

/* Test 7: Switch statement with bitfield-derived values */
volatile unsigned int test7_switch_bitfield(void) {
    struct __attribute__((packed)) Control {
        unsigned int opcode : 4;
        unsigned int mode : 2;
        unsigned int size : 3;
        unsigned int data : 23;
    };
    
    volatile struct Control ctrl = {0};
    volatile unsigned int output = 0;
    
    ctrl.opcode = 0xA;
    ctrl.mode = 0x2;
    ctrl.size = 0x5;
    ctrl.data = 0x123456;
    
    for (int i = 0; i < 200; ++i) {
        /* Update bitfields */
        ctrl.opcode = (ctrl.opcode + 1) & 0xF;
        ctrl.data = (ctrl.data * 3) & 0x7FFFFF;
        
        /* Switch on bitfield-derived value */
        switch (ctrl.opcode) {
            case 0x0:
                output += ctrl.data & 0xFF;
                /* Partial update */
                ctrl.data = (ctrl.data & ~0xFF) | ((i * 2) & 0xFF);
                break;
            case 0x1:
                output += (ctrl.data >> 8) & 0xFF;
                ctrl.data = (ctrl.data & ~0xFF00) | ((i * 3) << 8);
                break;
            case 0x2:
                output += ctrl.size;
                ctrl.size = (ctrl.size + 1) & 0x7;
                break;
            case 0x3:
                output += ctrl.mode;
                ctrl.mode = (ctrl.mode + 1) & 0x3;
                break;
            default:
                /* Bitfield extraction */
                unsigned int extracted = (ctrl.data >> ctrl.size) & ((1U << ctrl.opcode) - 1);
                output += extracted;
                /* Partial store */
                ctrl.data = (ctrl.data & ~0xFFFF) | (extracted & 0xFFFF);
                break;
        }
        
        COMPILER_BARRIER();
    }
    
    return output;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    volatile unsigned int final_result = 0;
    
    /* Run tests based on command line arguments or all by default */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && atoi(argv[1]) == 1)) {
        final_result += test1_bitfield_extract();
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && atoi(argv[1]) == 2)) {
        final_result += test2_packed_struct();
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && atoi(argv[1]) == 3)) {
        final_result += test3_bitfield_array();
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && atoi(argv[1]) == 4)) {
        final_result += test4_strict_low_part();
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && atoi(argv[1]) == 5)) {
        final_result += test5_inline_asm_partial();
    }
    
    if (run_all || strstr(argv[0], "test6") || (argc > 1 && atoi(argv[1]) == 6)) {
        final_result += test6_mixed_operations();
    }
    
    if (run_all || strstr(argv[0], "test7") || (argc > 1 && atoi(argv[1]) == 7)) {
        final_result += test7_switch_bitfield();
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %u\n", final_result);
    
    return 0;
}
