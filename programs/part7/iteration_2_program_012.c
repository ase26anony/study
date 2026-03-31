/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile seed to prevent constant folding */
static volatile int seed = 0x12345678;
static volatile int index_seed = 3;
static volatile int mask_seed = 0xFF00FF00;

/* Function 1: Generate ZERO_EXTRACT patterns */
__attribute__((noinline))
static int test_zero_extract(void) {
    /* Pattern 1: Union with bitfields */
    union {
        uint32_t full;
        struct {
            uint32_t low16 : 16;
            uint32_t high16 : 16;
        } bits;
    } u1;
    
    /* Pattern 2: Explicit masking and shifting */
    struct {
        uint32_t field1 : 4;
        uint32_t field2 : 8;
        uint32_t field3 : 20;
    } bitfield;
    
    volatile uint32_t input = seed;
    int result = 0;
    
    /* ZERO_EXTRACT pattern via union */
    u1.full = input;
    result += u1.bits.high16;
    
    /* ZERO_EXTRACT pattern via bitfield assignment */
    bitfield.field1 = (input >> 0) & 0xF;
    bitfield.field2 = (input >> 4) & 0xFF;
    bitfield.field3 = (input >> 12) & 0xFFFFF;
    
    result += bitfield.field2;
    
    /* Complex ZERO_EXTRACT with volatile */
    volatile uint32_t mask = mask_seed;
    uint32_t extracted = (input & (mask >> 8)) >> 4;
    result += extracted;
    
    use_int(result);
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns */
__attribute__((noinline))
static int test_strict_low_part(void) {
    volatile int input = seed;
    int result = 0;
    
    /* Pattern 1: Structure with small members */
    struct {
        unsigned char low_byte;
        unsigned char mid_byte;
        unsigned short high_word;
    } s;
    
    /* STRICT_LOW_PART: Assign only low part */
    s.low_byte = input & 0xFF;           /* Should generate STRICT_LOW_PART */
    result += s.low_byte;
    
    /* Pattern 2: Pointer to low part */
    int temp = input;
    unsigned char *low_ptr = (unsigned char*)&temp;
    *low_ptr = (input >> 8) & 0xFF;      /* Modify only first byte */
    result += *low_ptr;
    
    /* Pattern 3: Union with byte access */
    union {
        uint32_t full;
        uint8_t bytes[4];
    } u;
    u.full = input;
    u.bytes[1] = (input >> 16) & 0xFF;   /* Modify only second byte */
    result += u.bytes[1];
    
    /* Control flow variation */
    if (input & 1) {
        s.mid_byte = (input >> 8) & 0xFF;
        result += s.mid_byte;
    } else {
        s.high_word = (input >> 16) & 0xFFFF;
        result += s.high_word;
    }
    
    use_int(result);
    return result;
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
static int test_subreg_mem(void) {
    volatile int idx = index_seed;
    volatile int value = seed;
    int result = 0;
    
    /* Array for memory access patterns */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = i * 100 + seed;
    }
    
    /* SUBREG pattern 1: Type punning with different sizes */
    uint32_t data32 = value;
    uint16_t *ptr16 = (uint16_t*)&data32;
    result += ptr16[0];  /* Access low 16 bits */
    result += ptr16[1];  /* Access high 16 bits */
    
    /* SUBREG pattern 2: Array element as different type */
    char *byte_ptr = (char*)array;
    int offset = idx * sizeof(int);
    short *short_ptr = (short*)(byte_ptr + offset + 1);
    result += *short_ptr;  /* Unaligned access */
    
    /* MEM_P with complex addressing */
    int *ptr = &array[idx & 7];
    ptr += (idx >> 3) & 1;
    result += *ptr;
    
    /* MEM_P with pointer arithmetic */
    int *base = array;
    for (int i = 0; i < 4; i++) {
        result += base[idx + i];
    }
    
    /* Complex MEM_P with multiple indices */
    volatile int idx2 = idx + 2;
    int *p1 = &array[idx % 8];
    int *p2 = &array[idx2 % 8];
    result += *p1 * *p2;
    
    use_int(result);
    return result;
}

/* Function 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int test_combined_patterns(void) {
    volatile int input = seed;
    volatile int idx = index_seed;
    int result = 0;
    
    /* Mixed data structure */
    struct mixed {
        uint32_t full;
        struct {
            uint16_t low;
            uint16_t high;
        } parts;
        uint8_t bytes[4];
    } data;
    
    data.full = input;
    
    /* Loop with combined patterns */
    for (int i = 0; i < 4; i++) {
        /* ZERO_EXTRACT in loop */
        uint8_t byte = (data.full >> (i * 8)) & 0xFF;
        
        /* STRICT_LOW_PART assignment */
        data.bytes[i] = byte;
        
        /* MEM_P with array indexing */
        static int buffer[8];
        buffer[i] = byte;
        
        /* SUBREG access */
        uint16_t *half = (uint16_t*)&buffer[i];
        result += half[0];
        
        /* Control flow variation */
        switch (i % 3) {
            case 0:
                /* ZERO_EXTRACT pattern */
                result += (data.parts.low >> 4) & 0xF;
                break;
            case 1:
                /* STRICT_LOW_PART pattern */
                data.parts.low = (input >> 8) & 0xFFFF;
                result += data.parts.low;
                break;
            case 2:
                /* MEM_P pattern */
                int *ptr = &buffer[idx % 8];
                result += *ptr;
                break;
        }
    }
    
    /* Nested conditionals */
    if (input & 0x100) {
        /* Complex memory addressing */
        int temp_array[4];
        for (int i = 0; i < 4; i++) {
            temp_array[i] = data.bytes[i] * (i + 1);
        }
        
        /* Pointer chain creating SUBREG */
        char *cptr = (char*)temp_array;
        short *sptr = (short*)(cptr + 1);
        result += sptr[idx % 2];
    } else if (input & 0x200) {
        /* Bitfield operations */
        struct {
            uint32_t a : 10;
            uint32_t b : 10;
            uint32_t c : 12;
        } bf;
        
        bf.a = (input >> 0) & 0x3FF;
        bf.b = (input >> 10) & 0x3FF;
        bf.c = (input >> 20) & 0xFFF;
        
        result += bf.a + bf.b;
    }
    
    use_int(result);
    return result;
}

/* Main function to execute all tests */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Execute all pattern generators */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg_mem();
    checksum += test_combined_patterns();
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", final_result);
    printf("Test completed.\n");
    
    return final_result & 0xFF;
}

/* Dummy external function definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_ptr(void* x) { (void)x; }
void use_long(long x) { (void)x; }
