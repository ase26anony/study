/* test_resource.c - Program to trigger specific RTL patterns for GCC coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_char(char);
extern void use_ptr(void*);

/* Volatile variables to prevent constant folding */
static volatile int volatile_seed = 0x12345678;
static volatile int volatile_index = 3;
static volatile short volatile_short = 0xABCD;
static volatile char volatile_char = 0xEF;

/* Function 1: Generate ZERO_EXTRACT patterns */
__attribute__((noinline)) 
int test_zero_extract(void) {
    /* Pattern 1: Using union with bitfields */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 12;
            uint32_t high: 12;
        } bits;
    } u1;
    
    u1.full = volatile_seed;
    int result1 = u1.bits.mid;  /* Should generate ZERO_EXTRACT */
    
    /* Pattern 2: Manual bitfield extraction with volatile */
    uint32_t val = volatile_seed;
    int result2 = (val >> 8) & 0xFFF;  /* Another ZERO_EXTRACT candidate */
    
    /* Pattern 3: Nested extraction */
    struct {
        union {
            uint64_t data;
            struct {
                uint32_t part1: 16;
                uint32_t part2: 16;
                uint32_t part3: 16;
                uint32_t part4: 16;
            } segments;
        } inner;
    } nested;
    
    nested.inner.data = (uint64_t)volatile_seed * 0x10001ULL;
    int result3 = nested.inner.segments.part2;
    
    return result1 + result2 + result3;
}

/* Function 2: Generate STRICT_LOW_PART patterns */
__attribute__((noinline))
int test_strict_low_part(void) {
    int result = 0;
    
    /* Pattern 1: Structure with small members */
    struct {
        char low_byte;
        int rest;
    } s1;
    
    int temp = volatile_seed;
    s1.low_byte = temp & 0xFF;  /* Should generate STRICT_LOW_PART */
    result += s1.low_byte;
    
    /* Pattern 2: Pointer to low part */
    uint32_t data = volatile_seed;
    uint8_t *low_ptr = (uint8_t*)&data;
    *low_ptr = volatile_char;  /* Modifying only low byte */
    result += *low_ptr;
    
    /* Pattern 3: Union assignment to partial register */
    union {
        uint32_t dword;
        struct {
            uint16_t low;
            uint16_t high;
        } words;
    } u2;
    
    u2.dword = volatile_seed;
    u2.words.low = volatile_short;  /* STRICT_LOW_PART candidate */
    result += u2.words.low;
    
    return result;
}

/* Function 3: Generate SUBREG patterns */
__attribute__((noinline))
int test_subreg(void) {
    int result = 0;
    
    /* Pattern 1: Type punning with different sized accesses */
    uint32_t array[4] = {0, 0, 0, 0};
    volatile int idx = volatile_index & 3;
    
    /* Access as different types to generate SUBREG */
    uint16_t *half_ptr = (uint16_t*)&array[idx];
    half_ptr[0] = volatile_short;  /* SUBREG from 32-bit to 16-bit */
    result += half_ptr[0];
    
    /* Pattern 2: Pointer arithmetic with type conversion */
    char *byte_ptr = (char*)array;
    byte_ptr += idx * sizeof(uint32_t) + 1;
    *byte_ptr = volatile_char;  /* SUBREG access */
    result += *byte_ptr;
    
    /* Pattern 3: Nested structure with mixed types */
    struct mixed {
        uint64_t big;
        uint32_t medium;
        uint16_t small;
        uint8_t tiny;
    } m;
    
    m.big = (uint64_t)volatile_seed;
    m.medium = volatile_seed;
    m.small = volatile_short;
    m.tiny = volatile_char;
    
    /* Access through different sized pointers */
    uint32_t *med_ptr = &m.medium;
    uint16_t *small_ptr = (uint16_t*)med_ptr;  /* SUBREG conversion */
    result += *small_ptr;
    
    return result;
}

/* Function 4: Generate MEM_P with complex addressing */
__attribute__((noinline))
int test_mem_complex(void) {
    int result = 0;
    static int buffer[16];
    
    /* Initialize buffer */
    for (int i = 0; i < 16; i++) {
        buffer[i] = i * 100;
    }
    
    /* Pattern 1: Complex pointer arithmetic */
    volatile int offset = volatile_index;
    int *ptr1 = buffer + (offset * 2) % 16;
    result += *ptr1;  /* MEM with index addressing */
    
    /* Pattern 2: Nested pointer dereference */
    int **ptr2 = &ptr1;
    result += **ptr2;  /* MEM with indirect addressing */
    
    /* Pattern 3: Array with computed index in loop */
    for (volatile int i = 0; i < 4; i++) {
        int idx = (i + offset) % 16;
        result += buffer[idx];  /* MEM with loop variant addressing */
    }
    
    /* Pattern 4: Structure pointer with field access */
    struct node {
        int value;
        struct node *next;
    } nodes[4];
    
    for (int i = 0; i < 3; i++) {
        nodes[i].value = i * 200;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[3].value = 300;
    nodes[3].next = NULL;
    
    struct node *current = &nodes[0];
    while (current) {
        result += current->value;  /* MEM with structure field access */
        current = current->next;
    }
    
    return result;
}

/* Function 5: Combined patterns to trigger recursive mark_referenced_resources */
__attribute__((noinline))
int test_combined(void) {
    /* Create a complex scenario that combines multiple patterns */
    union {
        uint32_t data[4];
        struct {
            uint16_t low[8];
            uint8_t bytes[16];
        } parts;
    } storage;
    
    /* Initialize with volatile values */
    for (int i = 0; i < 4; i++) {
        storage.data[i] = volatile_seed + i * 0x1000;
    }
    
    int result = 0;
    
    /* Combination 1: ZERO_EXTRACT + MEM_P */
    uint32_t extracted = (storage.data[1] >> 8) & 0xFFFF;  /* ZERO_EXTRACT */
    storage.parts.low[2] = extracted & 0xFF;  /* STRICT_LOW_PART + MEM */
    result += storage.parts.low[2];
    
    /* Combination 2: SUBREG + complex addressing */
    volatile int idx = volatile_index & 3;
    uint8_t *byte_ptr = (uint8_t*)&storage.data[idx];
    byte_ptr += 2;
    *byte_ptr = (extracted >> 8) & 0xFF;  /* SUBREG + MEM */
    result += *byte_ptr;
    
    /* Combination 3: Nested structure with bitfield extraction */
    struct {
        union {
            uint32_t full;
            struct {
                uint16_t low: 10;
                uint16_t high: 6;
                uint16_t extra: 16;
            } bits;
        } value;
        uint8_t buffer[8];
    } complex;
    
    complex.value.full = volatile_seed;
    complex.buffer[0] = complex.value.bits.low;  /* ZERO_EXTRACT + MEM */
    result += complex.buffer[0];
    
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for GCC coverage...\n");
    
    /* Call each test function */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg();
    checksum += test_mem_complex();
    checksum += test_combined();
    
    /* Use results to prevent dead code elimination */
    use_int(checksum);
    
    printf("Checksum: %d\n", checksum);
    printf("Pattern generation complete.\n");
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy implementations of external functions */
void use_int(int x) {
    volatile static int sink;
    sink = x;
}

void use_short(short x) {
    volatile static short sink;
    sink = x;
}

void use_char(char x) {
    volatile static char sink;
    sink = x;
}

void use_ptr(void* x) {
    volatile static void* sink;
    sink = x;
}
