/* Compile with: gcc -O3 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */
/* For MIPS cross-compilation: mips-linux-gnu-gcc -O3 -mips32 -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test_mips scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force complex scheduling decisions by creating high register pressure */
__attribute__((noinline))
static int compute_path_a(int val, int *checksum) {
    /* Use many local variables to overwhelm registers */
    int v1 = val * 3;
    int v2 = val + 0x7F;
    int v3 = v1 ^ v2;
    int v4 = v3 << 3;
    int v5 = v4 - 0x1234;
    int v6 = v5 & 0xABCD;
    int v7 = v6 | 0x5555;
    int v8 = v7 * 13;
    int v9 = v8 / 7;
    int v10 = v9 + 0x3333;
    int v11 = v10 ^ v1;
    int v12 = v11 - v2;
    int v13 = v12 * 17;
    int v14 = v13 & 0xFF;
    int v15 = v14 | 0x80;
    
    /* Mix in floating point to increase scheduling complexity */
    float f1 = (float)v1 * 1.5f;
    float f2 = (float)v2 * 2.5f;
    float f3 = f1 + f2;
    float f4 = f3 * 3.14f;
    float f5 = f4 - 100.0f;
    
    /* Memory barrier to create serialization point */
    asm volatile("" ::: "memory");
    
    /* Complex dependency chain */
    v1 = (int)f5 + v15;
    v2 = v1 * v3;
    v3 = v2 ^ v4;
    v4 = v3 + v5;
    
    /* Update checksum with all variables */
    *checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                 v11 + v12 + v13 + v14 + v15 + (int)f1 + (int)f2 + 
                 (int)f3 + (int)f4 + (int)f5;
    
    return v4;
}

__attribute__((noinline))
static int compute_path_b(int val, int *checksum) {
    /* Different computation pattern to create divergent scheduling */
    int v1 = val + 0xFF;
    int v2 = val * 5;
    int v3 = v1 & v2;
    int v4 = v3 >> 2;
    int v5 = v4 + 0x5678;
    int v6 = v5 ^ 0x9ABC;
    int v7 = v6 * 11;
    int v8 = v7 - 0x2468;
    int v9 = v8 & 0x7777;
    int v10 = v9 | 0x8888;
    int v11 = v10 / 3;
    int v12 = v11 + v1;
    int v13 = v12 ^ v2;
    int v14 = v13 * 19;
    int v15 = v14 % 256;
    
    /* Different floating point pattern */
    float f1 = (float)val * 0.75f;
    float f2 = (float)v1 * 1.25f;
    float f3 = f2 - f1;
    float f4 = f3 * 2.71f;
    float f5 = f4 + 50.0f;
    
    /* Memory barrier at different position */
    asm volatile("" ::: "memory");
    
    /* Alternative dependency chain */
    v1 = (int)f5 * v15;
    v2 = v1 | v3;
    v3 = v2 - v4;
    v4 = v3 ^ v5;
    
    /* Update checksum */
    *checksum += v1 * 2 + v2 * 3 + v3 * 4 + v4 * 5 + v5 + v6 + v7 + v8 + 
                 v9 + v10 + v11 + v12 + v13 + v14 + v15 + (int)(f1 * 2) + 
                 (int)(f2 * 3) + (int)(f3 * 4) + (int)(f4 * 5) + (int)f5;
    
    return v4;
}

/* Function pointer to prevent optimization */
static int (* volatile compute_fn)(int, int*) = NULL;

int main(void) {
    const int ARRAY_SIZE = 256;
    const int THRESHOLD = 0x4000;
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    int checksum = 0;
    
    /* Initialize with pseudo-random values */
    unsigned int seed = time(NULL);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array_a[i] = seed & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        array_b[i] = seed & 0x7FFF;
    }
    
    /* Complex loop with data-dependent branching */
    for (int iter = 0; iter < 1000; iter++) {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int val = array_a[i] ^ array_b[(i * 17) % ARRAY_SIZE];
            
            /* Hard-to-predict branch using __builtin_expect with runtime condition */
            if (__builtin_expect((val & 0x3F) > (iter & 0x3F), 0)) {
                /* Path A - with switch to create control flow complexity */
                switch (val & 0x7) {
                    case 0:
                        checksum += compute_path_a(val, &checksum);
                        break;
                    case 1:
                        checksum += compute_path_a(val + 1, &checksum);
                        break;
                    case 2:
                        checksum += compute_path_a(val * 2, &checksum);
                        break;
                    default:
                        checksum += compute_path_a(val ^ 0x55, &checksum);
                        break;
                }
            } else {
                /* Path B - with goto to create complex CFG */
                if (val & 1) {
                    goto path_b1;
                } else {
                    goto path_b2;
                }
                
            path_b1:
                checksum += compute_path_b(val, &checksum);
                goto merge_point;
                
            path_b2:
                checksum += compute_path_b(val + 0x100, &checksum);
                goto merge_point;
                
            merge_point:
                /* Empty merge point - scheduler must handle CFG merge */
                ;
            }
            
            /* Occasionally use function pointer to inhibit optimizations */
            if ((val & 0xFF) == 0) {
                compute_fn = (val & 0x80) ? compute_path_a : compute_path_b;
                if (compute_fn) {
                    checksum += compute_fn(val, &checksum);
                }
            }
            
            /* Modify arrays to create loop-carried dependencies */
            array_a[i] = (array_a[i] + checksum) & 0x7FFF;
            array_b[(i + 1) % ARRAY_SIZE] = (array_b[(i + 1) % ARRAY_SIZE] + 
                                           (checksum >> 8)) & 0x7FFF;
        }
        
        /* Additional memory barrier between outer loop iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Final computation to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result ^= array_a[i];
        final_result += array_b[i];
    }
    final_result += checksum;
    
    printf("Result: %d\n", final_result);
    return 0;
}
