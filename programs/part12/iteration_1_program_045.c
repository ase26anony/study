#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __GNUC__
#define TARGET_ARM __attribute__((target("arch=armv8-a")))
#else
#define TARGET_ARM
#endif

/* Function with complex nested loops to trigger hardware loop analysis */
TARGET_ARM
void nested_loops_complex(int N, int M, int arr[200][200]) {
    volatile int early_exit_trigger = 0;
    volatile int outer_limit = N;
    volatile int inner_base = M;
    
    /* First nested loop structure: for inside for */
    for (int i = 0; i < outer_limit; ++i) {
        /* Loop-invariant calculation that varies with outer loop */
        int scale = i * 2;
        int threshold = (i % 3) * 10;
        
        /* Multiple basic blocks created by if-else */
        if (i % 5 == 0) {
            scale += 5;
        } else if (i % 3 == 0) {
            scale -= 2;
        } else {
            scale *= 2;
        }
        
        /* Inner loop with dependency on outer index */
        for (int j = 0; j < inner_base + i; ++j) {
            /* Complex conditional creating multiple basic blocks */
            if (j % 7 == 0) {
                arr[i][j] += scale * j;
            } else if (j % 11 == 0) {
                arr[i][j] -= scale / (j + 1);
            } else {
                arr[i][j] = arr[i][j] * 3 + scale;
            }
            
            /* Early exit based on volatile condition */
            if (early_exit_trigger && (j > threshold)) {
                break;
            }
            
            /* Switch statement to create more basic blocks */
            switch (j % 4) {
                case 0:
                    arr[i][j] += 1;
                    break;
                case 1:
                    arr[i][j] -= 1;
                    break;
                case 2:
                    arr[i][j] *= 2;
                    break;
                default:
                    arr[i][j] /= 2;
                    break;
            }
        }
    }
    
    /* Second nested loop structure: while inside for (different pattern) */
    int k = 0;
    while (k < outer_limit) {
        volatile int inner_counter = inner_base + (k % 10);
        
        /* Loop-invariant code */
        int offset = k * 3;
        int max_iter = (k % 4) * 5 + 10;
        
        /* For loop inside while */
        for (int m = 0; m < inner_counter && m < max_iter; ++m) {
            /* Conditional continue */
            if (m % 6 == 0) {
                arr[k][m] += offset;
                continue;
            }
            
            /* Nested if creating additional basic blocks */
            if (m % 2 == 0) {
                if (arr[k][m] > 1000) {
                    arr[k][m] = 1000;
                } else {
                    arr[k][m] += offset * 2;
                }
            } else {
                arr[k][m] -= offset / (m + 1);
            }
            
            /* Another early exit possibility */
            if (early_exit_trigger && (m > inner_counter / 2)) {
                break;
            }
        }
        
        /* Complex update with conditional */
        if (k % 7 == 0) {
            k += 2;
        } else {
            k += 1;
        }
    }
}

/* Another function with different loop nesting pattern */
TARGET_ARM
void another_loop_nest(int limit, int arr[200][200]) {
    volatile int v_limit = limit;
    int counter = 0;
    
    /* do-while inside for loop */
    for (int x = 0; x < v_limit; x += 2) {
        int y = 0;
        int inner_limit = (x % 5) + 3;
        
        do {
            /* Multiple conditions creating complex CFG */
            if (y % 2 == 0 && x % 3 == 0) {
                arr[x][y] = counter++;
            } else if (y % 3 == 0 || x % 4 == 0) {
                arr[x][y] = counter--;
            } else {
                arr[x][y] = counter * 2;
            }
            
            /* Conditional break */
            if (y > 50 && (counter % 100 == 0)) {
                break;
            }
            
            y++;
        } while (y < inner_limit && y < 100);
        
        /* Loop with switch inside */
        for (int z = 0; z < 10; z++) {
            switch (z % 3) {
                case 0:
                    arr[x][z] += x * z;
                    /* Fall through */
                case 1:
                    arr[x][z] += 100;
                    break;
                default:
                    arr[x][z] -= 50;
                    break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments for non-constant loop bounds */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int M = (argc > 2) ? atoi(argv[2]) : 40;
    
    /* Initialize array with pseudo-random data */
    int arr[200][200];
    srand(time(NULL));
    
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            arr[i][j] = rand() % 1000;
        }
    }
    
    /* Call functions with complex nested loops */
    nested_loops_complex(N, M, arr);
    another_loop_nest(N / 2, arr);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < 200; i++) {
        for (int j = 0; j < 200; j++) {
            checksum += arr[i][j];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
