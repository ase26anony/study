#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100

/* Function with ARM target attribute to enable hardware loop optimizations */
__attribute__((target("arch=armv8-a")))
void nested_loops_arm(int N, int arr[SIZE][SIZE]) {
    volatile int early_exit_trigger = 0;
    int result = 0;
    
    /* First nested loop structure: for inside for */
    for (int i = 0; i < N; ++i) {
        /* Loop-invariant code that varies with outer loop */
        int scale = i * 2 + 1;
        volatile int inner_limit = i + 5;
        
        /* Multiple basic blocks created by if-else */
        if (i % 3 == 0) {
            scale *= 2;
        } else if (i % 3 == 1) {
            scale += 10;
        } else {
            scale -= 5;
        }
        
        /* Inner loop with complex control flow */
        for (int j = 0; j < inner_limit; ++j) {
            /* Conditional break based on volatile variable */
            if (early_exit_trigger && j > inner_limit / 2) {
                break;
            }
            
            /* Switch statement to create more basic blocks */
            switch (j % 4) {
                case 0:
                    arr[i][j] = scale * j;
                    break;
                case 1:
                    arr[i][j] = scale + j;
                    break;
                case 2:
                    arr[i][j] = scale - j;
                    break;
                default:
                    arr[i][j] = scale / (j + 1);
                    break;
            }
            
            /* Conditional continue */
            if (j % 7 == 0) {
                continue;
            }
            
            result += arr[i][j];
        }
        
        /* Another if statement after inner loop */
        if (i % 10 == 0) {
            early_exit_trigger = !early_exit_trigger;
        }
    }
    
    printf("Result from first nest: %d\n", result);
}

/* Second function with different loop structure */
__attribute__((target("arch=armv8-a")))
void while_for_nest(int M, int arr[SIZE][SIZE]) {
    volatile int counter = 0;
    int sum = 0;
    
    /* Different nesting pattern: while outside, for inside */
    int k = 0;
    while (k < M) {
        volatile int inner_max = (k % 3) + 2;
        
        /* Loop-invariant computation */
        int base = k * k + 10;
        
        /* Multiple basic blocks before inner loop */
        if (k < M / 2) {
            base += 100;
        } else {
            base -= 50;
        }
        
        /* Inner for loop */
        for (int m = 0; m < inner_max; ++m) {
            /* Complex condition with early exit */
            if (counter > 100 && m > inner_max / 2) {
                break;
            }
            
            /* Nested if-else creating multiple blocks */
            if (m % 2 == 0) {
                arr[k][m] = base + m * 3;
            } else {
                arr[k][m] = base - m * 2;
            }
            
            /* Conditional continue with volatile check */
            volatile int skip_cond = (k + m) % 5;
            if (skip_cond == 0) {
                counter++;
                continue;
            }
            
            sum += arr[k][m];
        }
        
        /* Update with conditional */
        if (k % 4 == 0) {
            counter += 2;
        }
        k++;
    }
    
    printf("Result from second nest: %d\n", sum);
}

/* Third function with deeply nested loops */
__attribute__((target("arch=armv8-a")))
void triple_nested(int limit, int arr[SIZE][SIZE]) {
    int total = 0;
    volatile int mod_check = limit % 7;
    
    /* Triple nested loops */
    for (int a = 0; a < limit; ++a) {
        int outer_var = a * 3;
        
        for (int b = 0; b < a + 2; ++b) {
            volatile int inner_limit = b + 3;
            
            /* Switch to create multiple blocks */
            switch (b % 3) {
                case 0:
                    outer_var += 5;
                    break;
                case 1:
                    outer_var -= 2;
                    break;
                default:
                    outer_var *= 1;
                    break;
            }
            
            for (int c = 0; c < inner_limit; ++c) {
                /* Early break based on volatile */
                if (mod_check > 3 && c > inner_limit / 2) {
                    break;
                }
                
                /* Multiple conditional paths */
                if (c % 2 == 0) {
                    arr[a][b] += outer_var + c;
                } else {
                    arr[a][b] += outer_var - c;
                }
                
                /* Another continue condition */
                if ((a + b + c) % 11 == 0) {
                    continue;
                }
                
                total += arr[a][b];
            }
        }
    }
    
    printf("Result from triple nest: %d\n", total);
}

int main(int argc, char *argv[]) {
    /* Non-constant loop bounds from command line */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int M = (argc > 2) ? atoi(argv[2]) : 40;
    int P = (argc > 3) ? atoi(argv[3]) : 30;
    
    /* Initialize array with pseudo-random data */
    int arr[SIZE][SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Call functions with different loop nests */
    nested_loops_arm(N, arr);
    while_for_nest(M, arr);
    triple_nested(P, arr);
    
    /* Final checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            checksum += arr[i][j];
        }
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    return 0;
}
