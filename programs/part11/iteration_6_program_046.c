/* Target: resource.cc lines 282-290
 * Goal: Generate SET_DEST with ZERO_EXTRACT, STRICT_LOW_PART, SUBREG wrapping MEM
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations that might simplify RTL patterns */
#define NOINLINE __attribute__((noinline))
#define NOOPT __attribute__((optimize("O0")))

/* Global volatile struct with bit-fields - forces ZERO_EXTRACT/STRICT_LOW_PART */
volatile struct BitFieldStruct {
    unsigned int a : 4;    /* Will generate ZERO_EXTRACT for 4-bit field */
    unsigned int b : 8;    /* 8-bit field */
    unsigned int c : 12;   /* 12-bit field */
    unsigned int d : 8;    /* Another 8-bit field for variety */
} g_bfs = {0};

/* NOINLINE function to modify bit-fields - prevents inlining and optimization */
NOINLINE NOOPT
void modify_bitfields(struct BitFieldStruct *s) {
    /* Multiple bit-field assignments - should generate ZERO_EXTRACT/STRICT_LOW_PART */
    s->a = 1;      /* 4-bit ZERO_EXTRACT */
    s->b = 0x55;   /* 8-bit ZERO_EXTRACT */
    s->c = 0x7FF;  /* 12-bit ZERO_EXTRACT */
    s->d = s->b;   /* Copy between bit-fields */
    
    /* Compound assignment to force complex RTL */
    s->a = (s->b & 0x3) | (s->c & 0x1);
}

/* Function to create SUBREG patterns through mixed-width operations */
NOINLINE NOOPT
void mixed_width_operations(volatile short *shorts, volatile char *chars, 
                           volatile int *ints, int count) {
    for (int i = 0; i < count; i++) {
        /* Operations that generate SUBREG patterns */
        shorts[i] = ints[i] & 0xFFFF;           /* int -> short: SUBREG */
        chars[i] = (shorts[i] >> 8) & 0xFF;     /* short -> char: SUBREG */
        
        /* Sign extension operations that may use SUBREG */
        ints[i] = (int)chars[i] * 2;            /* char -> int with sign extend */
        
        /* Mixed-width arithmetic */
        shorts[i] = (short)(ints[i] + shorts[i]); /* SUBREG in SET_DEST */
    }
}

/* Function with complex addressing modes */
NOINLINE NOOPT
int complex_addressing(int (*arr)[100], volatile int *idx1, volatile int *idx2) {
    int sum = 0;
    
    /* Complex addressing with volatile indices */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Non-constant indices from volatile variables */
            int idx_i = *idx1 + i;
            int idx_j = *idx2 + j;
            
            if (idx_i < 100 && idx_j < 100) {
                /* Memory access with complex address computation */
                sum += arr[idx_i][idx_j];
                
                /* Bit-field like operation on the array element */
                arr[idx_i][idx_j] = (arr[idx_i][idx_j] & 0xFF00FF00) | 
                                   ((sum & 0xFF) << 16) | (sum & 0xFF);
            }
        }
    }
    return sum;
}

/* Function to increase register pressure */
NOINLINE NOOPT
void force_register_pressure(void) {
    /* Many local variables to force register allocation/spilling */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Use all variables in complex expressions */
    v1 = v2 + v3 - v4 * v5 / (v6 + 1);
    v7 = v8 & v9 | v10 ^ v11;
    v12 = v13 << v14 >> v15;
    v16 = v17 > v18 ? v19 : v20;
    
    /* Inline assembly to clobber registers and force reload */
    asm volatile("" 
                 : 
                 : 
                 : "r0", "r1", "r2", "r3", "r4", "r5", 
                   "r6", "r7", "r8", "r9", "r10", "memory");
    
    /* Use variables to prevent elimination */
    g_bfs.a = (v1 + v7 + v12 + v16) & 0xF;
}

int main(int argc, char **argv) {
    /* Use argc to create non-constant loop bounds */
    int loop_count = argc > 1 ? atoi(argv[1]) : 10;
    if (loop_count > 100) loop_count = 100;
    if (loop_count < 1) loop_count = 1;
    
    /* 1. Bit-field operations on global struct */
    modify_bitfields((struct BitFieldStruct *)&g_bfs);
    
    /* 2. Mixed-width operations with local volatile arrays */
    volatile short short_arr[100];
    volatile char char_arr[100];
    volatile int int_arr[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i * 3;
        short_arr[i] = i * 2;
        char_arr[i] = i;
    }
    
    mixed_width_operations(short_arr, char_arr, int_arr, loop_count);
    
    /* 3. Complex addressing with 2D array */
    int array_2d[100][100];
    volatile int idx1 = 10, idx2 = 20;
    
    /* Initialize 2D array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            array_2d[i][j] = i * 100 + j;
        }
    }
    
    int addr_sum = complex_addressing(array_2d, &idx1, &idx2);
    
    /* 4. Force register pressure and resource tracking */
    force_register_pressure();
    
    /* 5. Additional bit-field operations combined with memory accesses */
    volatile struct BitFieldStruct local_bfs = {0};
    
    /* Take address and modify - ensures MEM in SET_DEST */
    struct BitFieldStruct *bfs_ptr = (struct BitFieldStruct *)&local_bfs;
    bfs_ptr->a = g_bfs.b & 0x7;
    bfs_ptr->b = (g_bfs.c >> 4) & 0xFF;
    bfs_ptr->c = addr_sum & 0xFFF;
    
    /* Pointer casting for SUBREG patterns */
    volatile int *int_ptr = (volatile int *)&local_bfs;
    volatile short *short_ptr = (volatile short *)int_ptr;
    volatile char *char_ptr = (volatile char *)int_ptr;
    
    /* Mixed-type accesses through pointers */
    *short_ptr = (*int_ptr >> 16) & 0xFFFF;  /* SUBREG pattern */
    char_ptr[2] = (*short_ptr >> 8) & 0xFF;  /* Another SUBREG */
    
    /* 6. Compute checksum to prevent dead code elimination */
    unsigned int checksum = 0;
    
    checksum += g_bfs.a + g_bfs.b + g_bfs.c + g_bfs.d;
    checksum += short_arr[loop_count % 100];
    checksum += char_arr[loop_count % 100];
    checksum += int_arr[loop_count % 100];
    checksum += addr_sum & 0xFFFF;
    checksum += local_bfs.a + local_bfs.b + local_bfs.c;
    checksum += *int_ptr & 0xFF;
    checksum += *short_ptr;
    checksum += char_ptr[3];
    
    printf("Checksum: %u\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}
