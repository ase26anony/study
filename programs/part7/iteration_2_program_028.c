/* test_resource.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>

/* External functions to prevent optimization */
extern void use_int(int);
extern void use_short(short);
extern void use_char(char);
extern void use_ptr(void *);

/* Volatile seed to prevent constant propagation */
static volatile int seed = 0x12345678;

/* Pattern 1: Generate ZERO_EXTRACT operations */
__attribute__((noinline))
static int pattern_zero_extract(void) {
    volatile int input = seed + 1;
    int result = 0;
    
    /* Method 1: Using bitfield union */
    union {
        uint32_t full;
        struct {
            uint32_t low: 8;
            uint32_t mid: 8;
            uint32_t high: 16;
        } bits;
    } u;
    
    u.full = input;
    result = u.bits.high;  /* Should generate ZERO_EXTRACT */
    
    /* Method 2: Manual masking and shifting */
    volatile int mask = 0xFF00;
    result |= (input & mask) >> 8;
    
    /* Method 3: In conditional context */
    if (input & 0x80000000) {
        result |= (input >> 24) & 0xFF;
    }
    
    use_int(result);
    return result;
}

/* Pattern 2: Generate STRICT_LOW_PART operations */
__attribute__((noinline))
static int pattern_strict_low_part(void) {
    volatile int input = seed + 2;
    int result = 0;
    
    /* Method 1: Structure with small member */
    struct {
        char low_byte;
        int rest;
    } s;
    
    s.low_byte = input & 0xFF;  /* Should generate STRICT_LOW_PART */
    result = s.low_byte;
    
    /* Method 2: Pointer to char */
    int temp = input;
    char *cptr = (char *)&temp;
    *cptr = (input >> 8) & 0xFF;
    result += *cptr;
    
    /* Method 3: In loop context */
    for (int i = 0; i < 3; i++) {
        struct {
            short low_word;
        } w;
        w.low_word = (input >> (i * 8)) & 0xFFFF;
        result += w.low_word;
    }
    
    use_int(result);
    return result;
}

/* Pattern 3: Generate SUBREG and complex MEM_P patterns */
__attribute__((noinline))
static int pattern_subreg_mem(void) {
    volatile int index = seed & 0xF;
    volatile int value = seed + 3;
    int result = 0;
    
    /* Array with type punning */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = i * 2;
    }
    
    /* SUBREG pattern: Access through different pointer types */
    short *sptr = (short *)((char *)array + index * sizeof(int));
    *sptr = value & 0xFFFF;  /* Should generate SUBREG */
    result = *sptr;
    
    /* Complex MEM_P with address computation */
    volatile int offset = index * 2;
    int *ptr = &array[offset % 16];
    
    /* Nested memory access */
    int **pptr = &ptr;
    result += **pptr;
    
    /* Memory access in switch */
    switch (index & 3) {
        case 0:
            *(char *)ptr = value;
            break;
        case 1:
            *(short *)ptr = value >> 8;
            break;
        case 2:
            *(int *)ptr = value;
            break;
        default:
            *(char *)(ptr + 1) = value >> 16;
            break;
    }
    
    use_int(result);
    return result;
}

/* Pattern 4: Combined patterns in complex control flow */
__attribute__((noinline))
static int pattern_combined(void) {
    volatile int input = seed + 4;
    volatile int selector = seed & 1;
    int result = 0;
    
    /* Mixed operations in loop */
    for (int i = 0; i < 4; i++) {
        /* ZERO_EXTRACT pattern */
        union {
            uint32_t val;
            struct {
                uint32_t a: 4;
                uint32_t b: 12;
                uint32_t c: 16;
            } fields;
        } extract;
        
        extract.val = input + i;
        int extracted = extract.fields.b;
        
        /* STRICT_LOW_PART pattern */
        struct {
            char data[4];
        } packet;
        
        packet.data[i & 3] = extracted & 0xFF;
        
        /* MEM_P with SUBREG pattern */
        int buffer[8];
        short *buf_ptr = (short *)buffer;
        buf_ptr[i] = packet.data[i & 3];
        
        result += buf_ptr[i];
        
        /* Conditional with different patterns */
        if (selector) {
            /* Another ZERO_EXTRACT */
            result |= (input >> (i * 4)) & 0xF;
        } else {
            /* Another STRICT_LOW_PART */
            int temp = input;
            ((char *)&temp)[i & 3] = result & 0xFF;
            result = temp;
        }
    }
    
    /* Nested memory access with pointer arithmetic */
    int matrix[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = i * 4 + j;
        }
    }
    
    int *row_ptr = matrix[selector];
    short *elem_ptr = (short *)row_ptr;
    result += elem_ptr[selector * 2];
    
    use_int(result);
    return result;
}

/* Pattern 5: Recursive-like patterns with function pointers */
__attribute__((noinline))
static int pattern_complex_mem(void) {
    volatile int base = seed + 5;
    int result = 0;
    
    /* Complex addressing modes */
    struct node {
        int data;
        struct node *next;
    } nodes[4];
    
    /* Initialize linked structure */
    for (int i = 0; i < 4; i++) {
        nodes[i].data = base + i;
        nodes[i].next = &nodes[(i + 1) % 4];
    }
    
    /* Traverse with different access sizes */
    struct node *current = &nodes[0];
    for (int i = 0; i < 4; i++) {
        /* Access through pointer with offset */
        char *byte_ptr = (char *)current;
        result += byte_ptr[i % sizeof(struct node)];
        
        /* SUBREG access to part of structure */
        short *half_ptr = (short *)&current->data;
        result += half_ptr[i & 1];
        
        current = current->next;
    }
    
    /* Array of pointers with type punning */
    void *ptr_array[4];
    for (int i = 0; i < 4; i++) {
        ptr_array[i] = &nodes[i];
    }
    
    /* Access through void pointer with cast */
    for (int i = 0; i < 4; i++) {
        struct node *n = (struct node *)ptr_array[i];
        result += ((char *)&n->data)[0];
    }
    
    use_int(result);
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting resource pattern generation...\n");
    
    /* Execute all patterns */
    checksum ^= pattern_zero_extract();
    checksum ^= pattern_strict_low_part();
    checksum ^= pattern_subreg_mem();
    checksum ^= pattern_combined();
    checksum ^= pattern_complex_mem();
    
    /* Use volatile to ensure execution */
    volatile int final_result = checksum;
    
    printf("Checksum: %d\n", checksum);
    printf("Pattern generation complete.\n");
    
    return checksum & 0xFF;
}

/* External function definitions to satisfy linker */
void use_int(int x) { (void)x; }
void use_short(short x) { (void)x; }
void use_char(char x) { (void)x; }
void use_ptr(void *x) { (void)x; }
