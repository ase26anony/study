/* test_resource.c - Program to trigger specific RTL patterns in GCC's resource.cc */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_char(char);
extern void use_ptr(void*);

/* Volatile variables to prevent constant propagation */
static volatile int volatile_seed = 0x12345678;
static volatile int volatile_index = 0;
static volatile int volatile_mask = 0xFF;

/* Function 1: Generate ZERO_EXTRACT patterns */
__attribute__((noinline))
static int test_zero_extract(void) {
    /* Pattern 1: Union with bitfields */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 16;
            uint32_t high: 8;
        } bits;
    } u;
    
    u.full = volatile_seed;
    int result1 = u.bits.mid;  /* Should generate ZERO_EXTRACT for middle 16 bits */
    
    /* Pattern 2: Manual masking and shifting */
    uint32_t val = volatile_seed;
    int result2 = (val >> 8) & 0xFFFF;  /* Another ZERO_EXTRACT candidate */
    
    /* Pattern 3: Multiple extractions in control flow */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        uint32_t temp = volatile_seed + i;
        int extracted = (temp >> (i * 4)) & 0xF;
        total += extracted;
    }
    
    use_int(result1);
    use_int(result2);
    return total;
}

/* Function 2: Generate STRICT_LOW_PART patterns */
__attribute__((noinline))
static int test_strict_low_part(void) {
    /* Pattern 1: Structure with small members */
    struct {
        char low_byte;
        short low_word;
        int full;
    } s;
    
    s.full = volatile_seed;
    s.low_byte = volatile_seed & 0xFF;      /* Should generate STRICT_LOW_PART */
    s.low_word = volatile_seed & 0xFFFF;    /* Another STRICT_LOW_PART candidate */
    
    /* Pattern 2: Pointer to low part */
    int temp = volatile_seed;
    char *low_ptr = (char*)&temp;
    *low_ptr = volatile_mask & 0xFF;        /* Modify only low byte */
    
    /* Pattern 3: Union modification */
    union {
        int32_t full;
        int8_t parts[4];
    } u;
    u.full = volatile_seed;
    u.parts[0] = volatile_mask;             /* Modify lowest byte */
    
    use_char(s.low_byte);
    use_short(s.low_word);
    return temp + u.full;
}

/* Function 3: Generate SUBREG patterns */
__attribute__((noinline))
static int test_subreg(void) {
    /* Pattern 1: Type punning through pointers */
    int array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = volatile_seed + i;
    }
    
    /* Access through different type pointers */
    short *short_ptr = (short*)array;
    int result1 = short_ptr[volatile_index % 5];  /* SUBREG access to int array */
    
    /* Pattern 2: Byte access with pointer arithmetic */
    char *byte_ptr = (char*)array;
    byte_ptr += volatile_index;
    int result2 = *(int*)(byte_ptr + 2);          /* Unaligned access with SUBREG */
    
    /* Pattern 3: Nested structures with different sizes */
    struct inner {
        short a;
        short b;
    };
    
    struct outer {
        struct inner i;
        int x;
    } outer_struct;
    
    outer_struct.x = volatile_seed;
    outer_struct.i.a = volatile_seed & 0xFFFF;    /* SUBREG access to struct member */
    
    use_short(result1);
    use_int(result2);
    return outer_struct.i.a + outer_struct.x;
}

/* Function 4: Generate complex MEM_P patterns */
__attribute__((noinline))
static int test_mem_patterns(void) {
    /* Pattern 1: Complex addressing modes */
    int buffer[100];
    volatile int idx = volatile_index;
    
    /* Multiple memory accesses with different addressing */
    buffer[idx] = volatile_seed;
    buffer[idx + 10] = buffer[idx] + 5;
    buffer[idx * 2] = buffer[idx + 10] * 2;
    
    /* Pattern 2: Pointer chains */
    int *ptr1 = &buffer[idx];
    int *ptr2 = ptr1 + 5;
    int *ptr3 = &ptr2[volatile_index % 3];
    
    /* Pattern 3: Memory access in loop with index */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += buffer[idx + i];
    }
    
    /* Pattern 4: Switch with memory operations */
    switch (volatile_seed & 0x3) {
        case 0:
            buffer[0] = sum;
            break;
        case 1:
            buffer[1] = sum * 2;
            break;
        case 2:
            buffer[2] = sum / 2;
            break;
        default:
            buffer[3] = -sum;
            break;
    }
    
    use_ptr(ptr3);
    return sum + buffer[0];
}

/* Function 5: Combined patterns to trigger recursive mark_referenced_resources */
__attribute__((noinline))
static int test_combined_patterns(void) {
    /* Combined structure with bitfields and regular members */
    struct combined {
        unsigned int field1: 10;
        unsigned int field2: 6;
        unsigned int field3: 16;
        int regular;
        short low_part;
    } data;
    
    /* Initialize with volatile to prevent constant folding */
    volatile int init = volatile_seed;
    data.regular = init;
    
    /* ZERO_EXTRACT pattern */
    unsigned int temp = init;
    data.field2 = (temp >> 10) & 0x3F;      /* ZERO_EXTRACT */
    
    /* STRICT_LOW_PART pattern */
    data.low_part = init & 0xFFFF;          /* STRICT_LOW_PART */
    
    /* Memory access with complex addressing */
    struct combined *ptr = &data;
    int *int_ptr = (int*)ptr;
    int_ptr[volatile_index % 2] = init + 1;  /* MEM with SUBREG */
    
    /* Additional memory indirection */
    struct combined **pptr = &ptr;
    (*pptr)->field3 = (init >> 16) & 0xFFFF; /* Another ZERO_EXTRACT */
    
    return data.field2 + data.low_part + data.field3;
}

/* Main function that calls all test patterns */
int main(void) {
    int checksum = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage\n");
    
    /* Call each test function and accumulate results */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg();
    checksum += test_mem_patterns();
    checksum += test_combined_patterns();
    
    /* Use volatile to ensure all computations are kept */
    volatile int final_result = checksum;
    
    printf("Final checksum: %d\n", final_result);
    printf("Hex: 0x%08X\n", final_result);
    
    return final_result & 0xFF;  /* Return only low byte */
}

/* External function definitions (in separate file normally) */
#ifdef DEFINE_EXTERNAL_FUNCTIONS
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_char(char x) { (void)x; }
void use_ptr(void* x) { (void)x; }
#endif
