/* Target: resource.cc lines 282-290 */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate ZERO_EXTRACT/STRICT_LOW_PART RTL patterns */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 12;
    unsigned int d : 20;
} g_bfs;

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
void modify_bitfields(volatile struct BitFieldStruct *s) {
    /* Multiple bit-field assignments to force ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = 1;      /* Likely ZERO_EXTRACT for 4-bit field */
    s->b = 0xFF;   /* 8-bit field */
    s->c = 0x7FF;  /* 12-bit field */
    s->d = 0xFFFFF; /* 20-bit field */
    
    /* Mixed assignments to same memory location */
    s->a = s->b & 0x3;  /* Complex expression with bit-field destination */
}

/* Another noinline function to force SUBREG patterns */
__attribute__((noinline, noipa))
void mixed_width_ops(short *shorts, int *ints, char *chars, int n) {
    for (int i = 0; i < n; i++) {
        /* SUBREG patterns: mixing different width operations */
        shorts[i] = ints[i] & 0xFFFF;  /* Truncation to 16-bit */
        ints[i] = chars[i] * 2;        /* Char to int promotion */
        
        /* Complex memory addressing */
        volatile int temp = shorts[i];
        ints[i] += temp << 8;
    }
}

/* Complex addressing mode generator */
__attribute__((noinline, noipa))
int complex_addressing(int arr[100][100], volatile int *idx1, volatile int *idx2) {
    /* Force non-constant indices */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Multiple complex memory accesses */
    int val1 = arr[i][j];
    int val2 = arr[j][i];
    
    /* Bit-field like operation on memory result */
    return (val1 & 0xFF) | ((val2 & 0xFF00) >> 8);
}

/* Function with high register pressure */
__attribute__((noinline, noipa, optimize("O0")))
void high_register_pressure(void) {
    /* Many local variables to force register spilling */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile short s1, s2, s3, s4, s5;
    volatile char c1, c2, c3, c4, c5;
    
    /* Mixed-width operations causing SUBREG */
    s1 = v1;  /* int to short */
    s2 = v2;
    s3 = v3;
    s4 = v4;
    s5 = v5;
    
    /* Inline assembly to clobber registers and force reload */
    asm volatile("" ::: 
        "r0", "r1", "r2", "r3", "r4", "r5", 
        "r6", "r7", "r8", "r9", "r10", "memory");
    
    /* More mixed operations */
    c1 = s1 & 0xFF;
    c2 = s2 & 0xFF;
    v1 = c1 * c2;  /* char to int promotion */
}

int main(int argc, char *argv[]) {
    /* Initialize global bit-field struct */
    g_bfs.a = 0;
    g_bfs.b = 0;
    g_bfs.c = 0;
    g_bfs.d = 0;
    
    /* 1. Trigger ZERO_EXTRACT/STRICT_LOW_PART patterns */
    modify_bitfields(&g_bfs);
    
    /* 2. Create mixed-width arrays for SUBREG patterns */
    volatile short short_arr[100];
    volatile int int_arr[100];
    volatile char char_arr[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i * 3;
        char_arr[i] = i & 0x7F;
    }
    
    /* Process with mixed widths using argc to prevent constant propagation */
    mixed_width_ops((short *)short_arr, (int *)int_arr, (char *)char_arr, 
                   argc > 1 ? atoi(argv[1]) % 100 : 50);
    
    /* 3. Complex 2D array addressing */
    int arr_2d[100][100];
    volatile int idx1 = argc * 37;
    volatile int idx2 = argc * 73;
    
    /* Initialize 2D array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr_2d[i][j] = i * 100 + j;
        }
    }
    
    /* Complex addressing with volatile indices */
    int result = complex_addressing(arr_2d, &idx1, &idx2);
    
    /* 4. High register pressure section */
    high_register_pressure();
    
    /* 5. Additional bit-field operations on local struct */
    volatile struct {
        unsigned int x : 3;
        unsigned int y : 5;
        unsigned int z : 24;
    } local_bf;
    
    local_bf.x = result & 0x7;
    local_bf.y = (result >> 3) & 0x1F;
    local_bf.z = result >> 8;
    
    /* Force memory access pattern that might generate STRICT_LOW_PART */
    volatile int *ptr = (volatile int *)&local_bf;
    *ptr = (*ptr & ~0xFF) | (local_bf.x << 4) | local_bf.y;
    
    /* 6. More SUBREG patterns with pointer casting */
    volatile int wide = 0x12345678;
    volatile short *narrow_ptr = (volatile short *)&wide;
    volatile short low = narrow_ptr[0];  /* SUBREG load */
    volatile short high = narrow_ptr[1]; /* SUBREG load */
    
    narrow_ptr[0] = high;  /* SUBREG store */
    narrow_ptr[1] = low;   /* SUBREG store */
    
    /* Compute checksum to prevent dead code elimination */
    unsigned int checksum = g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d;
    checksum += result;
    checksum += local_bf.x + local_bf.y + local_bf.z;
    checksum += wide & 0xFFFF;
    checksum += (wide >> 16) & 0xFFFF;
    
    /* Use argc to make array accesses non-constant */
    for (int i = 0; i < (argc % 10); i++) {
        checksum += int_arr[i];
        checksum += short_arr[i];
        checksum += char_arr[i];
    }
    
    printf("Checksum: %u\n", checksum);
    
    return checksum > 1000 ? 0 : 1;
}
