/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_ptr(void*);
extern void use_long(long);

/* Volatile variables to prevent constant folding */
static volatile int volatile_seed = 0x12345678;
static volatile int volatile_index = 3;
static volatile short volatile_short = 0xABCD;
static volatile char volatile_char = 0x42;

/* Function 1: Generate ZERO_EXTRACT patterns */
__attribute__((noinline))
int generate_zero_extract(void) {
    /* Pattern 1: Union with bitfields */
    union {
        unsigned int full;
        struct {
            unsigned int low:8;
            unsigned int middle:16;
            unsigned int high:8;
        } bits;
    } u;
    
    u.full = volatile_seed;
    int result1 = u.bits.middle;  /* Should generate ZERO_EXTRACT */
    
    /* Pattern 2: Manual bitfield extraction with volatile */
    unsigned int val = volatile_seed;
    int result2 = (val >> 8) & 0xFFFF;  /* Another ZERO_EXTRACT candidate */
    
    /* Pattern 3: Nested extraction */
    struct {
        union {
            unsigned int data;
            struct {
                unsigned int a:4;
                unsigned int b:12;
                unsigned int c:16;
            } fields;
        } inner;
    } s;
    
    s.inner.data = volatile_seed;
    int result3 = s.inner.fields.b;
    
    /* Use results to keep them alive */
    use_int(result1);
    use_int(result2);
    use_int(result3);
    
    return result1 + result2 + result3;
}

/* Function 2: Generate STRICT_LOW_PART patterns */
__attribute__((noinline))
int generate_strict_low_part(void) {
    /* Pattern 1: Structure with small member */
    struct {
        char low_byte;
        int rest;
    } data1;
    
    int temp = volatile_seed;
    data1.low_byte = temp & 0xFF;  /* Should generate STRICT_LOW_PART */
    
    /* Pattern 2: Pointer to low part */
    int full_value = volatile_seed;
    char *low_ptr = (char*)&full_value;
    *low_ptr = volatile_char;  /* Modifies only low byte */
    
    /* Pattern 3: Union assignment to partial register */
    union {
        int full;
        struct {
            short low;
            short high;
        } parts;
    } u;
    
    u.full = volatile_seed;
    u.parts.low = volatile_short;  /* STRICT_LOW_PART candidate */
    
    /* Pattern 4: Array with byte access */
    int array[4] = {0};
    int idx = volatile_index & 3;
    ((char*)array)[idx] = volatile_char;  /* Partial store */
    
    use_int(data1.low_byte);
    use_int(full_value);
    use_short(u.parts.low);
    use_int(array[idx]);
    
    return data1.low_byte + full_value + u.parts.low + array[idx];
}

/* Function 3: Generate SUBREG and MEM_P patterns */
__attribute__((noinline))
int generate_subreg_mem(void) {
    /* Pattern 1: Type punning with arrays */
    int main_array[16];
    volatile int init_val = volatile_seed;
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        main_array[i] = init_val + i;
    }
    
    /* Complex memory access with pointer arithmetic */
    int idx = volatile_index;
    short *short_ptr = (short*)((char*)main_array + idx * sizeof(int) + 1);
    *short_ptr = volatile_short;  /* MEM with SUBREG addressing */
    
    /* Pattern 2: Nested SUBREG through unions */
    union {
        long long doubleword;
        struct {
            int first;
            int second;
        } words;
    } u;
    
    u.doubleword = (long long)volatile_seed * 2;
    int *word_ptr = &u.words.second;
    *word_ptr = volatile_seed;  /* MEM access to union member */
    
    /* Pattern 3: Multi-level pointer indirection */
    int **ptr_ptr = (int**)main_array;
    int *deref_ptr = ptr_ptr[idx & 3];
    if (deref_ptr) {
        deref_ptr = (int*)((char*)deref_ptr + 2);
        *((short*)deref_ptr) = volatile_short;
    }
    
    /* Pattern 4: Switch with different memory access patterns */
    int result = 0;
    switch (volatile_index & 3) {
        case 0:
            result = *((char*)main_array + 8);
            break;
        case 1:
            result = *((short*)main_array + 2);
            break;
        case 2:
            result = main_array[4];
            break;
        case 3:
            result = *((int*)((char*)main_array + 12));
            break;
    }
    
    use_short(*short_ptr);
    use_int(*word_ptr);
    use_int(result);
    use_ptr(ptr_ptr);
    
    return *short_ptr + *word_ptr + result;
}

/* Function 4: Combined patterns in complex control flow */
__attribute__((noinline))
int generate_combined_patterns(void) {
    volatile int counter = volatile_seed & 0xF;
    int accumulator = 0;
    
    /* Loop with combined operations */
    for (int i = 0; i < counter; i++) {
        /* ZERO_EXTRACT in loop */
        union {
            unsigned int val;
            struct {
                unsigned int a:10;
                unsigned int b:10;
                unsigned int c:12;
            } parts;
        } u;
        
        u.val = volatile_seed + i;
        int extracted = u.parts.b;  /* ZERO_EXTRACT */
        
        /* STRICT_LOW_PART in loop */
        struct {
            char low;
            char high[3];
        } s;
        
        s.low = extracted & 0xFF;  /* STRICT_LOW_PART */
        
        /* MEM with SUBREG in loop */
        int buffer[8];
        int *ptr = (int*)((char*)buffer + (i & 3));
        *ptr = extracted;  /* Complex MEM address */
        
        /* Conditional based on volatile */
        if (volatile_char & (1 << (i & 3))) {
            short *short_ptr = (short*)ptr;
            *short_ptr = s.low;  /* Another MEM with SUBREG */
        }
        
        accumulator += extracted + s.low + *ptr;
    }
    
    /* Nested conditionals */
    if (volatile_seed & 1) {
        /* Additional ZERO_EXTRACT */
        unsigned int mask = 0x00FF00FF;
        int masked = (volatile_seed & mask) | ((volatile_seed >> 8) & mask);
        accumulator += masked;
    } else {
        /* Additional STRICT_LOW_PART */
        long long big_val = (long long)volatile_seed << 32;
        int *low_part = (int*)&big_val;
        *low_part = accumulator;  /* Store to low part of long long */
        accumulator = *low_part;
    }
    
    use_int(accumulator);
    return accumulator;
}

/* Main function that drives all patterns */
int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Generate each pattern type */
    checksum += generate_zero_extract();
    printf("Zero extract patterns generated\n");
    
    checksum += generate_strict_low_part();
    printf("Strict low part patterns generated\n");
    
    checksum += generate_subreg_mem();
    printf("Subreg and MEM patterns generated\n");
    
    checksum += generate_combined_patterns();
    printf("Combined patterns generated\n");
    
    /* Final checksum */
    printf("Final checksum: %d\n", checksum);
    
    /* Use checksum to prevent dead code elimination */
    use_int(checksum);
    
    return checksum & 0xFF;
}

/* Dummy external function definitions to satisfy linker */
/* These would normally be in a separate file */
void __attribute__((weak)) use_int(int x) { (void)x; }
void __attribute__((weak)) use_short(short x) { (void)x; }
void __attribute__((weak)) use_ptr(void* x) { (void)x; }
void __attribute__((weak)) use_long(long x) { (void)x; }
