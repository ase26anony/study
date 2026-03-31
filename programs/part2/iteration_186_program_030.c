/* mcf_test.c - Test program to trigger GCC's Minimum Cost Flow pass */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
static volatile int seed = 12345;

/* Large function with high register pressure and complex control flow */
static int __attribute__((noinline)) 
complex_mcf_function(int iterations, int mode) 
{
    /* Declare many local variables to create register pressure */
    register int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    register int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    register int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    volatile int result = 0;
    
    /* Initialize variables with dependent values */
    v0 = seed + iterations;
    v1 = v0 * 3;
    v2 = v1 - v0;
    v3 = v2 / 2;
    v4 = v3 ^ v1;
    v5 = v4 | v2;
    v6 = v5 & v3;
    v7 = v6 << 2;
    v8 = v7 >> 1;
    v9 = v8 + v0;
    v10 = v9 - v1;
    v11 = v10 * v2;
    v12 = v11 / 3;
    v13 = v12 ^ v3;
    v14 = v13 | v4;
    v15 = v14 & v5;
    v16 = v15 << 1;
    v17 = v16 >> 2;
    v18 = v17 + v6;
    v19 = v18 - v7;
    v20 = v19 * v8;
    v21 = v20 / 4;
    v22 = v21 ^ v9;
    v23 = v22 | v10;
    v24 = v23 & v11;
    v25 = v24 << 3;
    v26 = v25 >> 1;
    v27 = v26 + v12;
    v28 = v27 - v13;
    v29 = v28 * v14;
    
    /* Complex control flow with many basic blocks */
    int i;
    for (i = 0; i < iterations; i++) {
        /* Multiple conditional branches creating control flow merges */
        if (i % 3 == 0) {
            v0 = v1 + v2;
            v3 = v4 - v5;
            
            /* Inline assembly with register clobbers */
            asm volatile (
                "# Force register pressure\n"
                : "=r"(v0), "=r"(v1), "=r"(v2)
                : "0"(v0), "1"(v1), "2"(v2)
                : "eax", "ebx", "ecx", "edx", "memory"
            );
            
            goto block_a;
        } else if (i % 3 == 1) {
            v6 = v7 * v8;
            v9 = v10 ^ v11;
            
            /* Different clobber set */
            asm volatile (
                "# More register pressure\n"
                : "=r"(v6), "=r"(v7), "=r"(v8)
                : "0"(v6), "1"(v7), "2"(v8)
                : "esi", "edi", "ebp", "memory"
            );
            
            goto block_b;
        } else {
            v12 = v13 | v14;
            v15 = v16 & v17;
            goto block_c;
        }
        
    block_a:
        /* Complex computation block A */
        v18 = v19 + v20;
        v21 = v22 - v23;
        v24 = v25 * v26;
        if (v18 > 1000) {
            v27 = v28 / 2;
            goto merge_point;
        } else {
            v27 = v29 * 3;
            goto merge_point;
        }
        
    block_b:
        /* Complex computation block B */
        v18 = v19 - v20;
        v21 = v22 + v23;
        v24 = v25 / v26;
        if (v18 < 500) {
            v27 = v28 | 0xFF;
            goto merge_point;
        } else {
            v27 = v29 & 0x7F;
            goto merge_point;
        }
        
    block_c:
        /* Complex computation block C */
        v18 = v19 ^ v20;
        v21 = v22 | v23;
        v24 = v25 & v26;
        if (v18 == 0) {
            v27 = v28 << 2;
            goto merge_point;
        } else {
            v27 = v29 >> 1;
            goto merge_point;
        }
        
    merge_point:
        /* Merge point with more computations */
        switch (mode) {
            case 0:
                v0 = v27 + v18;
                v1 = v21 * v24;
                break;
            case 1:
                v0 = v27 - v18;
                v1 = v21 / (v24 ? v24 : 1);
                break;
            case 2:
                v0 = v27 ^ v18;
                v1 = v21 | v24;
                break;
            case 3:
                v0 = v27 & v18;
                v1 = v21 ^ v24;
                break;
            case 4:
                v0 = v27 | v18;
                v1 = v21 & v24;
                break;
            case 5:
                v0 = v27 << 1;
                v1 = v21 >> 2;
                break;
            case 6:
                v0 = v27 >> 1;
                v1 = v21 << 2;
                break;
            case 7:
                v0 = v27 * v18;
                v1 = v21 + v24;
                break;
            case 8:
                v0 = v27 / (v18 ? v18 : 1);
                v1 = v21 - v24;
                break;
            case 9:
                v0 = ~v27;
                v1 = ~v21;
                break;
            default:
                v0 = v27 + v21;
                v1 = v18 + v24;
                break;
        }
        
        /* More arithmetic to extend live ranges */
        v2 = v0 + v1;
        v3 = v2 * v27;
        v4 = v3 - v18;
        v5 = v4 / (v21 ? v21 : 1);
        v6 = v5 ^ v24;
        v7 = v6 | v0;
        v8 = v7 & v1;
        v9 = v8 << (i % 4);
        v10 = v9 >> ((i + 1) % 4);
        
        /* Accumulate result */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Force spill/reload with volatile */
        seed = result;
    }
    
    /* Final complex computation chain */
    v11 = v0 * v1 + v2;
    v12 = v3 - v4 * v5;
    v13 = v6 | v7 & v8;
    v14 = v9 ^ v10;
    v15 = v11 << 3;
    v16 = v12 >> 2;
    v17 = v13 + v14;
    v18 = v15 - v16;
    v19 = v17 * v18;
    v20 = v19 / (v11 ? v11 : 1);
    v21 = v20 ^ v12;
    v22 = v21 | v13;
    v23 = v22 & v14;
    v24 = v23 << 1;
    v25 = v24 >> 2;
    v26 = v25 + v15;
    v27 = v26 - v16;
    v28 = v27 * v17;
    v29 = v28 / (v18 ? v18 : 1);
    
    /* Return checksum to prevent optimization */
    return result + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
}

/* Another complex function to increase overall compilation complexity */
static int __attribute__((noinline))
secondary_mcf_function(int x, int y) 
{
    volatile int a = x, b = y;
    int c, d, e, f, g, h, i, j, k, l;
    
    /* Nested loops with conditions */
    for (c = 0; c < 10; c++) {
        d = a + c;
        for (e = 0; e < 5; e++) {
            f = b - e;
            if (d > f) {
                g = d * f;
                
                /* More inline assembly */
                asm volatile (
                    "# Additional pressure\n"
                    : "=r"(g), "=r"(d), "=r"(f)
                    : "0"(g), "1"(d), "2"(f)
                    : "r8", "r9", "r10", "r11", "memory"
                );
                
                h = g >> 1;
            } else {
                g = d + f;
                h = g << 1;
            }
            
            i = h ^ d;
            j = i | f;
            k = j & g;
            l = k + h;
            
            a += l;
            b -= k;
        }
    }
    
    return a ^ b;
}

int main(void) 
{
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        total ^= complex_mcf_function(50 + (i % 10), i % 11);
        total += secondary_mcf_function(i, total);
        
        /* Mix in some pointer operations for different register classes */
        {
            volatile int arr[4] = {i, total, i * 2, total / 2};
            volatile int *ptr = arr;
            total += ptr[0] + ptr[1] - ptr[2] + ptr[3];
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
