/* Target: resource.cc lines 282-290 */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} g_bfs = {0};

/* Force memory addressing with complex patterns */
volatile int g_mem_pressure[256];

/* NOINLINE function to prevent optimization of bit-field operations */
__attribute__((noinline))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;          /* Likely generates ZERO_EXTRACT */
    s->b = (x >> 4) & 0xFF;  /* Another bit-field operation */
    s->c = y & 0xFFF;        /* More bit-field extraction */
    s->d = (y >> 12) & 0xFF; /* Mixed-width bit-field */
}

/* Another NOINLINE function to force SUBREG patterns */
__attribute__((noinline, optimize("O0")))
void mixed_width_operations(short *shorts, int *ints, char *chars, int n) {
    for (int i = 0; i < n; i++) {
        /* Operations that generate SUBREG patterns */
        shorts[i] = ints[i] & 0xFFFF;           /* int -> short truncation */
        chars[i] = (shorts[i] >> 8) & 0xFF;     /* short -> char */
        ints[i] = chars[i] * 2;                 /* char -> int promotion */
        
        /* Complex expression with SUBREG possibilities */
        shorts[i] = (shorts[i] + (ints[i] & 0xFF)) & 0x7FFF;
    }
}

/* Function to create complex addressing modes */
__attribute__((noinline))
int complex_addressing(int (*arr)[100], volatile int *idx1, volatile int *idx2) {
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex memory access that may generate MEM with XEXP */
    int val = arr[i][j];
    
    /* Bitwise operation that could be represented as ZERO_EXTRACT */
    val = (val & 0x00FF00FF) | ((val & 0xFF00FF00) >> 8);
    
    return val;
}

int main(int argc, char *argv[]) {
    /* Ensure argc > 1 for loop bounds */
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    srand(seed);
    
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, rand(), rand());
    
    /* 2. Mixed-width operations to generate SUBREG patterns */
    volatile short short_array[100];
    volatile int int_array[100];
    volatile char char_array[100];
    
    /* Initialize with random values */
    for (int i = 0; i < 100; i++) {
        int_array[i] = rand();
        short_array[i] = 0;
        char_array[i] = 0;
    }
    
    /* Force mixed-width operations */
    mixed_width_operations((short*)short_array, (int*)int_array, 
                          (char*)char_array, argc % 50 + 10);
    
    /* 3. Complex 2D array addressing */
    int arr_2d[100][100];
    volatile int idx1 = rand() % 100;
    volatile int idx2 = rand() % 100;
    
    /* Initialize 2D array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr_2d[i][j] = i * 100 + j;
        }
    }
    
    /* Complex addressing with volatile indices */
    int complex_val = complex_addressing(arr_2d, &idx1, &idx2);
    
    /* 4. Increase register pressure with inline assembly */
    asm volatile (
        "/* Clobber registers to force register allocation */"
        :
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* 5. More operations to generate SET_DEST with MEM patterns */
    volatile struct {
        unsigned short low;
        unsigned short high;
    } __attribute__((packed)) packed_data;
    
    /* Generate SUBREG patterns through packed access */
    unsigned int combined = 0x12345678;
    packed_data.low = combined & 0xFFFF;      /* SUBREG for low part */
    packed_data.high = (combined >> 16) & 0xFFFF; /* SUBREG for high part */
    
    /* Reconstruct with potential STRICT_LOW_PART */
    unsigned int reconstructed = packed_data.low | (packed_data.high << 16);
    
    /* 6. Additional bit-field on local volatile struct */
    volatile struct {
        unsigned int field1 : 3;
        unsigned int field2 : 5;
        unsigned int field3 : 10;
        unsigned int field4 : 14;
    } local_bf;
    
    local_bf.field1 = reconstructed & 0x7;
    local_bf.field2 = (reconstructed >> 3) & 0x1F;
    local_bf.field3 = (reconstructed >> 8) & 0x3FF;
    local_bf.field4 = (reconstructed >> 18) & 0x3FFF;
    
    /* 7. Memory pressure with array of different types */
    volatile int* volatile_ptr_array[10];
    for (int i = 0; i < 10; i++) {
        volatile_ptr_array[i] = &g_mem_pressure[i * 10];
        *volatile_ptr_array[i] = i * complex_val;
    }
    
    /* 8. Final computation to prevent dead code elimination */
    unsigned long checksum = 0;
    checksum += g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d;
    checksum += short_array[0] + int_array[0] + char_array[0];
    checksum += complex_val;
    checksum += reconstructed;
    checksum += local_bf.field1 + local_bf.field2 + local_bf.field3 + local_bf.field4;
    
    for (int i = 0; i < 10; i++) {
        checksum += *volatile_ptr_array[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    /* Additional inline assembly to force resource tracking in reload pass */
    asm volatile (
        "/* Force memory barrier and register clobbering */"
        :
        :
        : "memory", "cc", "r14", "r15", "r16", "r17"
    );
    
    return (int)(checksum % 256);
}
