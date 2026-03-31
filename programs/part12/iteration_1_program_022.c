#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 100

/* Function with ARM target attribute to enable hardware loop optimizations */
__attribute__((target("arch=armv8-a")))
void nested_loops_arm(int N, int M, int arr[SIZE][SIZE]) {
    volatile int early_exit_trigger = 0;
    int sum = 0;
    
    /* First nested loop: for inside for with complex control flow */
    for (int i = 0; i < N; ++i) {
        /* Loop-invariant code that varies with outer loop */
        int scale = i * 2;
        volatile int inner_limit = M - i % 3;
        
        /* Multiple basic blocks created by if-else */
        if (i % 5 == 0) {
            scale += 10;  /* Creates separate basic block */
        }
        
        for (int j = 0; j < inner_limit; ++j) {
            /* Conditional break based on volatile variable */
            if (early_exit_trigger && j > inner_limit / 2) {
                break;  /* Creates exit block */
            }
            
            /* Complex conditional with multiple basic blocks */
            if (j % 2 == 0) {
                arr[i][j] = scale + j;
            } else {
                arr[i][j] = scale - j;
                /* Nested if creates more blocks */
                if (j % 3 == 0) {
                    arr[i][j] *= 2;
                }
            }
            
            sum += arr[i][j];
            
            /* Another conditional continue */
            if (j % 7 == 0) {
                continue;
            }
            
            /* Additional computation */
            arr[i][j] += sum % 100;
        }
        
        /* Early exit from outer loop based on volatile */
        if (early_exit_trigger && i > N / 2) {
            /* Switch statement creates multiple blocks */
            switch (i % 4) {
                case 0: break;
                case 1: i += 1; break;
                case 2: continue;
                case 3: /* fall through */;
            }
        }
    }
    
    /* Second independent loop nest: while inside for, different pattern */
    int k = 0;
    while (k < N) {
        volatile int while_limit = M - k % 2;
        int m = 0;
        
        /* Loop-invariant calculation */
        int offset = k * 3;
        
        /* For loop inside while */
        for (m = 0; m < while_limit; ++m) {
            /* Conditional with multiple outcomes */
            if (k + m > N) {
                arr[k][m] = offset - m;
                /* Nested conditional */
                if (m % 4 == 0) {
                    break;  /* Early exit from inner loop */
                }
            } else if (k + m == N) {
                arr[k][m] = offset;
                continue;
            } else {
                arr[k][m] = offset + m;
            }
            
            sum += arr[k][m];
            
            /* Volatile-based condition */
            if (early_exit_trigger) {
                arr[k][m] /= 2;
            }
        }
        
        /* Complex update with conditional */
        k += (k % 3 == 0) ? 2 : 1;
        
        /* Another if creating basic block */
        if (k > N / 2) {
            volatile int temp = k;
            k = temp;  /* Prevent optimization */
        }
    }
    
    /* Third loop nest: do-while inside for */
    for (int p = 0; p < N / 2; ++p) {
        int q = 0;
        volatile int do_limit = M / 2;
        
        do {
            /* Switch creates multiple basic blocks */
            switch ((p + q) % 3) {
                case 0:
                    arr[p][q] = p * q;
                    break;
                case 1:
                    arr[p][q] = p + q;
                    if (q % 2 == 0) {
                        arr[p][q] *= -1;
                    }
                    break;
                case 2:
                    arr[p][q] = q - p;
                    /* Nested loop-invariant use */
                    int invariant = p * 2;
                    arr[p][q] += invariant;
                    break;
            }
            
            sum += arr[p][q];
            q++;
            
            /* Conditional continue in do-while */
            if (q % 5 == 0) {
                continue;
            }
            
        } while (q < do_limit && q < M);
        
        /* Another conditional */
        if (p % 7 == 0) {
            volatile int dummy = p;
            p += dummy % 2;  /* Prevent optimization */
        }
    }
    
    /* Use sum to prevent dead code elimination */
    arr[0][0] = sum % 1000;
}

/* Another function with different nesting pattern */
__attribute__((target("arch=armv8-a")))
void complex_nesting_arm(int N, int arr[SIZE][SIZE]) {
    volatile int trigger = 1;
    int total = 0;
    
    /* Nested loops where inner loop limit depends on outer in complex way */
    for (int x = 1; x < N; x *= 2) {
        int base = x * 2;
        
        /* Multiple if conditions before inner loop */
        if (x % 3 == 0) {
            base += x;
        } else if (x % 3 == 1) {
            base -= x;
        }
        
        for (int y = 0; y < base && y < SIZE; ++y) {
            /* Complex conditional chain */
            if (x > y) {
                arr[x][y] = x - y;
                if (trigger) {
                    arr[x][y] += total;
                }
            } else if (x < y) {
                arr[x][y] = y - x;
                /* Early continue */
                if (y % 11 == 0) {
                    continue;
                }
            } else {
                arr[x][y] = x * y;
                /* Possible break */
                if (trigger && y > base / 2) {
                    break;
                }
            }
            
            total += arr[x][y];
            
            /* Nested switch */
            switch (total % 4) {
                case 0: arr[x][y] &= 0xFF; break;
                case 1: arr[x][y] |= 0x0F; break;
                case 2: arr[x][y] ^= 0xAA; break;
                case 3: arr[x][y] = ~arr[x][y]; break;
            }
        }
        
        /* Outer loop conditional */
        if (trigger && x > N / 4) {
            x += 1;  /* Skip pattern */
        }
    }
    
    arr[1][1] = total % 1000;
}

int main(int argc, char *argv[]) {
    /* Use command-line argument for non-constant loop bounds */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    if (N > SIZE) N = SIZE;
    if (N < 10) N = 10;
    
    int M = (argc > 2) ? atoi(argv[2]) : 60;
    if (M > SIZE) M = SIZE;
    if (M < 10) M = 10;
    
    /* Initialize array with pseudo-random data */
    int arr[SIZE][SIZE];
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            arr[i][j] = rand() % 100;
        }
    }
    
    /* Call functions with hardware loop target attributes */
    nested_loops_arm(N, M, arr);
    complex_nesting_arm(N, arr);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            checksum += arr[i][j];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
