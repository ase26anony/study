/* test_resource.c - Generate ZERO_EXTRACT and STRICT_LOW_PART RTL patterns */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compiler barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Test 1: Bitfield extraction from volatile integer */
void test_zero_extract_volatile(void) {
    volatile unsigned int source = 0xDEADBEEF;
    volatile unsigned int result = 0;
    
    /* This should generate ZERO_EXTRACT RTL */
    for (int i = 0; i < 100; ++i) {
        /* Extract bits 8-15 (width=8, starting at bit 8) */
        unsigned int extracted = (source >> 8) & 0xFF;
        result += extracted;
        COMPILER_BARRIER();
        source = (source << 1) | (source >> 31); /* Rotate to vary bits */
    }
    
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 2: Packed struct with bitfields */
void test_zero_extract_struct(void) {
    struct __attribute__((packed)) {
        unsigned int header : 4;
        unsigned int data   : 20;
        unsigned int footer : 8;
    } s;
    
    volatile unsigned int *ptr = (volatile unsigned int*)&s;
    *ptr = 0x12345678;
    
    volatile int sum = 0;
    
    /* Multiple bitfield accesses that should generate ZERO_EXTRACT */
    for (int i = 0; i < 50; ++i) {
        /* Read bitfield - should generate ZERO_EXTRACT */
        unsigned int val1 = s.data;
        
        /* Write to bitfield - may generate ZERO_EXTRACT in SET_DEST */
        s.header = (val1 + i) & 0xF;
        
        /* Complex bitfield expression */
        s.footer = (s.data >> 4) & 0xFF;
        
        sum += val1 + s.header + s.footer;
        COMPILER_BARRIER();
    }
    
    volatile int sink __attribute__((unused)) = sum;
}

/* Test 3: Mixed bitfield operations with arrays */
void test_zero_extract_array(void) {
    struct __attribute__((packed)) {
        unsigned short low : 6;
        unsigned short mid : 5;
        unsigned short high : 5;
    } arr[32];
    
    volatile int total = 0;
    
    /* Initialize array */
    for (int i = 0; i < 32; ++i) {
        *(volatile unsigned short*)&arr[i] = i * 0x111;
    }
    
    /* Nested loops with bitfield accesses */
    for (int outer = 0; outer < 10; ++outer) {
        for (int i = 0; i < 32; ++i) {
            /* Multiple bitfield reads */
            int val = arr[i].mid;
            
            /* Conditional bitfield write */
            if (val > 8) {
                arr[i].low = (arr[i].high + val) & 0x3F;
            }
            
            /* Switch based on bitfield value */
            switch(arr[i].high & 0x7) {
                case 0: total += 1; break;
                case 1: total += val; break;
                case 2: total += arr[i].low; break;
                default: total -= 1; break;
            }
            COMPILER_BARRIER();
        }
    }
    
    volatile int sink __attribute__((unused)) = total;
}

/* Test 4: STRICT_LOW_PART via inline assembly */
void test_strict_low_part_asm(void) {
    volatile unsigned int reg = 0x87654321;
    volatile unsigned int result = 0;
    
    for (int i = 0; i < 100; ++i) {
        unsigned int temp;
        
        /* Inline assembly that modifies only low bits */
        asm volatile (
            "and %0, %1, %2\n\t"          /* Clear high bits */
            "orr %0, %0, %3\n\t"          /* Set some low bits */
            : "=r"(temp)
            : "r"(reg), "i"(0x0000FFFF), "i"(i & 0xFFFF)
            : /* No clobbers */
        );
        
        /* Another asm that hints at partial register update */
        asm volatile (
            "bfi %0, %1, #0, #8\n\t"      /* Bitfield insert (if supported) */
            : "+r"(temp)
            : "r"(i)
            : /* No clobbers */
        );
        
        result ^= temp;
        reg = (reg << 1) | (reg >> 31);   /* Rotate */
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = result;
}

/* Test 5: STRICT_LOW_PART via conditional merge operations */
void test_strict_low_part_merge(void) {
    volatile unsigned int data = 0x12345678;
    volatile unsigned int accumulator = 0;
    
    for (int i = 0; i < 200; ++i) {
        /* Conditional update of low byte only */
        if (i & 1) {
            /* This pattern often generates STRICT_LOW_PART */
            data = (data & ~0xFF) | ((i + data) & 0xFF);
        }
        
        /* Update low 16 bits based on condition */
        if (i % 3 == 0) {
            data = (data & ~0xFFFF) | ((data * 3) & 0xFFFF);
        }
        
        /* Partial store through char pointer */
        volatile unsigned char *byte_ptr = (volatile unsigned char*)&data;
        byte_ptr[1] = (i >> 2) & 0xFF;  /* Modify second byte only */
        
        accumulator += data;
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = accumulator;
}

/* Test 6: Combined patterns with memory operations */
void test_combined_patterns(void) {
    volatile unsigned int buffer[64];
    volatile unsigned int *ptr = buffer;
    
    /* Initialize buffer */
    for (int i = 0; i < 64; ++i) {
        buffer[i] = i * 0x01010101;
    }
    
    struct __attribute__((packed)) {
        unsigned int a : 10;
        unsigned int b : 12;
        unsigned int c : 10;
    } bitfield;
    
    *(volatile unsigned int*)&bitfield = 0;
    
    register unsigned int reg_sum asm("r12") = 0;  /* Suggest register */
    
    for (int i = 0; i < 100; ++i) {
        /* Memory load */
        unsigned int mem_val = ptr[i % 64];
        
        /* Bitfield extraction (ZERO_EXTRACT) */
        bitfield.a = (mem_val >> 2) & 0x3FF;
        bitfield.b = (mem_val >> 12) & 0xFFF;
        
        /* Partial update (potential STRICT_LOW_PART) */
        if (bitfield.b > 2048) {
            unsigned int temp = bitfield.c;
            bitfield.c = (temp & ~0x1FF) | ((temp + 1) & 0x1FF);
        }
        
        /* Complex expression mixing everything */
        reg_sum += bitfield.a + bitfield.b + bitfield.c + (mem_val & 0xFF);
        
        /* Pointer arithmetic */
        ptr = &buffer[(i + 1) % 64];
        COMPILER_BARRIER();
    }
    
    volatile unsigned int sink __attribute__((unused)) = reg_sum;
}

/* Main driver */
int main(int argc, char *argv[]) {
    volatile unsigned int total_result = 0;
    
    /* Run tests based on command line or all by default */
    int run_all = (argc <= 1);
    
    if (run_all || strstr(argv[1], "1")) {
        test_zero_extract_volatile();
        total_result += 1;
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "2"))) {
        test_zero_extract_struct();
        total_result += 2;
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "3"))) {
        test_zero_extract_array();
        total_result += 3;
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "4"))) {
        test_strict_low_part_asm();
        total_result += 4;
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "5"))) {
        test_strict_low_part_merge();
        total_result += 5;
    }
    
    if (run_all || (argc > 1 && strstr(argv[1], "6"))) {
        test_combined_patterns();
        total_result += 6;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %u\n", total_result);
    
    return 0;
}
