/* Target: Generate RTL patterns for SET_DEST with ZERO_EXTRACT, STRICT_LOW_PART, SUBREG, and MEM */
/* Compile with: gcc -O3 -fschedule-insns -fno-strict-aliasing -fdump-rtl-all -c this_file.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;   /* Will generate ZERO_EXTRACT for 4-bit field */
    unsigned int b : 8;   /* 8-bit field */
    unsigned int c : 20;  /* 20-bit field */
    unsigned int d : 1;   /* Single bit field - may generate STRICT_LOW_PART */
} g_bfs = {0};

/* 2D array for complex addressing modes */
static int g_arr[100][100];

/* Non-inline function to force memory addressing */
__attribute__((noinline)) 
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments - should generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;          /* 4-bit assignment */
    s->b = (x >> 4) & 0xFF;  /* 8-bit assignment */
    s->c = y & 0xFFFFF;      /* 20-bit assignment */
    s->d = (x ^ y) & 0x1;    /* 1-bit assignment - may generate STRICT_LOW_PART */
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
}

/* Another noinline function for mixed-width operations */
__attribute__((noinline))
void mixed_width_ops(volatile short *shorts, volatile char *chars, int count) {
    for (int i = 0; i < count; i++) {
        /* Operations that may generate SUBREG RTL */
        int temp_int = i * 256;
        shorts[i] = (short)(temp_int + i);  /* int to short - SUBREG */
        
        /* char to int with sign extension - may involve SUBREG */
        chars[i] = (char)(i & 0x7F);
        int char_as_int = chars[i];  /* Load char, sign extend to int */
        
        /* Mixed-width arithmetic */
        shorts[i] = (short)(shorts[i] + char_as_int);
    }
}

/* Function with complex array addressing */
__attribute__((noinline))
int complex_addressing(volatile int *idx1, volatile int *idx2) {
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex addressing with bitwise operation */
    int val = g_arr[i][j];
    
    /* Bitwise operation that might be represented as ZERO_EXTRACT */
    return (val & 0xFF00) >> 8;  /* Extract middle byte */
}

/* Function to create register pressure */
__attribute__((noinline))
void create_register_pressure(void) {
    /* Many local variables to force register allocation/spilling */
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    short s0, s1, s2, s3, s4;
    char c0, c1, c2, c3;
    
    /* Initialize with arithmetic to prevent optimization */
    r0 = 1; r1 = r0 * 2; r2 = r1 * 3; r3 = r2 * 4; r4 = r3 * 5;
    r5 = r4 * 6; r6 = r5 * 7; r7 = r6 * 8; r8 = r7 * 9; r9 = r8 * 10;
    
    /* Mixed-width operations */
    s0 = (short)r0; s1 = (short)r1; s2 = (short)r2; 
    s3 = (short)r3; s4 = (short)r4;
    
    /* More operations to use the variables */
    c0 = (char)(r0 & 0xFF);
    c1 = (char)(r1 & 0xFF);
    c2 = (char)(r2 & 0xFF);
    c3 = (char)(r3 & 0xFF);
    
    /* Inline assembly that clobbers registers */
    asm volatile(
        "# Force register clobbering\n"
        : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3)
        : "0"(r0), "1"(r1), "2"(r2), "3"(r3)
        : /* Clobber specific registers to force reload */
    );
    
    /* Use results to prevent dead code elimination */
    g_bfs.a = (r0 ^ r1 ^ r2 ^ r3) & 0xF;
}

int main(int argc, char **argv) {
    /* Initialize 2D array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            g_arr[i][j] = i * 100 + j;
        }
    }
    
    /* 1. Process bit-fields - should generate ZERO_EXTRACT/STRICT_LOW_PART */
    modify_bitfields((struct BitFieldStruct *)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations - should generate SUBREG */
    volatile short short_array[100];
    volatile char char_array[100];
    mixed_width_ops(short_array, char_array, argc % 50 + 10);
    
    /* 3. Complex addressing with volatile indices */
    volatile int idx1 = argc;
    volatile int idx2 = argc * 3;
    int extracted = complex_addressing(&idx1, &idx2);
    
    /* 4. Create register pressure to force reload pass */
    create_register_pressure();
    
    /* 5. Additional mixed-type operations in main */
    volatile int int_var = argc;
    volatile short short_var;
    volatile char char_var;
    
    /* int to short assignment - SUBREG pattern */
    short_var = (short)int_var;
    
    /* short to char with intermediate computation */
    char_var = (char)(short_var + 1);
    
    /* Memory access with bit-field extraction */
    struct BitFieldStruct local_bfs;
    local_bfs.a = char_var & 0x0F;  /* 4-bit field */
    local_bfs.b = char_var & 0xF0;  /* 8-bit field */
    
    /* Array access with non-constant index */
    int arr_index = argc % 100;
    int val = g_arr[arr_index][arr_index];
    
    /* Bit-field extraction from memory value */
    local_bfs.c = (val >> 4) & 0xFFFFF;  /* 20-bit field */
    local_bfs.d = val & 0x1;             /* 1-bit field */
    
    /* Another inline assembly to increase register pressure */
    asm volatile(
        "# Clobber multiple registers\n"
        ::
        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
    );
    
    /* Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    checksum += g_bfs.a;
    checksum += g_bfs.b;
    checksum += g_bfs.c;
    checksum += g_bfs.d;
    checksum += short_array[0];
    checksum += char_array[0];
    checksum += extracted;
    checksum += short_var;
    checksum += char_var;
    checksum += local_bfs.a;
    checksum += local_bfs.b;
    
    printf("Checksum: %u\n", checksum);
    
    return checksum > 100 ? 0 : 1;
}
