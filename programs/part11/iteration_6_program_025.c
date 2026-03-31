/* Target: Generate RTL patterns that exercise SET_DEST with ZERO_EXTRACT,
   STRICT_LOW_PART, SUBREG, and MEM for mark_referenced_resources coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile structure with bit-fields to force ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;
    unsigned int b : 8;
    unsigned int c : 20;
    unsigned int d : 3;
    unsigned int e : 17;
} g_bfs = {0};

/* 2D array for complex addressing modes */
static int g_arr[100][100];

/* Non-inline function to force memory addressing */
__attribute__((noinline, optimize("O0")))
void modify_bitfields(struct BitFieldStruct *s, int x, int y) {
    /* Multiple bit-field assignments - may generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = x & 0xF;
    s->b = (x >> 4) & 0xFF;
    s->c = y & 0xFFFFF;
    s->d = (x ^ y) & 0x7;
    s->e = (x + y) & 0x1FFFF;
}

/* Another noinline function for mixed-width operations */
__attribute__((noinline, optimize("O0")))
int mixed_width_ops(short *shorts, char *chars, int count) {
    int sum = 0;
    volatile int temp; /* Prevent optimizations */
    
    for (int i = 0; i < count; i++) {
        /* Operations that may generate SUBREG RTL */
        short s_val = (short)(chars[i] * 2);  /* char to short */
        int i_val = (int)s_val * 3;           /* short to int with arithmetic */
        
        /* Store with potential SUBREG */
        shorts[i] = (short)(i_val & 0xFFFF);
        
        /* Load with sign extension - may involve SUBREG */
        char loaded = chars[(i + 1) % count];
        sum += (int)loaded;  /* Sign extension */
        
        /* Complex expression mixing types */
        temp = ((int)shorts[i] << 8) | (unsigned char)chars[i];
    }
    return sum;
}

/* Function with complex array addressing */
__attribute__((noinline))
int complex_addressing(volatile int *idx1, volatile int *idx2) {
    int result = 0;
    
    /* Volatile indices prevent constant propagation */
    int i = *idx1 % 100;
    int j = *idx2 % 100;
    
    /* Complex addressing mode for MEM */
    result = g_arr[i][j];
    
    /* Bitwise operation that might be ZERO_EXTRACT */
    result &= 0xFF;  /* Could generate ZERO_EXTRACT */
    
    /* More complex addressing with pointer arithmetic */
    int *ptr = &g_arr[i][0];
    ptr += j;
    result ^= *ptr;
    
    return result;
}

/* Function to increase register pressure */
__attribute__((noinline, optimize("O0")))
void register_pressure(int iterations) {
    /* Many local variables to force spilling */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
    
    for (int i = 0; i < iterations; i++) {
        /* Mixed operations generating SUBREG */
        v1 = (v1 << 1) | (c1 & 1);
        v2 = (v2 + s1) * (c2 - 'a');
        v3 = (v3 ^ (int)s2) + (c3 * 256);
        v4 = (v4 | ((int)s3 << 8)) & 0xFFFFFF;
        v5 = (v5 + (int)c4) - (s4 / 2);
        
        /* Inline assembly to clobber registers and force reload */
        asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", "memory");
        
        /* Bit-field like operation on integers */
        unsigned int mask = 0xF0F0F0F0;
        v1 = (v1 & ~mask) | ((v2 << 4) & mask);
    }
    
    /* Prevent dead code elimination */
    g_bfs.a = v1 & 0xF;
    g_bfs.b = v2 & 0xFF;
}

int main(int argc, char **argv) {
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            g_arr[i][j] = i * 100 + j;
        }
    }
    
    /* 1. Bit-field operations on volatile struct */
    modify_bitfields((struct BitFieldStruct*)&g_bfs, argc, argc * 2);
    
    /* 2. Mixed-width operations with local arrays */
    volatile short short_arr[50];
    volatile char char_arr[50];
    
    for (int i = 0; i < 50; i++) {
        char_arr[i] = (char)(i + argc);
    }
    
    int sum1 = mixed_width_ops((short*)short_arr, (char*)char_arr, 
                               (argc % 40) + 10);
    
    /* 3. Complex addressing with volatile indices */
    volatile int idx1 = argc * 3;
    volatile int idx2 = argc * 7 + 1;
    int sum2 = complex_addressing(&idx1, &idx2);
    
    /* 4. Register pressure and spilling */
    register_pressure((argc % 20) + 5);
    
    /* 5. Additional bit-field operations in main */
    struct BitFieldStruct local_bfs;
    
    /* Take address to force memory operations */
    struct BitFieldStruct *bfs_ptr = &local_bfs;
    
    /* Multiple assignments to different bit-fields */
    bfs_ptr->a = (argc >> 0) & 0xF;
    bfs_ptr->b = (argc >> 4) & 0xFF;
    bfs_ptr->c = (argc >> 8) & 0xFFFFF;
    bfs_ptr->d = (argc >> 28) & 0x7;
    bfs_ptr->e = (argc * 3) & 0x1FFFF;
    
    /* 6. Mixed-type operations that may generate SUBREG */
    volatile int int_var = argc;
    volatile short short_var;
    volatile char char_var;
    
    /* int to short assignment - potential SUBREG */
    short_var = (short)(int_var & 0xFFFF);
    
    /* short to char with masking */
    char_var = (char)(short_var & 0xFF);
    
    /* Back to int with extension */
    int_var = (int)char_var * 2;
    
    /* 7. Complex expression combining everything */
    int final_result = 
        (g_bfs.a + g_bfs.b) * 
        (sum1 & 0xFF) + 
        (sum2 ^ (int)short_var) - 
        (int)char_var +
        bfs_ptr->c;
    
    /* Final inline assembly to ensure resource tracking */
    asm volatile("" ::: "r0", "r1", "r2", "r3", "r4", "r5", 
                 "r6", "r7", "r8", "r9", "memory");
    
    printf("Result: %d\n", final_result % 1000);
    return final_result % 256;
}
