/* Target: resource.cc lines 282-290 */
/* Compile with: gcc -O2 -fdump-rtl-reload -fno-strict-aliasing -o coverage_test coverage_test.c */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to generate ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;    /* Will generate ZERO_EXTRACT for 4-bit field */
    unsigned int b : 8;    /* 8-bit field */
    unsigned int c : 12;   /* 12-bit field */
    unsigned int d : 8;    /* Another 8-bit field */
} g_bitfield = {0};

/* Non-inline function to force memory addressing modes */
__attribute__((noinline)) 
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments - should generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;        /* 4-bit assignment */
    s->b = (x >> 4) & 0xFF; /* 8-bit assignment */
    s->c = y & 0xFFF;      /* 12-bit assignment */
    s->d = (y >> 12) & 0xFF; /* 8-bit assignment */
    
    /* Additional complex pattern: nested bit-field operations */
    s->a = (s->b & 0x3) | ((s->c & 0x1) << 2);
}

/* Another noinline function for SUBREG patterns */
__attribute__((noinline, optimize("O0")))
void mixed_width_operations(short *shorts, int *ints, char *chars, int n) {
    volatile int i;
    for (i = 0; i < n; i++) {
        /* Generate SUBREG patterns by mixing widths */
        shorts[i] = ints[i] & 0xFFFF;           /* int -> short truncation */
        chars[i] = (shorts[i] >> 8) & 0xFF;     /* short -> char truncation */
        
        /* More complex SUBREG patterns with arithmetic */
        ints[i] = (ints[i] & 0xFF00) | (chars[i] << 8);
        
        /* Pointer casting for SUBREG generation */
        *((volatile short*)((char*)ints + i*2)) = 
            *((volatile char*)((char*)shorts + i));
    }
}

/* Function to create complex addressing modes */
__attribute__((noinline))
int complex_addressing(int arr[100][100], volatile int *idx1, volatile int *idx2) {
    /* Complex memory addressing that will generate XEXP patterns */
    int sum = 0;
    volatile int i, j;
    
    /* Non-constant indices force complex address calculation */
    i = *idx1 % 100;
    j = *idx2 % 100;
    
    /* Multiple array accesses with complex indices */
    sum += arr[i][j];
    sum += arr[j][i];
    sum += arr[(i + j) % 100][(i - j + 100) % 100];
    
    /* Combine with bit-field like operation */
    sum = (sum & 0xFF) | ((sum & 0xFF00) >> 8);
    
    return sum;
}

/* Function to increase register pressure */
__attribute__((optimize("O0")))
void force_register_pressure(void) {
    /* Many local variables to force register allocation */
    volatile int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    volatile short s0, s1, s2, s3, s4;
    volatile char c0, c1, c2, c3;
    
    /* Initialize with arithmetic to prevent optimization */
    r0 = 1; r1 = 2; r2 = 3; r3 = 4; r4 = 5;
    r5 = 6; r6 = 7; r7 = 8; r8 = 9; r9 = 10;
    
    /* Mixed-width operations generating SUBREG */
    s0 = r0; s1 = r1; s2 = r2; s3 = r3; s4 = r4;
    c0 = s0; c1 = s1; c2 = s2; c3 = s3;
    
    /* Inline assembly to clobber registers and force reload */
    asm volatile("" 
                 : 
                 : 
                 : "r0", "r1", "r2", "r3", "r4", "r5", 
                   "r6", "r7", "r8", "r9", "r10", "r11",
                   "r12", "r13", "r14", "r15", "memory");
    
    /* Use all variables to prevent dead code elimination */
    r0 = s0 + c0; r1 = s1 + c1; r2 = s2 + c2; r3 = s3 + c3;
}

int main(int argc, char *argv[]) {
    int i, j;
    
    /* 1. Trigger bit-field patterns (ZERO_EXTRACT/STRICT_LOW_PART) */
    modify_bitfields((struct BitFieldStruct*)&g_bitfield, 
                     argc * 0x1234, 
                     argc * 0x5678);
    
    /* 2. Mixed-width operations for SUBREG generation */
    volatile short short_array[100];
    volatile int int_array[100];
    volatile char char_array[100];
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        int_array[i] = i * argc;
        short_array[i] = 0;
        char_array[i] = 0;
    }
    
    /* Call function that generates SUBREG patterns */
    mixed_width_operations((short*)short_array, 
                          (int*)int_array, 
                          (char*)char_array, 
                          argc % 50 + 10);
    
    /* 3. Complex addressing modes */
    int matrix[100][100];
    volatile int idx1 = argc * 3;
    volatile int idx2 = argc * 7;
    
    /* Initialize matrix */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    int addr_result = complex_addressing(matrix, &idx1, &idx2);
    
    /* 4. Force register pressure and resource tracking */
    force_register_pressure();
    
    /* 5. Additional volatile bit-field operations in main */
    volatile struct {
        unsigned int low : 16;
        unsigned int high : 16;
    } split_int;
    
    /* This should generate ZERO_EXTRACT/STRICT_LOW_PART in SET_DEST */
    split_int.low = addr_result & 0xFFFF;
    split_int.high = (addr_result >> 16) & 0xFFFF;
    
    /* More SUBREG patterns with pointer casting */
    volatile int *int_ptr = &int_array[0];
    volatile short *short_ptr = (volatile short*)int_ptr;
    volatile char *char_ptr = (volatile char*)int_ptr;
    
    /* Mixed operations through pointers */
    for (i = 0; i < 10; i++) {
        *short_ptr++ = *char_ptr++ + i;
        *((volatile char*)short_ptr) = *int_ptr & 0xFF;
        int_ptr++;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    checksum += g_bitfield.a + g_bitfield.b + g_bitfield.c + g_bitfield.d;
    
    for (i = 0; i < 50; i++) {
        checksum += short_array[i];
        checksum += int_array[i] & 0xFFFF;
        checksum += char_array[i];
    }
    
    checksum += addr_result;
    checksum += split_int.low + split_int.high;
    
    printf("Checksum: %u\n", checksum);
    
    return checksum > 0 ? 0 : 1;
}
