#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100

/* Function with ARM target attribute to enable hardware loop optimizations */
__attribute__((target("arch=armv8-a")))
void nested_loops_arm(int N, int arr[SIZE][SIZE]) {
    volatile int early_break_cond = 0;
    int sum = 0;
    
    /* First nested loop: for inside for with complex control flow */
    for (int i = 0; i < N; ++i) {
        /* Loop-invariant code that varies with outer loop */
        int scale = i * 2 + 1;
        volatile int inner_limit = i + 5;
        
        /* Multiple basic blocks created by if statement */
        if (i % 3 == 0) {
            scale += 10;  /* Creates separate basic block */
        }
        
        /* Inner loop with dependency on outer index */
        for (int j = 0; j < inner_limit; ++j) {
            /* Conditional early exit creates more CFG complexity */
            if (early_break_cond && j > inner_limit/2) {
                break;  /* Creates exit block */
            }
            
            /* Complex calculation with multiple basic blocks */
            int val = arr[i][j] * scale;
            if (val % 2 == 0) {
                sum += val;
            } else {
                sum -= val / 2;  /* Alternative path */
            }
            
            /* Another conditional that might affect bitmap */
            if (j == i && i > N/2) {
                sum += 1000;  /* Rare path */
            }
        }
        
        /* Additional code between loops creates more blocks */
        if (i % 7 == 0) {
            early_break_cond = !early_break_cond;
        }
    }
    
    printf("Sum from first nest: %d\n", sum);
}

/* Another function with different loop structure */
__attribute__((target("arch=armv8-a")))
void mixed_loops_arm(int M, int arr[SIZE][SIZE]) {
    volatile int counter = 0;
    int total = 0;
    int row = 0;
    
    /* Second loop nest: while outside, for inside */
    while (row < M) {
        volatile int col_limit = (row % 4) + 3;
        
        /* Switch statement creates multiple basic blocks */
        switch (row % 3) {
            case 0: col_limit += 5; break;
            case 1: col_limit += 10; break;
            case 2: col_limit += 15; break;
        }
        
        /* Inner for loop */
        for (int col = 0; col < col_limit; ++col) {
            /* Conditional continue */
            if (col % 2 == 0 && counter > 5) {
                continue;
            }
            
            /* Nested if-else for more blocks */
            int elem = arr[row][col];
            if (elem > 50) {
                total += elem * 2;
            } else if (elem < 20) {
                total += elem / 2;
            } else {
                total += elem;
            }
            
            /* Volatile check for unpredictable behavior */
            if (counter++ > 100) {
                counter = 0;
            }
        }
        
        /* Conditional break in outer loop */
        if (row > M/2 && total > 10000) {
            break;
        }
        
        row++;
    }
    
    printf("Total from second nest: %d\n", total);
}

/* Third function with deeply nested loops */
__attribute__((target("arch=armv8-a")))
void triple_nested_arm(int limit, int arr[SIZE][SIZE]) {
    int result = 0;
    
    /* Triple nested loops for complex hierarchy */
    for (int x = 0; x < limit; x += 2) {
        volatile int y_limit = x + 3;
        
        for (int y = 0; y < y_limit; ++y) {
            /* Loop-invariant for middle loop */
            int offset = x * y + 7;
            
            for (int z = 0; z < 5; ++z) {
                /* Complex conditional structure */
                if (z == 0) {
                    result += arr[x][y] + offset;
                } else if (z % 2 == 0) {
                    result += arr[y][x] - z;
                } else {
                    result += offset * z;
                }
                
                /* Early exit from innermost loop */
                if (result > 5000 && z > 2) {
                    break;
                }
            }
            
            /* Conditional that might skip rest of middle loop */
            if (y == x && x > limit/3) {
                y += 2;  /* Skip some iterations */
            }
        }
    }
    
    printf("Result from triple nest: %d\n", result);
}

int main(int argc, char *argv[]) {
    /* Non-constant loop bounds from command line */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int M = (argc > 2) ? atoi(argv[2]) : 40;
    int limit = (argc > 3) ? atoi(argv[3]) : 30;
    
    /* Initialize array with pseudo-random data */
    int arr[SIZE][SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Call functions with different loop structures */
    nested_loops_arm(N, arr);
    mixed_loops_arm(M, arr);
    triple_nested_arm(limit, arr);
    
    /* Final checksum to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            final_sum += arr[i][j];
        }
    }
    printf("Final checksum: %d\n", final_sum);
    
    return 0;
}
