#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100

/* Function with ARM target attribute to ensure hardware loop pass runs */
__attribute__((target("arch=armv8-a")))
void nested_loops_arm(int N, int arr[SIZE][SIZE]) {
    volatile int outer_limit = N;
    volatile int early_exit_flag = 0;
    int sum = 0;
    
    /* First nested loop: for-for with complex control flow */
    for (int i = 0; i < outer_limit; ++i) {
        /* Loop-invariant calculation */
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
        
        /* Inner loop with early exit possibility */
        for (int j = 0; j < inner_limit; ++j) {
            /* Conditional break creating additional basic block */
            if (early_exit_flag && j > inner_limit / 2) {
                break;
            }
            
            /* Complex computation with array access */
            arr[i][j] = (i * scale + j) % 256;
            
            /* Conditional continue */
            if (j % 7 == 0) {
                continue;
            }
            
            sum += arr[i][j];
            
            /* Switch statement for more basic blocks */
            switch (j % 4) {
                case 0: arr[i][j] += 1; break;
                case 1: arr[i][j] -= 1; break;
                case 2: arr[i][j] *= 2; break;
                case 3: arr[i][j] /= 2; break;
            }
        }
        
        /* Reset early exit flag occasionally */
        if (i % 10 == 0) {
            early_exit_flag = !early_exit_flag;
        }
    }
    
    printf("Sum after first loops: %d\n", sum);
}

/* Second function with different loop structure */
__attribute__((target("arch=armv8-a")))
void while_for_nested(int M, int arr[SIZE][SIZE]) {
    volatile int counter = M;
    int k = 0;
    int total = 0;
    
    /* while-for nesting pattern */
    while (counter > 0) {
        volatile int inner_max = (counter % 7) + 3;
        
        /* Pre-inner loop computation */
        int base = k * 3;
        
        /* Inner for loop */
        for (int m = 0; m < inner_max; ++m) {
            /* Conditional with multiple paths */
            if (m % 2 == 0) {
                arr[k % SIZE][m % SIZE] = base + m;
                total += arr[k % SIZE][m % SIZE];
            } else {
                arr[k % SIZE][m % SIZE] = base - m;
                total -= arr[k % SIZE][m % SIZE];
                
                /* Nested if for more complexity */
                if (m % 3 == 0 && k > 5) {
                    arr[k % SIZE][m % SIZE] *= 2;
                }
            }
            
            /* Early continue */
            if (m == inner_max / 2) {
                continue;
            }
        }
        
        k++;
        counter--;
        
        /* Break condition in while loop */
        if (k > 50) {
            break;
        }
    }
    
    printf("Total after while-for: %d\n", total);
}

/* Third function with triple nesting */
__attribute__((target("arch=armv8-a")))
void triple_nested(int limit, int arr[SIZE][SIZE]) {
    volatile int L1 = limit;
    volatile int L2 = limit / 2;
    int checksum = 0;
    
    for (int x = 0; x < L1; x++) {
        for (int y = 0; y < L2; y++) {
            /* Middle loop invariant */
            int offset = x * y;
            
            for (int z = 0; z < 10; z++) {
                /* Complex condition with multiple basic blocks */
                if (z % 2 == 0) {
                    if (x > y) {
                        arr[x][y] += offset + z;
                    } else {
                        arr[x][y] -= offset - z;
                    }
                } else {
                    arr[x][y] = arr[x][y] * 2 % 1000;
                }
                
                checksum ^= arr[x][y];
                
                /* Conditional break in innermost loop */
                volatile int stop = z > 5 && (x + y) % 3 == 0;
                if (stop) {
                    break;
                }
            }
        }
    }
    
    printf("Checksum after triple nested: %d\n", checksum);
}

int main(int argc, char *argv[]) {
    /* Non-constant loop bounds from command line */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int M = (argc > 2) ? atoi(argv[2]) : 30;
    
    /* Initialize array with volatile data */
    int arr[SIZE][SIZE];
    srand(time(NULL));
    
    /* Fill array with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            arr[i][j] = rand() % 1000;
        }
    }
    
    /* Call functions with different loop nesting patterns */
    nested_loops_arm(N, arr);
    while_for_nested(M, arr);
    triple_nested(N / 2, arr);
    
    /* Final computation to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            final_sum += arr[i][j];
        }
    }
    
    printf("Final array sum: %d\n", final_sum);
    
    return 0;
}
