#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100

/* Function with ARM target attribute to enable hardware loop optimizations */
__attribute__((target("arch=armv8-a")))
void nested_loops_arm(int N, int arr[SIZE][SIZE]) {
    volatile int early_break_cond = 0;
    int sum = 0;
    
    /* First nested loop structure: for inside for with complex control flow */
    for (int i = 0; i < N; ++i) {
        /* Loop-invariant calculation that varies with outer loop */
        int scale = i * 2 + 1;
        volatile int inner_limit = i + 5;
        
        /* Multiple basic blocks created by if-else */
        if (i % 3 == 0) {
            scale += 10;
        } else if (i % 3 == 1) {
            scale -= 5;
        } else {
            /* Empty else creates another basic block */
        }
        
        /* Inner loop with dependency on outer loop index */
        for (int j = 0; j < inner_limit; ++j) {
            /* Complex conditional with early exit possibility */
            if (early_break_cond && j > inner_limit / 2) {
                /* Early break creates separate basic block */
                break;
            }
            
            /* Conditional continue */
            if (j % 7 == 0) {
                continue;
            }
            
            /* Switch statement to create more basic blocks */
            switch (j % 4) {
                case 0: arr[i][j] += scale * 2; break;
                case 1: arr[i][j] -= scale; break;
                case 2: arr[i][j] *= scale / 2; break;
                default: arr[i][j] = scale; break;
            }
            
            sum += arr[i][j];
        }
        
        /* Another if statement after inner loop */
        if (sum > 1000) {
            sum /= 2;
        }
    }
    
    /* Second independent loop nest in same function: while inside for */
    int k = 0;
    volatile int while_limit = N / 2;
    
    while (k < while_limit) {
        /* Loop-invariant code */
        int base = k * 3;
        
        /* Inner for loop with different pattern */
        for (int m = N - 1; m >= 0; m -= 2) {
            /* Conditional with volatile to prevent optimization */
            volatile int cond = k % 2;
            if (cond) {
                arr[k][m] += base + m;
            } else {
                arr[k][m] -= base - m;
            }
            
            /* Another conditional break */
            if (m < N / 4) {
                break;
            }
            
            sum += arr[k][m] % 256;
        }
        
        k++;
        
        /* Continue condition */
        if (k % 5 == 0) {
            continue;
        }
    }
    
    /* Prevent dead code elimination */
    printf("Checksum 1: %d\n", sum % 1000);
}

/* Another function with different loop structure */
__attribute__((target("arch=armv8-a")))
void complex_nested_loops(int N, int arr[SIZE][SIZE]) {
    int total = 0;
    volatile int mod_base = 7;
    
    /* Triple nested loops */
    for (int x = 0; x < N / 2; ++x) {
        int x_factor = x * x;
        
        for (int y = 0; y < x + 3; ++y) {
            /* Multiple basic blocks with if-else chain */
            if (y % 2 == 0) {
                x_factor += y;
            } else if (y % 3 == 0) {
                x_factor -= y * 2;
            }
            
            int z = 0;
            volatile int z_limit = (x + y) % 10 + 5;
            
            /* while loop as innermost */
            while (z < z_limit) {
                /* Complex condition with volatile */
                volatile int rand_cond = rand() % 100;
                
                if (rand_cond > 50) {
                    arr[x][y] += x_factor + z;
                    total += arr[x][y];
                } else if (rand_cond > 25) {
                    arr[x][y] -= z;
                    total -= arr[x][y] % 100;
                } else {
                    /* Empty else creates basic block */
                }
                
                /* Early exit based on volatile */
                if (rand_cond > 90) {
                    break;
                }
                
                z += (z % 2) + 1;  /* Variable increment */
            }
            
            /* Another if after while */
            if (total > 5000) {
                total = 5000;
            }
        }
    }
    
    printf("Checksum 2: %d\n", total % 1000);
}

/* Function without target attribute for comparison */
void simple_nested_loop(int N, int arr[SIZE][SIZE]) {
    int sum = 0;
    
    /* Different nesting pattern */
    for (int a = N - 1; a >= 0; --a) {
        volatile int b_limit = (a * 2) % 15 + 3;
        int b = 0;
        
        while (b < b_limit) {
            /* Switch with multiple cases */
            switch ((a + b) % 5) {
                case 0: arr[a][b] += a * b; break;
                case 1: arr[a][b] -= a + b; break;
                case 2: arr[a][b] *= 2; break;
                case 3: arr[a][b] /= (b + 1); break;
                case 4: arr[a][b] = a - b; break;
            }
            
            sum += arr[a][b];
            
            /* Conditional continue */
            if (b % 4 == 0) {
                b += 2;
                continue;
            }
            
            b++;
        }
    }
    
    printf("Checksum 3: %d\n", sum % 1000);
}

int main(int argc, char *argv[]) {
    /* Non-constant loop bounds from command line */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    if (N > SIZE) N = SIZE;
    if (N < 10) N = 10;
    
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
    complex_nested_loops(N, arr);
    simple_nested_loop(N, arr);
    
    /* Final checksum to ensure all loops executed */
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            final_sum += arr[i][j] % 256;
        }
    }
    
    printf("Final checksum: %d\n", final_sum % 10000);
    
    return 0;
}
