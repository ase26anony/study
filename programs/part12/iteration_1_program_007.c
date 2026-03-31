#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100

// Function with ARM target attribute to enable hardware loop optimizations
__attribute__((target("arch=armv8-a")))
void nested_loops_arm(int N, int arr[SIZE][SIZE]) {
    volatile int outer_limit = N;
    volatile int early_exit_trigger = N / 3;
    int sum = 0;
    
    // First nested loop: for-for with complex control flow
    for (int i = 0; i < outer_limit; ++i) {
        // Loop-invariant calculation
        int scale = i * 2 + 1;
        volatile int inner_limit = i + 5;
        
        // Multiple basic blocks created by if-else
        if (i % 2 == 0) {
            // Additional basic block
            scale += 10;
        } else {
            scale -= 5;
        }
        
        // Inner loop with early exit
        for (int j = 0; j < inner_limit; ++j) {
            // Conditional break creating more CFG complexity
            if (j > early_exit_trigger) {
                break;  // Early exit creates separate basic block
            }
            
            // Complex expression with loop-invariant part
            arr[i][j] = (i * j + scale) % 256;
            
            // Conditional continue
            if ((i + j) % 7 == 0) {
                continue;  // Another basic block
            }
            
            sum += arr[i][j];
        }
        
        // Switch statement for additional basic blocks
        switch (i % 4) {
            case 0: sum += 1; break;
            case 1: sum += 2; break;
            case 2: sum += 3; break;
            default: sum += 4;
        }
    }
    
    // Second independent loop nest with different structure
    int k = 0;
    volatile int while_limit = N / 2;
    
    // while-for nested loop
    while (k < while_limit) {
        volatile int inner_for_limit = (k * 3) % 10 + 5;
        
        // Inner for loop
        for (int m = 0; m < inner_for_limit; ++m) {
            // Array access with different pattern
            arr[k][m] = (arr[k][m] + k * m) % 256;
            
            // Nested if for more basic blocks
            if (m % 3 == 0) {
                if (k % 2 == 0) {
                    sum += arr[k][m] * 2;
                } else {
                    sum -= arr[k][m];
                }
            }
            
            // Early break based on volatile
            if (m > early_exit_trigger) {
                break;
            }
        }
        
        k++;
        
        // Loop with multiple exit points
        if (k > N / 3) {
            // Another basic block
            sum += 1000;
        }
    }
    
    printf("ARM function checksum: %d\n", sum);
}

// Another function with different loop structure
__attribute__((target("arch=armv8-a")))
void complex_nested_loops(int N, int arr[SIZE][SIZE]) {
    volatile int limit1 = N;
    volatile int limit2 = N / 2;
    int total = 0;
    
    // Triple nested loops
    for (int a = 0; a < limit1; a += 2) {
        int base = a * 10;
        
        for (int b = 1; b < limit2; b++) {
            // Multiple basic blocks before inner loop
            int modifier;
            if (b % 3 == 0) {
                modifier = 5;
            } else if (b % 3 == 1) {
                modifier = 10;
            } else {
                modifier = 15;
            }
            
            for (int c = 0; c < b; c++) {
                // Complex indexing
                int idx1 = (a + c) % SIZE;
                int idx2 = (b * c) % SIZE;
                
                arr[idx1][idx2] = (base + b * c + modifier) % 256;
                
                // Conditional with multiple paths
                if (arr[idx1][idx2] > 128) {
                    total += arr[idx1][idx2];
                } else if (arr[idx1][idx2] > 64) {
                    total += arr[idx1][idx2] / 2;
                } else {
                    total += 1;
                }
                
                // Volatile-based early exit
                volatile int exit_cond = limit1 / 4;
                if (c > exit_cond) {
                    break;
                }
            }
        }
    }
    
    // Do-while with for inside
    int x = 0;
    do {
        volatile int inner_max = (x % 5) + 3;
        
        for (int y = 0; y < inner_max; y++) {
            arr[x % SIZE][y % SIZE] = (arr[x % SIZE][y % SIZE] + x * y) % 256;
            total += arr[x % SIZE][y % SIZE];
            
            // Nested switch for CFG complexity
            switch (y % 3) {
                case 0: total += x; break;
                case 1: total += y; break;
                case 2: total += x + y; break;
            }
        }
        
        x++;
    } while (x < N / 3);
    
    printf("Complex function checksum: %d\n", total);
}

int main(int argc, char *argv[]) {
    // Non-constant loop bounds from command line
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    if (N > SIZE) N = SIZE;
    
    // Initialize array with pseudo-random data
    int arr[SIZE][SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            arr[i][j] = rand() % 256;
        }
    }
    
    // Call functions with hardware loop target attributes
    nested_loops_arm(N, arr);
    complex_nested_loops(N, arr);
    
    // Final checksum to prevent dead code elimination
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            final_sum += arr[i][j];
        }
    }
    
    printf("Final array checksum: %d\n", final_sum);
    return 0;
}
