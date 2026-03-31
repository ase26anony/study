/* test_resource_patterns.c */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and dead code elimination */
volatile int g_seed = 42;
#define GET_RAND() (g_seed = (g_seed * 1103515245 + 12345) & 0x7fffffff)

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function 1: Bit-field extraction from integer */
int test_zero_extract_int(uint32_t x) {
    int sum = 0;
    /* Multiple bit-field extractions that should generate ZERO_EXTRACT */
    sum += (x >> 3) & 0x1F;      /* Extract bits 3-7 */
    sum += (x >> 8) & 0xFF;      /* Extract bits 8-15 */
    sum += (x >> 16) & 0x7FF;    /* Extract bits 16-26 */
    sum += (x >> 0) & 0x3;       /* Extract bits 0-1 */
    
    /* Complex expression with multiple extracts */
    sum += ((x & 0xFF00) >> 8) + ((x & 0xF0) >> 4);
    
    return sum;
}

/* Function 2: Bit-field structure operations */
int test_zero_extract_struct(struct bitfield_struct *s, int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Bit-field comparisons and assignments */
        if (s->field1 == 5) {
            sum += s->field2;
        }
        
        if (s->field3 > 2) {
            sum += s->field4 & 0x1FFF;  /* Nested extract */
        }
        
        /* Bit-field assignment */
        s->field2 = (s->field1 + i) & 0x7F;
        
        /* Complex bit-field expression */
        sum += (s->field1 << 2) | (s->field3 & 1);
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */
/* Function 3: Partial register updates with char/short */
int test_strict_low_part(int base) {
    volatile short vs;  /* volatile to prevent optimization */
    char buffer[64];
    int sum = 0;
    
    /* Partial writes to char variables */
    for (int i = 0; i < 32; i++) {
        char c = (base + i) & 0xFF;
        buffer[i] = c;           /* Should generate partial write */
        sum += buffer[i];
        
        /* Cast to short with partial update */
        short s = (short)(c * 2);
        vs = s;                  /* Volatile store of partial register */
        sum += vs;
    }
    
    /* Pointer to volatile short */
    volatile short *ps = &vs;
    *ps = (base >> 8) & 0xFFFF;  /* Partial write through pointer */
    sum += *ps;
    
    return sum;
}

/* Function 4: Inline assembly for partial register access */
int test_strict_low_part_asm(int x) {
    short result;
    
    /* Inline assembly that modifies byte/word register */
    __asm__ volatile (
        "movw %1, %%ax\n\t"
        "addw $100, %%ax\n\t"
        "movw %%ax, %0"
        : "=r"(result)
        : "r"(x)
        : "%ax"
    );
    
    /* Multiple partial updates */
    char c1 = (x >> 8) & 0xFF;
    char c2 = (x >> 16) & 0xFF;
    
    /* Force partial register writes */
    volatile char *pc = (volatile char*)&result;
    pc[0] = c1;
    pc[1] = c2;
    
    return result + c1 + c2;
}

/* ========== SUBREG patterns ========== */
/* Function 5: Union for type-punning (generates SUBREG) */
int test_subreg_union(int x) {
    union {
        uint32_t i;
        uint16_t s[2];
        uint8_t c[4];
    } u;
    
    int sum = 0;
    
    u.i = x;
    
    /* Access different views of the same register */
    sum += u.s[0];      /* SUBREG for half-word access */
    sum += u.s[1];      /* Another SUBREG */
    sum += u.c[0];      /* SUBREG for byte access */
    sum += u.c[3];      /* SUBREG for byte access */
    
    /* Modify through sub-register view */
    u.s[1] = (u.s[0] + 1) & 0xFFFF;
    
    return sum + u.i;
}

/* Function 6: Casting between types of different sizes */
int test_subreg_casts(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Casts that should generate SUBREG */
        short s = (short)(arr[i] & 0xFFFF);
        char c = (char)(arr[i] & 0xFF);
        
        /* Mixed-size operations */
        sum += s + c;
        
        /* Pointer casting for sub-register access */
        int val = arr[i];
        short *sp = (short*)&val;
        sum += sp[0] + sp[1];  /* Access halves through pointers */
    }
    
    return sum;
}

/* ========== Complex memory references ========== */
/* Function 7: Memory references with addressing modes */
int test_complex_memory(struct bitfield_struct *structs, int count) {
    int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Array indexing with stride */
        struct bitfield_struct *s = &structs[i];
        
        /* Complex address calculation */
        sum += s->field1 + s->field2;
        
        /* Pointer arithmetic */
        uint32_t *ptr = (uint32_t*)s;
        sum += ptr[0] & 0x1F;  /* Bit extract from memory */
        sum += (ptr[0] >> 16) & 0x7;  /* Another extract */
    }
    
    return sum;
}

/* ========== Main test driver ========== */
int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total_sum = 0;
    
    /* Initialize test data */
    struct bitfield_struct bf_struct = {5, 32, 2, 65535};
    int *array = malloc(iterations * sizeof(int));
    struct bitfield_struct *struct_array = malloc(iterations * sizeof(struct bitfield_struct));
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < iterations; i++) {
        array[i] = GET_RAND();
        struct_array[i].field1 = GET_RAND() & 0x1F;
        struct_array[i].field2 = GET_RAND() & 0x7F;
        struct_array[i].field3 = GET_RAND() & 0x7;
        struct_array[i].field4 = GET_RAND() & 0x1FFFF;
    }
    
    /* Test ZERO_EXTRACT patterns */
    total_sum += test_zero_extract_int(GET_RAND());
    total_sum += test_zero_extract_struct(&bf_struct, iterations);
    
    /* Test STRICT_LOW_PART patterns */
    total_sum += test_strict_low_part(GET_RAND());
    total_sum += test_strict_low_part_asm(GET_RAND());
    
    /* Test SUBREG patterns */
    total_sum += test_subreg_union(GET_RAND());
    total_sum += test_subreg_casts(array, iterations);
    
    /* Test complex memory references */
    total_sum += test_complex_memory(struct_array, iterations);
    
    /* Cleanup */
    free(array);
    free(struct_array);
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    
    return total_sum & 0xFF;  /* Return non-zero result */
}
