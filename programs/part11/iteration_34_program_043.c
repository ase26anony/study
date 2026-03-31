/* test_resource.c - Test program to cover ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Packed structs with bitfields for ZERO_EXTRACT generation */
struct packed_bitfields_5_11 {
    unsigned int a:5;
    unsigned int b:11;
    unsigned int c:8;
    unsigned int d:8;
} __attribute__((packed));

struct packed_bitfields_varying {
    unsigned int x:3;
    unsigned int y:7;
    unsigned int z:10;
    unsigned int w:12;
} __attribute__((packed));

/* Global volatile variables to prevent optimization */
volatile unsigned int global_counter = 0;
volatile unsigned int global_result = 0;
volatile unsigned int global_array[256];

/* Test 1: Bitfield extraction from volatile integer */
void test_bitfield_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* Multiple extraction patterns that should generate ZERO_EXTRACT */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 4-11 */
        unsigned int bits_4_11 = (source >> 4) & 0xFF;
        
        /* Extract bits 8-15 with arithmetic */
        unsigned int bits_8_15 = ((source >> 8) + i) & 0xFF;
        
        /* Extract bits 16-23 using mask */
        unsigned int mask = (1U << 8) - 1;
        unsigned int bits_16_23 = (source >> 16) & mask;
        
        /* Combine extractions */
        result = bits_4_11 + bits_8_15 + bits_16_23;
        
        /* Store to volatile array to prevent optimization */
        global_array[i % 256] = result;
        
        COMPILER_BARRIER();
    }
    
    global_result += result;
}

/* Test 2: Packed struct bitfield operations */
void test_packed_struct_bitfields(void) {
    volatile struct packed_bitfields_5_11 s1 = {0};
    volatile struct packed_bitfields_varying s2 = {0};
    
    /* Initialize with pattern */
    s1.a = 0x1F;  /* 5 bits max */
    s1.b = 0x7FF; /* 11 bits max */
    s1.c = 0xFF;  /* 8 bits max */
    s1.d = 0xAA;  /* 8 bits max */
    
    s2.x = 0x7;   /* 3 bits max */
    s2.y = 0x7F;  /* 7 bits max */
    s2.z = 0x3FF; /* 10 bits max */
    s2.w = 0xFFF; /* 12 bits max */
    
    volatile unsigned int sum = 0;
    
    /* Complex bitfield operations in loop */
    for (int i = 0; i < 50; ++i) {
        /* Read bitfields - should generate ZERO_EXTRACT */
        unsigned int val1 = s1.b;  /* 11-bit extraction */
        unsigned int val2 = s2.z;  /* 10-bit extraction */
        
        /* Write with arithmetic - may generate ZERO_EXTRACT in SET_DEST */
        s1.a = (val1 + i) & 0x1F;  /* Mask to 5 bits */
        s2.y = (val2 ^ i) & 0x7F;  /* Mask to 7 bits */
        
        /* Cross-struct operation */
        s1.c = (s2.w >> 4) & 0xFF;
        
        sum += val1 + val2 + s1.a + s2.y + s1.c;
        
        COMPILER_BARRIER();
    }
    
    /* Nested loop for more scheduling opportunities */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            s1.d = (s1.d + s2.x) & 0xFF;
            s2.x = (s2.x + 1) & 0x7;
        }
    }
    
    global_result += sum + s1.d + s2.x;
}

/* Test 3: STRICT_LOW_PART via conditional narrow stores */
void test_strict_low_part_conditional(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned char flag = 1;
    
    for (int i = 0; i < 100; ++i) {
        /* Conditional update of low byte - may generate STRICT_LOW_PART */
        if (flag) {
            /* Update only low 8 bits */
            data = (data & ~0xFF) | ((i + 0xAB) & 0xFF);
        }
        
        /* Conditional update of low 16 bits */
        if (i % 3 == 0) {
            data = (data & ~0xFFFF) | ((data + 0x1234) & 0xFFFF);
        }
        
        /* Update low 4 bits based on condition */
        unsigned int mask = 0xF;
        unsigned int new_low = (i * 7) & mask;
        data = (data & ~mask) | new_low;
        
        /* Store partial result */
        global_array[i % 256] = data & 0xFF;
        
        COMPILER_BARRIER();
        
        /* Toggle flag */
        flag = !flag;
    }
    
    global_result += data;
}

/* Test 4: Inline assembly for partial register updates */
void test_inline_asm_partial_store(void) {
    volatile unsigned int reg_var = 0x87654321;
    volatile unsigned int temp;
    
    for (int i = 0; i < 50; ++i) {
        /* Inline asm that operates on partial register */
        /* The compiler may generate STRICT_LOW_PART for these */
        
        /* Clear low byte */
        asm volatile (
            "and %0, %1, %2"
            : "=r"(temp)
            : "r"(reg_var), "i"(~0xFFU)
        );
        
        /* Set low byte to pattern */
        unsigned int pattern = (i * 3) & 0xFF;
        asm volatile (
            "orr %0, %1, %2"
            : "=r"(reg_var)
            : "r"(temp), "r"(pattern)
        );
        
        /* Extract low 12 bits */
        unsigned int low12;
        asm volatile (
            "and %0, %1, %2"
            : "=r"(low12)
            : "r"(reg_var), "i"(0xFFF)
        );
        
        /* Update middle 16 bits */
        unsigned int mid16 = (reg_var >> 8) & 0xFFFF;
        mid16 = (mid16 + 0x1111) & 0xFFFF;
        reg_var = (reg_var & ~0x00FFFF00) | (mid16 << 8);
        
        global_array[i % 256] = low12 + mid16;
        
        COMPILER_BARRIER();
    }
    
    global_result += reg_var;
}

/* Test 5: Mixed operations with memory references */
void test_mixed_operations_memory(void) {
    volatile unsigned int array[64];
    volatile struct packed_bitfields_5_11 structs[8];
    
    /* Initialize arrays */
    for (int i = 0; i < 64; ++i) {
        array[i] = i * 0x01010101;
    }
    
    for (int i = 0; i < 8; ++i) {
        structs[i].a = i;
        structs[i].b = i * 16;
        structs[i].c = i * 32;
        structs[i].d = i * 64;
    }
    
    volatile unsigned int accumulator = 0;
    
    /* Complex loop with mixed operations */
    for (int i = 0; i < 32; ++i) {
        /* Memory load */
        unsigned int mem_val = array[i];
        
        /* Bitfield extraction from struct */
        unsigned int bf_val = structs[i % 8].b;
        
        /* Extract bits from memory value */
        unsigned int extracted = (mem_val >> (i % 24)) & 0x3FF;  /* 10 bits */
        
        /* Conditional partial store to array */
        if (extracted > bf_val) {
            /* Update only low 16 bits */
            array[i] = (array[i] & ~0xFFFF) | (extracted & 0xFFFF);
        }
        
        /* Update struct bitfield */
        structs[i % 8].a = (extracted + bf_val) & 0x1F;
        
        /* Pointer arithmetic with partial update */
        volatile unsigned char *byte_ptr = (volatile unsigned char *)&array[i];
        byte_ptr[1] = (extracted >> 8) & 0xFF;  /* May generate partial store */
        
        accumulator += extracted + bf_val + array[i] + structs[i % 8].a;
        
        COMPILER_BARRIER();
    }
    
    /* Switch statement based on bitfield value */
    volatile unsigned int switch_var = structs[0].a;
    volatile unsigned int switch_result = 0;
    
    switch (switch_var & 0x7) {  /* Use low 3 bits */
        case 0:
            switch_result = array[0] & 0xFF;
            break;
        case 1:
            switch_result = (array[1] >> 8) & 0xFF;
            break;
        case 2:
            switch_result = (array[2] >> 16) & 0xFF;
            break;
        case 3:
            switch_result = (array[3] >> 24) & 0xFF;
            break;
        default:
            switch_result = 0xDEAD;
            break;
    }
    
    global_result += accumulator + switch_result;
}

/* Test 6: Register variables with bitfield operations */
void test_register_variables(void) {
    /* Use register keyword to encourage register allocation */
    register unsigned int reg1 asm("r8") = 0xABCD1234;
    register unsigned int reg2 asm("r9") = 0x5678EF90;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 25; ++i) {
        /* Extract varying bit ranges */
        unsigned int width = (i % 8) + 1;
        unsigned int mask = (1U << width) - 1;
        unsigned int shift = i % 16;
        
        /* ZERO_EXTRACT from register variable */
        unsigned int extracted = (reg1 >> shift) & mask;
        
        /* Partial update of register variable */
        reg2 = (reg2 & ~mask) | (extracted & mask);
        
        /* Rotate and extract */
        reg1 = (reg1 << 1) | (reg1 >> 31);
        extracted = reg1 & 0x7FF;  /* 11 bits */
        
        result += extracted + reg2;
        
        COMPILER_BARRIER();
    }
    
    global_result += result;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    printf("Testing RTL patterns for ZERO_EXTRACT and STRICT_LOW_PART coverage\n");
    
    /* Initialize global array */
    for (int i = 0; i < 256; ++i) {
        global_array[i] = i;
    }
    
    /* Run tests based on command line or all by default */
    int run_all = (argc == 1);
    
    if (run_all || strstr(argv[0], "test1") || (argc > 1 && atoi(argv[1]) == 1)) {
        test_bitfield_extract_volatile();
        printf("Test 1 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test2") || (argc > 1 && atoi(argv[1]) == 2)) {
        test_packed_struct_bitfields();
        printf("Test 2 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test3") || (argc > 1 && atoi(argv[1]) == 3)) {
        test_strict_low_part_conditional();
        printf("Test 3 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test4") || (argc > 1 && atoi(argv[1]) == 4)) {
        test_inline_asm_partial_store();
        printf("Test 4 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test5") || (argc > 1 && atoi(argv[1]) == 5)) {
        test_mixed_operations_memory();
        printf("Test 5 completed\n");
    }
    
    if (run_all || strstr(argv[0], "test6") || (argc > 1 && atoi(argv[1]) == 6)) {
        test_register_variables();
        printf("Test 6 completed\n");
    }
    
    /* Print final result to prevent dead code elimination */
    printf("Final result: %u\n", global_result);
    
    /* Also use the global array to prevent optimization */
    unsigned int checksum = 0;
    for (int i = 0; i < 256; ++i) {
        checksum ^= global_array[i];
    }
    printf("Array checksum: %08x\n", checksum);
    
    return 0;
}
