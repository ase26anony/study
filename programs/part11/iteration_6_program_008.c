/* Target coverage for resource.cc lines 282-290 */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} g_bfs;

/* Non-inline function to force memory addressing */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = 1;      /* Likely generates ZERO_EXTRACT for 4-bit field */
    s->b = 0xFF;   /* 8-bit field assignment */
    s->c = 0x7FF;  /* 12-bit field assignment */
    s->d = 0x55;   /* Another 8-bit field */
    
    /* Additional assignment to ensure SET_DEST processing */
    s->a = s->b & 0x7;  /* Complex expression with bit-field destination */
}

/* Another noinline function for SUBREG patterns */
__attribute__((noinline, optimize("O0")))
void mixed_width_operations(volatile short *shorts, volatile char *chars, int count) {
    for (int i = 0; i < count; i++) {
        /* Generate SUBREG patterns through mixed-width operations */
        int temp = chars[i];           /* char -> int (sign/zero extension) */
        shorts[i] = temp & 0x7FFF;     /* int -> short (truncation, SUBREG) */
        
        /* More complex SUBREG patterns */
        temp = shorts[i] * 2;          /* short promoted to int */
        chars[i] = (temp >> 8) & 0xFF; /* int -> char (SUBREG) */
    }
}

/* Function with complex addressing modes */
__attribute__((noinline))
int complex_addressing(int (*arr)[100], int idx1, int idx2) {
    /* Complex memory access that may generate interesting XEXP patterns */
    return arr[idx1][idx2] & 0x3FF;  /* Could generate ZERO_EXTRACT */
}

int main(int argc, char **argv) {
    /* 1. Bit-field operations for ZERO_EXTRACT/STRICT_LOW_PART */
    modify_bitfields((struct BitFieldStruct*)&g_bfs);
    
    /* 2. Mixed-width operations for SUBREG patterns */
    volatile short short_array[100];
    volatile char char_array[100];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < 100; i++) {
        short_array[i] = i * 3;
        char_array[i] = i & 0x7F;
    }
    
    mixed_width_operations(short_array, char_array, argc > 1 ? atoi(argv[1]) % 50 + 50 : 50);
    
    /* 3. Complex addressing with 2D array */
    int arr_2d[100][100];
    volatile int idx_i = argc > 2 ? atoi(argv[2]) % 50 : 10;
    volatile int idx_j = argc > 3 ? atoi(argv[3]) % 50 : 20;
    
    /* Initialize 2D array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr_2d[i][j] = i * 100 + j;
        }
    }
    
    /* Access with volatile indices to prevent optimization */
    int val = complex_addressing(arr_2d, idx_i, idx_j);
    
    /* 4. More SUBREG patterns through direct type punning */
    volatile int int_var = 0x12345678;
    volatile short *short_ptr = (volatile short*)&int_var;
    volatile char *char_ptr = (volatile char*)&int_var;
    
    /* Generate SUBREG stores */
    *short_ptr = 0xABCD;      /* int -> short store (SUBREG) */
    char_ptr[2] = 0xEF;       /* Partial store */
    
    /* 5. Inline assembly to increase register pressure and force resource tracking */
    asm volatile (
        "/* Clobber registers to force reload */"
        :
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* 6. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    checksum += g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d;
    checksum += short_array[0] + char_array[0];
    checksum += val;
    checksum += int_var;
    
    /* Use the checksum so it can't be optimized away */
    if (checksum > 0xFFFFFF) {
        printf("Checksum: %u\n", checksum);
    } else {
        printf("Checksum: %u (small)\n", checksum);
    }
    
    /* Additional complex expression with bit-field destination */
    struct BitFieldStruct local_bfs;
    local_bfs.a = (checksum >> 4) & 0xF;      /* ZERO_EXTRACT pattern */
    local_bfs.b = (checksum >> 8) & 0xFF;     /* Another ZERO_EXTRACT */
    
    /* Mixed operation creating potential SUBREG in SET_DEST */
    volatile int temp_int = checksum;
    volatile short temp_short;
    temp_short = (temp_int >> 16) & 0xFFFF;   /* int -> short (SUBREG) */
    
    /* Final memory access with complex address */
    int *ptr = &arr_2d[idx_i][idx_j];
    *ptr = checksum & 0x3FF;  /* Could generate ZERO_EXTRACT for memory dest */
    
    return 0;
}
