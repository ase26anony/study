/* resource_patterns.c - Generate RTL patterns for resource.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile unsigned int g_volatile_counter = 0;

/* ==================== ZERO_EXTRACT Patterns ==================== */

/* Pattern 1: Bit-field extraction using shift and mask */
unsigned int extract_bits_zextract(volatile unsigned int *p) {
    /* This should generate ZERO_EXTRACT for the bit range */
    return (*p >> 8) & 0xFF;  /* Extract bits 8-15 */
}

/* Pattern 2: Bit-field struct with address taken */
struct BitFieldStruct {
    unsigned int field1 : 4;
    unsigned int field2 : 8;
    unsigned int field3 : 4;
    unsigned int field4 : 16;
};

unsigned int read_bitfield(struct BitFieldStruct *bfs) {
    /* Taking address and reading bit-field may generate ZERO_EXTRACT */
    unsigned int val = bfs->field2;  /* 8-bit field */
    return val;
}

/* ==================== STRICT_LOW_PART Patterns ==================== */

/* Pattern 1: Writing only low byte of a larger integer */
void write_low_byte_strict(volatile unsigned int *p, unsigned char v) {
    /* This pattern may generate STRICT_LOW_PART */
    *p = (*p & ~0xFF) | v;  /* Only modify low byte */
}

/* Pattern 2: Cast to smaller type assignment */
void write_low_halfword(int32_t *x) {
    /* Direct assignment to part of larger type */
    *(int16_t*)x = 0x1234;  /* Write to low 16 bits */
}

/* ==================== SUBREG Patterns ==================== */

/* Pattern 1: Union for type punning */
union MixedTypes {
    int32_t full;
    int16_t halves[2];
    int8_t bytes[4];
};

int32_t access_via_subreg(union MixedTypes *u) {
    /* Access parts through different types */
    u->halves[0] = 100;      /* Write to low half */
    u->bytes[2] = 50;        /* Write to third byte */
    return u->full;          /* Read full word */
}

/* Pattern 2: Pointer casting between different sizes */
int64_t subreg_via_cast(int64_t ll) {
    /* Access 64-bit as 32-bit */
    int32_t *p32 = (int32_t*)&ll;
    p32[0] = p32[0] + p32[1];  /* Mix high and low parts */
    return ll;
}

/* ==================== Complex MEM Patterns ==================== */

/* Pattern 1: Array with complex indexing */
struct DataBlock {
    int arr[256];
    int metadata;
    int checksum;
};

int complex_mem_access(struct DataBlock *db, int idx1, int idx2) {
    /* Complex addressing: base + (idx1 + idx2*8) * sizeof(int) */
    return db->arr[idx1 + idx2 * 8];
}

/* Pattern 2: Pointer arithmetic with multiple components */
int* pointer_arithmetic(int *base, int offset1, int offset2) {
    /* base + offset1 + offset2*16 */
    return &base[offset1 + offset2 * 16];
}

/* ==================== Combined Function ==================== */

/* Function that combines multiple patterns in control flow */
unsigned int combined_patterns(volatile int flag) {
    unsigned int result = 0;
    static unsigned int static_counter = 0;
    
    /* Local variables for patterns */
    volatile unsigned int local_var = 0xABCD1234;
    union MixedTypes u;
    struct BitFieldStruct bfs = {0};
    struct DataBlock db;
    
    /* Initialize data */
    for (int i = 0; i < 256; i++) {
        db.arr[i] = i * 3;
    }
    
    /* Loop with conditional execution */
    for (int i = 0; i < 10; i++) {
        if (flag & 0x1) {
            /* ZERO_EXTRACT pattern */
            result ^= extract_bits_zextract(&local_var);
            
            /* Bit-field struct access */
            bfs.field2 = i & 0xFF;
            result += read_bitfield(&bfs);
        }
        
        if (flag & 0x2) {
            /* STRICT_LOW_PART patterns */
            write_low_byte_strict(&local_var, i & 0xFF);
            
            int32_t x = 0xDEADBEEF;
            write_low_halfword(&x);
            result += x;
        }
        
        if (flag & 0x4) {
            /* SUBREG patterns */
            u.full = i * 1000;
            result += access_via_subreg(&u);
            
            int64_t ll = 0x123456789ABCDEF0LL;
            result += (subreg_via_cast(ll) & 0xFFFFFFFF);
        }
        
        if (flag & 0x8) {
            /* Complex MEM patterns */
            int idx = complex_mem_access(&db, i, i % 4);
            result += idx;
            
            int *ptr = pointer_arithmetic(db.arr, i, (i + 1) % 4);
            result += *ptr;
        }
        
        /* Modify flag based on result to create data dependencies */
        flag = (flag ^ result) & 0xF;
        
        /* Prevent loop unrolling */
        static_counter++;
        if (static_counter > 1000) static_counter = 0;
    }
    
    return result;
}

/* ==================== Helper Functions ==================== */

/* Function focusing on ZERO_EXTRACT */
unsigned int test_zextract(void) {
    volatile unsigned int data = 0x89ABCDEF;
    unsigned int sum = 0;
    
    for (int i = 0; i < 8; i++) {
        /* Multiple bit-field extractions */
        sum += (data >> (i * 4)) & 0xF;
    }
    
    /* Bit-field struct */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 10;
        unsigned int d : 14;
    } bf;
    
    bf.a = 5;
    bf.b = 20;
    bf.c = 500;
    bf.d = 10000;
    
    sum += bf.a + bf.b + bf.c + bf.d;
    return sum;
}

/* Function focusing on STRICT_LOW_PART */
unsigned int test_strict_low_part(void) {
    volatile unsigned int value = 0;
    unsigned char bytes[4] = {10, 20, 30, 40};
    
    for (int i = 0; i < 4; i++) {
        /* Write individual bytes */
        value = (value & ~0xFF) | bytes[i];
        
        /* Also try 16-bit writes */
        if (i % 2 == 0) {
            *(uint16_t*)&value = (uint16_t)(bytes[i] * 256 + bytes[(i+1)%4]);
        }
    }
    
    return value;
}

/* Function focusing on SUBREG */
unsigned int test_subreg(void) {
    union {
        uint64_t qword;
        uint32_t dwords[2];
        uint16_t words[4];
        uint8_t bytes[8];
    } data;
    
    data.qword = 0;
    
    /* Mix accesses of different sizes */
    for (int i = 0; i < 8; i++) {
        data.bytes[i] = i * 10;
    }
    
    /* Perform operations mixing sizes */
    data.dwords[0] = data.words[0] * data.words[1];
    data.dwords[1] = data.bytes[4] + data.bytes[5] * 256;
    
    return (unsigned int)(data.qword & 0xFFFFFFFF);
}

/* Function focusing on complex MEM */
unsigned int test_complex_mem(void) {
    #define ARRAY_SIZE 128
    int array[ARRAY_SIZE];
    int *pointers[ARRAY_SIZE/4];
    
    /* Initialize */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i * i;
    }
    
    /* Create complex addressing patterns */
    unsigned int sum = 0;
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        pointers[i] = &array[(i * 7) % ARRAY_SIZE];
    }
    
    /* Access through pointer array with stride */
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        int idx = (i * 3) % (ARRAY_SIZE/4);
        sum += pointers[idx][i % 4];  /* Complex addressing */
    }
    
    /* Additional complex pattern: array of structs */
    struct {
        int x;
        int y;
        int z;
    } points[32];
    
    for (int i = 0; i < 32; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].z = i * 3;
        sum += points[i].x + points[(i + 5) % 32].y;  /* Non-sequential access */
    }
    
    return sum;
}

/* ==================== Main Function ==================== */

int main(void) {
    unsigned int final_result = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Use volatile to prevent dead code elimination */
    volatile int run_all = g_volatile_flag;
    
    if (run_all) {
        /* Combined test with all patterns */
        final_result = combined_patterns(g_volatile_counter);
        printf("Combined result: %u\n", final_result);
        
        /* Individual pattern tests */
        final_result ^= test_zextract();
        printf("After ZERO_EXTRACT test: %u\n", final_result);
        
        final_result ^= test_strict_low_part();
        printf("After STRICT_LOW_PART test: %u\n", final_result);
        
        final_result ^= test_subreg();
        printf("After SUBREG test: %u\n", final_result);
        
        final_result ^= test_complex_mem();
        printf("After complex MEM test: %u\n", final_result);
        
        /* Additional mixed pattern loop */
        for (int i = 0; i < 5; i++) {
            g_volatile_counter++;
            final_result += combined_patterns(g_volatile_counter);
        }
    }
    
    printf("Final checksum: %u\n", final_result);
    
    /* Return result to prevent optimization */
    return (int)(final_result & 0x7FFFFFFF);
}
