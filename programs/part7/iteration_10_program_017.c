/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile unsigned int global_val = 0x12345678;
volatile unsigned short global_short = 0;
volatile int global_array[256];
volatile int global_result = 0;

/* Function to ensure operations aren't optimized away */
__attribute__((noinline))
int perform_operations(int seed) {
    volatile int local_result = 0;
    
    /* 1. Generate ZERO_EXTRACT pattern */
    /* Bit-field extraction with variable width and position */
    volatile unsigned int extract_src = global_val ^ seed;
    volatile int start = (seed % 16) + 1;      /* Non-constant start position */
    volatile int width = (seed % 8) + 1;       /* Non-constant width */
    
    /* This should generate ZERO_EXTRACT RTL */
    unsigned int extracted = (extract_src >> start) & ((1U << width) - 1);
    local_result ^= extracted;
    
    /* 2. Generate STRICT_LOW_PART pattern */
    /* Assignment to smaller type - may generate STRICT_LOW_PART */
    volatile int src_int = seed * 37;
    volatile short dest_short;
    
    /* Explicit cast and assignment to volatile short */
    dest_short = (short)src_int;
    global_short = dest_short;  /* Force side effect */
    local_result += dest_short;
    
    /* 3. SUBREG is generated naturally through size changes */
    /* Multiple casts between different integer sizes */
    int32_t a = seed * 73;
    int16_t b = (int16_t)a;          /* Likely generates SUBREG */
    int8_t c = (int8_t)b;            /* Another SUBREG */
    local_result += c;
    
    /* Cast back and forth */
    uint64_t d = (uint64_t)a;
    uint32_t e = (uint32_t)d;        /* Another SUBREG */
    local_result ^= e;
    
    /* 4. Complex MEM address with register + offset */
    /* Array access with variable index computation */
    volatile int* arr = (volatile int*)global_array;
    
    /* Initialize array with some values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * seed;
    }
    
    /* Complex addressing: base + (index * scale) + offset */
    volatile int idx = (seed % 128) * 2;
    int value1 = arr[idx + 5];                /* MEM with complex address */
    int value2 = arr[(idx * 3) % 256];        /* More complex address */
    
    /* Even more complex: pointer arithmetic */
    volatile int* ptr = arr + (seed % 64) + 10;
    int value3 = *ptr;                        /* Another MEM */
    
    local_result += value1 + value2 + value3;
    
    /* 5. Additional ZERO_EXTRACT with memory source */
    /* Bit-field from memory location */
    volatile unsigned int* mem_loc = (volatile unsigned int*)&global_array[32];
    unsigned int mem_val = *mem_loc;
    unsigned int mem_extract = (mem_val >> (seed % 24)) & 0xFF;
    local_result += mem_extract;
    
    /* 6. Mixed operations to ensure all patterns are used together */
    /* This creates dependencies between different RTL patterns */
    volatile int temp = local_result;
    
    /* Another STRICT_LOW_PART-like operation */
    volatile char small = (char)(temp & 0xFF);
    local_result = small * 2;
    
    /* Memory store with complex address */
    volatile int store_idx = (seed * 7) % 256;
    global_array[store_idx] = local_result;
    
    /* Final memory load with even more complex address */
    int final_load = global_array[(store_idx + 37) % 256];
    
    return final_load ^ local_result;
}

/* Second function to create more opportunities for scheduling */
__attribute__((noinline))
int secondary_operations(int base) {
    volatile int acc = base;
    
    /* More bit-field operations */
    for (int i = 0; i < 4; i++) {
        volatile unsigned int x = acc ^ (i * 0x11111111);
        int shift = (i * 3) % 28;
        int bits = 4 + (i % 4);
        acc += (x >> shift) & ((1 << bits) - 1);
    }
    
    /* Memory operations with different addressing modes */
    volatile int local_arr[64];
    for (int i = 0; i < 64; i++) {
        local_arr[i] = i * base;
    }
    
    /* Multiple complex memory accesses */
    acc += local_arr[(base + 10) % 64];
    acc += local_arr[(base * 2) % 64];
    acc += local_arr[(base / 3) % 64];
    
    return acc;
}

int main(int argc, char *argv[]) {
    int seed = 42;  /* Default seed */
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize global array with pattern */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * seed;
    }
    
    /* Perform operations that should generate target RTL patterns */
    int result1 = perform_operations(seed);
    int result2 = secondary_operations(seed);
    
    /* Combine results to ensure all computations are used */
    global_result = result1 ^ result2;
    
    /* Additional loop to create scheduling opportunities */
    for (int i = 0; i < 100; i++) {
        /* Mix in more operations of each type */
        volatile int temp = global_result + i;
        
        /* More bit-field extracts */
        unsigned int bf = (temp >> (i % 16)) & 0xF;
        global_result ^= bf;
        
        /* More strict low part assignments */
        volatile short s = (short)temp;
        global_result += s;
        
        /* More memory accesses */
        global_result += global_array[(i + seed) % 256];
    }
    
    printf("Result: %d\n", global_result);
    
    return global_result != 0 ? 0 : 1;
}
