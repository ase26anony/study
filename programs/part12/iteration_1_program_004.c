#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100

/* Function with ARM target attribute to enable hardware loop optimizations */
__attribute__((target("arch=armv8-a")))
void nested_loops_arm(int N, int arr[SIZE][SIZE]) {
    volatile int outer_limit = N;
    volatile int early_exit_trigger = N / 3;
    int sum = 0;
    
    /* First nested loop: for-for with complex control flow */
    for (int i = 0; i < outer_limit; ++i) {
        /* Loop-invariant computation */
        int scale = i * 2 + 1;
        volatile int inner_limit = i + 5;
        
        /* Multiple basic blocks created by if-else */
        if (i % 2 == 0) {
            scale *= 2;
        } else {
            scale += 3;
        }
        
        /* Inner loop with early exit */
        for (int j = 0; j < inner_limit; ++j) {
            /* Conditional break creating additional basic block */
            if (j > early_exit_trigger) {
                break;
            }
            
            /* Computation using loop-invariant value */
            arr[i][j] = (i * j + scale) % 256;
            
            /* Another conditional continue */
            if ((i + j) % 7 == 0) {
                continue;
            }
            
            sum += arr[i][j];
        }
        
        /* Switch statement to create more basic blocks */
        switch (i % 4) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
            default: sum += 4;
        }
    }
    
    printf("First loop sum: %d\n", sum);
}

/* Second function with different loop structure */
__attribute__((target("arch=armv8-a")))
void while_for_nested(int M, int arr[SIZE][SIZE]) {
    volatile int counter = M;
    int k = 0;
    int total = 0;
    
    /* while-for nesting pattern */
    while (counter > 0) {
        volatile int inner_max = (counter % 10) + 1;
        
        /* Loop-invariant calculation */
        int offset = k * 3;
        
        /* Inner for loop */
        for (int m = 0; m < inner_max; ++m) {
            /* Conditional with multiple basic blocks */
            if (m % 3 == 0) {
                arr[k][m] = (offset + m) % 128;
                total += arr[k][m];
            } else if (m % 3 == 1) {
                arr[k][m] = (offset - m + 256) % 256;
                total += arr[k][m] * 2;
            } else {
                /* Early continue */
                if (m == inner_max - 1) {
                    continue;
                }
                arr[k][m] = (offset * m) % 192;
                total += arr[k][m] / 2;
            }
            
            /* Volatile condition for break */
            volatile int break_cond = m;
            if (break_cond > 8) {
                break;
            }
        }
        
        k++;
        counter--;
        
        /* Another control flow split */
        if (k % 5 == 0) {
            total += 100;
        }
    }
    
    printf("Second loop total: %d\n", total);
}

/* Third function with triple nesting */
__attribute__((target("arch=armv8-a")))
void triple_nested_loop(int limit, int arr[SIZE][SIZE]) {
    volatile int L = limit;
    int checksum = 0;
    
    for (int x = 0; x < L; ++x) {
        int x_squared = x * x;
        
        for (int y = 0; y < x + 2; ++y) {
            volatile int z_limit = y + 3;
            
            for (int z = 0; z < z_limit; ++z) {
                /* Complex condition creating multiple blocks */
                if (x > y && y > z) {
                    arr[x][y] += x_squared - y * z;
                    checksum += arr[x][y];
                } else if (x < y || y < z) {
                    arr[x][y] -= x * z + y;
                    checksum -= arr[x][y] / 2;
                } else {
                    /* Switch inside innermost loop */
                    switch ((x + y + z) % 3) {
                        case 0: checksum += 5; break;
                        case 1: checksum += 10; break;
                        case 2: checksum += 15; break;
                    }
                }
                
                /* Volatile break condition */
                volatile int stop = z;
                if (stop > 5) {
                    break;
                }
            }
            
            /* Conditional continue in middle loop */
            if (y % 4 == 0) {
                continue;
            }
            checksum += y;
        }
    }
    
    printf("Triple nested checksum: %d\n", checksum);
}

int main(int argc, char *argv[]) {
    int arr[SIZE][SIZE];
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Use command-line argument for non-constant loop bounds */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    if (N > SIZE - 5) N = SIZE - 5;
    if (N < 10) N = 10;
    
    printf("Running with N = %d\n", N);
    
    /* Call all three functions with different loop structures */
    nested_loops_arm(N, arr);
    while_for_nested(N / 2, arr);
    triple_nested_loop(N / 3, arr);
    
    /* Final checksum to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            final_sum += arr[i][j];
        }
    }
    printf("Final array sum: %d\n", final_sum);
    
    return 0;
}
