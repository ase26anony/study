/* Compile with: gcc -O2 -fschedule-insns -fno-strict-aliasing -fdump-rtl-reload -fdump-rtl-all -o coverage_test coverage_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 8;
} g_bitfield = {0};

/* Non-inline function to force memory addressing modes */
__attribute__((noinline))
void modify_bitfields(volatile struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = 1;      /* Likely generates ZERO_EXTRACT for 4-bit field */
    s->b = 0xFF;   /* 8-bit field assignment */
    s->c = 0x7FF;  /* 12-bit field assignment */
    s->d = 0x55;   /* Another 8-bit field */
    
    /* Additional assignment to ensure multiple SET_DEST patterns */
    s->a = s->b & 0x7;  /* Complex expression with bit-field destination */
}

/* Another noinline function for mixed-width operations */
__attribute__((noinline))
void mixed_width_operations(int argc, char **argv) {
    /* Local volatile variables to prevent optimization */
    volatile short vs;
    volatile char vc;
    volatile int vi;
    
    /* Mixed-width operations to generate SUBREG patterns */
    vi = argc * 256;
    vs = vi;  /* int to short: may generate SUBREG in SET_DEST */
    
    /* Character operations with sign extension */
    vc = (char)(argc & 0xFF);
    vi = vc * 2;  /* char to int: may involve SUBREG */
    
    /* Complex expression with SUBREG */
    vs = (vi >> 8) & 0xFF;  /* Multiple operations with width changes */
}

/* Function to create complex addressing modes */
__attribute__((noinline))
int complex_addressing(int idx1, int idx2) {
    /* 2D array with non-constant indices */
    static int arr[100][100];
    
    /* Volatile indices to prevent constant propagation */
    volatile int vi = idx1;
    volatile int vj = idx2;
    
    /* Complex array access - may generate MEM with complex XEXP */
    int val = arr[vi % 100][vj % 100];
    
    /* Bitwise operation that could be represented as ZERO_EXTRACT */
    val = val & 0x00FF00FF;  /* Mask operation */
    
    return val;
}

/* Function with register pressure */
__attribute__((optimize("O0")))
void high_register_pressure() {
    /* Many local variables to force register spilling */
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    
    /* Initialize with arithmetic to prevent optimization */
    r0 = 1; r1 = r0 * 2; r2 = r1 * 3; r3 = r2 * 4; r4 = r3 * 5;
    r5 = r4 * 6; r6 = r5 * 7; r7 = r6 * 8; r8 = r7 * 9; r9 = r8 * 10;
    r10 = r9 * 11; r11 = r10 * 12; r12 = r11 * 13; r13 = r12 * 14;
    r14 = r13 * 15; r15 = r14 * 16; r16 = r15 * 17; r17 = r16 * 18;
    r18 = r17 * 19; r19 = r18 * 20;
    
    /* Inline assembly to clobber registers and force resource tracking */
    asm volatile("" 
                 : 
                 : 
                 : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                   "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                   "memory");
    
    /* Use variables to prevent dead code elimination */
    volatile int sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                      r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19;
    (void)sum;
}

int main(int argc, char **argv) {
    int checksum = 0;
    
    /* 1. Trigger bit-field operations (ZERO_EXTRACT/STRICT_LOW_PART) */
    modify_bitfields(&g_bitfield);
    checksum += g_bitfield.a + g_bitfield.b + g_bitfield.c + g_bitfield.d;
    
    /* 2. Trigger mixed-width operations (SUBREG patterns) */
    mixed_width_operations(argc, argv);
    
    /* 3. Trigger complex addressing modes */
    checksum += complex_addressing(argc, argc * 2);
    
    /* 4. Create register pressure to force reload pass */
    high_register_pressure();
    
    /* 5. Additional mixed-type array operations */
    {
        volatile short short_array[100];
        volatile char char_array[100];
        volatile int int_array[100];
        
        /* Loop with non-constant bound */
        for (int i = 0; i < (argc % 50) + 10; i++) {
            /* Mixed-width assignments */
            int_array[i] = i * 256;
            short_array[i] = int_array[i];  /* int to short: SUBREG */
            char_array[i] = short_array[i] & 0xFF;  /* short to char */
            
            /* Complex expression with memory destination */
            int_array[i] = (int_array[i] >> 8) | (char_array[i] << 24);
            
            checksum += int_array[i] + short_array[i] + char_array[i];
        }
    }
    
    /* 6. More bit-field operations with different patterns */
    {
        volatile struct {
            unsigned int x : 3;
            unsigned int y : 5;
            unsigned int z : 24;
        } local_bitfield = {0};
        
        /* Assignment to different bit-fields */
        local_bitfield.x = argc & 0x7;
        local_bitfield.y = (argc >> 3) & 0x1F;
        local_bitfield.z = argc * 0xABCD;
        
        /* Compound assignment to bit-field */
        local_bitfield.x |= 0x4;
        local_bitfield.y &= 0x0F;
        
        checksum += local_bitfield.x + local_bitfield.y + local_bitfield.z;
    }
    
    /* 7. Force memory operations with different widths */
    {
        volatile int *mem_int = (volatile int *)malloc(100 * sizeof(int));
        volatile short *mem_short = (volatile short *)mem_int;
        volatile char *mem_char = (volatile char *)mem_int;
        
        if (mem_int) {
            /* Write with different widths */
            for (int i = 0; i < 10; i++) {
                mem_int[i] = i * 0x11111111;
                mem_short[i*2] = mem_int[i] & 0xFFFF;  /* SUBREG pattern */
                mem_char[i*4] = mem_short[i*2] & 0xFF;  /* Another SUBREG */
            }
            
            /* Read back and compute checksum */
            for (int i = 0; i < 10; i++) {
                checksum += mem_int[i] + mem_short[i*2] + mem_char[i*4];
            }
            
            free((void *)mem_int);
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
