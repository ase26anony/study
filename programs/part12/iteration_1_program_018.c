#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __GNUC__
#define TARGET_ARM __attribute__((target("arch=armv8-a")))
#else
#define TARGET_ARM
#endif

// Function with complex nested loops targeting ARM hardware loops
TARGET_ARM
void nested_loops_complex(int N, int M, int arr[200][200], volatile int* checksum) {
    volatile int early_exit_trigger = 0;
    volatile int outer_mod = 7;
    volatile int inner_mod = 3;
    
    // First nest: for-for with complex control flow
    for (int i = 0; i < N; ++i) {
        // Loop-invariant calculation for inner loop
        int scale = i * 2;
        int threshold = (i % 5) * 10;
        
        // Multiple basic blocks created by if-else
        if (i % 2 == 0) {
            // Additional basic block
            scale += 5;
        } else {
            // Another basic block
            threshold += 3;
        }
        
        // Inner loop with dependency on outer index
        for (int j = 0; j < i + 1; ++j) {
            // Complex conditional with multiple basic blocks
            if (j % inner_mod == 0) {
                arr[i][j] = scale * j;
            } else if (j % 2 == 0) {
                arr[i][j] = threshold - j;
            } else {
                arr[i][j] = i + j;
            }
            
            // Early exit based on volatile condition
            if (early_exit_trigger && (j > 10)) {
                break;  // Creates additional control flow
            }
            
            // Update checksum
            *checksum += arr[i][j];
        }
        
        // Conditional continue in outer loop
        if (i % outer_mod == 0) {
            continue;
        }
        
        // Additional computation between loops
        int temp = arr[i][0] * 3;
        if (temp > 100) {
            arr[i][0] = temp % 100;
        }
    }
    
    // Second independent loop nest: while-for with different pattern
    int k = 0;
    volatile int while_limit = M;
    while (k < while_limit) {
        // Switch statement creates multiple basic blocks
        switch (k % 4) {
            case 0:
                arr[k][k] *= 2;
                break;
            case 1:
                arr[k][k] /= 2;
                break;
            case 2:
                arr[k][k] += k;
                break;
            default:
                arr[k][k] -= k;
                break;
        }
        
        // Inner for loop with different bounds
        for (int l = k; l < N && l < k + 5; ++l) {
            // Nested if-else creating more basic blocks
            if (l % 2 == 0) {
                if (arr[k][l] > 50) {
                    arr[k][l] = 50;
                }
            } else {
                arr[k][l] += l * 3;
            }
            
            // Conditional break with volatile
            if (early_exit_trigger && (l > k + 2)) {
                break;
            }
            
            *checksum += arr[k][l];
        }
        
        // Loop variant update with condition
        k += (k % 3 == 0) ? 2 : 1;
    }
}

// Another function with different loop structure
TARGET_ARM
void another_nest(int limit, int arr[200][200], volatile int* sum) {
    volatile int mod_base = 11;
    
    // Triple nested loops
    for (int a = 0; a < limit && a < 50; ++a) {
        int base_val = a * a;
        
        for (int b = a; b < limit && b < a + 10; ++b) {
            // Multiple conditions creating basic blocks
            if (b % mod_base == 0) {
                continue;
            }
            
            int product = base_val * b;
            
            for (int c = 0; c < b && c < 8; ++c) {
                // Complex expression with conditional
                arr[a][b] += (product % (c + 1)) * ((a + b + c) % 7);
                
                // Early exit based on array value
                if (arr[a][b] > 1000) {
                    break;
                }
                
                *sum += arr[a][b];
            }
            
            // Post-inner-loop computation
            if (arr[a][b] < 0) {
                arr[a][b] = -arr[a][b];
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // Non-constant loop bounds from command line
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int M = (argc > 2) ? atoi(argv[2]) : 40;
    
    // Initialize array with pseudo-random data
    int arr[200][200];
    srand(time(NULL));
    for (int i = 0; i < 200; ++i) {
        for (int j = 0; j < 200; ++j) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum1 = 0;
    volatile int checksum2 = 0;
    
    // Call functions with complex nested loops
    nested_loops_complex(N, M, arr, &checksum1);
    another_nest(N, arr, &checksum2);
    
    // Additional loop nest in main (different optimization context)
    volatile int total = 0;
    volatile int dyn_limit = N / 2;
    
    // do-while inside for
    for (int x = 0; x < dyn_limit; ++x) {
        int y = 0;
        do {
            // Conditional with multiple outcomes
            switch ((x + y) % 3) {
                case 0:
                    arr[x][y] += x;
                    break;
                case 1:
                    arr[x][y] -= y;
                    break;
                case 2:
                    arr[x][y] *= 2;
                    break;
            }
            
            total += arr[x][y];
            y++;
        } while (y < x + 3 && y < 20);
        
        // Conditional break in outer loop
        if (total > 10000) {
            break;
        }
    }
    
    // Final checksum to prevent dead code elimination
    int final_sum = checksum1 + checksum2 + total;
    printf("Checksum: %d\n", final_sum);
    
    return 0;
}
