/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_char(char);
extern void use_ptr(void*);

/* Volatile seed to prevent constant propagation */
static volatile int seed = 0x12345678;

/* Pattern 1: Generate ZERO_EXTRACT RTL */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    /* Union with bitfield for ZERO_EXTRACT */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 8;
            uint32_t high: 16;
        } bits;
    } u;
    
    /* Volatile input prevents constant folding */
    volatile uint32_t input = seed + 1;
    u.full = input;
    
    /* Multiple extraction patterns */
    uint32_t result = 0;
    result |= u.bits.high;          /* Should generate ZERO_EXTRACT */
    result |= (u.bits.mid << 8);    /* Another extraction */
    
    /* Complex extraction with masking */
    uint32_t temp = u.full;
    result |= ((temp >> 4) & 0xF);  /* Manual extract that might become ZERO_EXTRACT */
    
    use_int(result);
    return result;
}

/* Pattern 2: Generate STRICT_LOW_PART RTL */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    struct {
        char low_byte;
        char padding[3];
        int full_word;
    } s;
    
    volatile int input = seed + 2;
    
    /* Direct low-part assignment */
    s.low_byte = input & 0xFF;      /* Should generate STRICT_LOW_PART */
    
    /* Through pointer with type punning */
    char *ptr = (char*)&s.full_word;
    *ptr = (input >> 8) & 0xFF;     /* Another low-part store */
    
    /* Union-based low-part modification */
    union {
        int32_t full;
        struct {
            int8_t b0;
            int8_t b1;
            int8_t b2;
            int8_t b3;
        } bytes;
    } u;
    
    u.full = input;
    u.bytes.b2 = (input >> 16) & 0xFF;  /* Modify specific byte */
    
    use_char(s.low_byte);
    use_int(u.full);
    
    return s.low_byte + u.bytes.b2;
}

/* Pattern 3: Generate SUBREG and MEM_P RTL */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    int array[16];
    volatile int index = seed % 8;
    volatile int value = seed + 3;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        array[i] = i * 100;
    }
    
    /* Complex memory access with pointer arithmetic */
    short *short_ptr = (short*)((char*)array + index * sizeof(int));
    *short_ptr = value & 0xFFFF;    /* Should generate SUBREG + MEM */
    
    /* Different type access to same memory */
    char *char_ptr = (char*)&array[4];
    char_ptr[index] = value & 0xFF; /* Another SUBREG pattern */
    
    /* Nested SUBREG through union */
    union {
        int64_t dword;
        int32_t words[2];
    } u;
    
    u.dword = (int64_t)value * 1000;
    array[0] = u.words[0];          /* Access half of larger register */
    array[1] = u.words[1];
    
    use_short(*short_ptr);
    use_ptr(array);
    
    return array[0] + array[1] + *short_ptr;
}

/* Pattern 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int selector = seed % 4;
    int result = 0;
    
    /* Switch with different pattern combinations */
    switch (selector) {
        case 0: {
            /* ZERO_EXTRACT + STRICT_LOW_PART */
            union {
                uint32_t val;
                struct {
                    uint16_t low;
                    uint16_t high;
                } parts;
            } u;
            
            u.val = seed + 4;
            uint16_t extracted = u.parts.high;  /* ZERO_EXTRACT */
            
            struct {
                uint16_t low;
                uint16_t high;
            } s;
            s.low = extracted;                   /* STRICT_LOW_PART */
            
            result = s.low + s.high;
            break;
        }
        
        case 1: {
            /* SUBREG + MEM in loop */
            int buffer[8];
            volatile int idx = seed % 4;
            
            for (int i = 0; i < 8; i++) {
                buffer[i] = i * 50;
            }
            
            /* Access through different-sized types */
            int32_t *int_ptr = &buffer[idx];
            int16_t *short_ptr = (int16_t*)int_ptr;
            
            *short_ptr = (seed >> 8) & 0xFFFF;  /* SUBREG store */
            result = *int_ptr;                   /* MEM load */
            break;
        }
        
        case 2: {
            /* Nested extractions */
            uint32_t val = seed + 5;
            
            /* Multiple ZERO_EXTRACT operations */
            uint8_t byte1 = (val >> 0) & 0xFF;
            uint8_t byte2 = (val >> 8) & 0xFF;
            uint8_t byte3 = (val >> 16) & 0xFF;
            uint8_t byte4 = (val >> 24) & 0xFF;
            
            /* Recombine with STRICT_LOW_PART-like operations */
            struct {
                uint8_t b[4];
            } packed;
            
            packed.b[0] = byte1;
            packed.b[1] = byte2;
            packed.b[2] = byte3;
            packed.b[3] = byte4;
            
            result = packed.b[0] + packed.b[1] + packed.b[2] + packed.b[3];
            break;
        }
        
        default: {
            /* Complex addressing mode */
            int matrix[4][4];
            volatile int row = seed % 4;
            volatile int col = seed % 4;
            
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    matrix[i][j] = i * 10 + j;
                }
            }
            
            /* Multi-dimensional array access with byte offset */
            char *byte_ptr = (char*)matrix;
            int offset = (row * 4 + col) * sizeof(int);
            int *elem_ptr = (int*)(byte_ptr + offset);
            
            *elem_ptr = seed + 6;  /* Complex MEM address */
            result = *elem_ptr;
            break;
        }
    }
    
    use_int(result);
    return result;
}

/* Pattern 5: Recursive pattern generation */
__attribute__((noinline))
static int pattern_recursive(void) {
    static volatile int counter = 0;
    counter++;
    
    /* Conditional pattern generation */
    int result = 0;
    
    if (counter & 1) {
        /* Bitfield operations */
        struct {
            unsigned int a: 3;
            unsigned int b: 5;
            unsigned int c: 8;
            unsigned int d: 16;
        } bitfield;
        
        bitfield.a = (seed >> 0) & 0x7;
        bitfield.b = (seed >> 3) & 0x1F;
        bitfield.c = (seed >> 8) & 0xFF;
        bitfield.d = (seed >> 16) & 0xFFFF;
        
        result = bitfield.a + bitfield.b + bitfield.c + bitfield.d;
    } else {
        /* Pointer chain with SUBREGs */
        int data[4] = {1, 2, 3, 4};
        volatile int idx = seed % 3;
        
        /* Chain of pointer casts */
        void *ptr1 = data;
        char *ptr2 = (char*)ptr1 + idx * sizeof(int);
        short *ptr3 = (short*)ptr2;
        int *ptr4 = (int*)ptr3;
        
        *ptr3 = seed & 0xFFFF;  /* SUBREG store through chain */
        result = *ptr4;         /* MEM load through chain */
    }
    
    use_int(result);
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Execute all patterns */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum ^= pattern_combined();
    checksum ^= pattern_recursive();
    
    /* Multiple iterations with different seeds */
    for (int i = 0; i < 3; i++) {
        seed += 0x1000;
        checksum ^= pattern_combined();
        checksum ^= pattern_recursive();
    }
    
    printf("Final checksum: %d (0x%08x)\n", checksum, checksum);
    
    return checksum & 0xFF;
}

/* External function definitions (in separate file normally) */
#ifdef DEFINE_EXTERNAL_FUNCTIONS
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_char(char x) { (void)x; }
void use_ptr(void* x) { (void)x; }
#endif
