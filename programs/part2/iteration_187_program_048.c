#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function declarations
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void rarely_called(void);
void path_selector(int mode);
void process_data(int *data, int size);
void helper_function(int x);

// External functions from lib.c
extern void lib_hot_function(int n);
extern void lib_cold_function(void);
extern void lib_medium_function(int m);
extern int lib_compute(int a, int b);
extern void lib_rarely_used(void);

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line arguments to vary execution paths
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Seed random number generator
    srand(time(NULL));
    
    // Vary execution based on mode
    switch (mode) {
        case 1:
            // Mode 1: Heavy execution of hot functions
            for (int i = 0; i < iterations; i++) {
                hot_function(i);
                if (i % 10 == 0) {
                    lib_hot_function(i);
                }
                if (i % 100 == 0) {
                    medium_function(i);
                    lib_medium_function(i);
                }
            }
            break;
            
        case 2:
            // Mode 2: More balanced execution
            for (int i = 0; i < iterations / 2; i++) {
                hot_function(i);
                lib_hot_function(i);
                if (i % 5 == 0) {
                    medium_function(i);
                }
                if (i % 20 == 0) {
                    lib_medium_function(i);
                }
            }
            // Call some cold functions in mode 2
            cold_function();
            lib_cold_function();
            break;
            
        case 3:
            // Mode 3: Different execution pattern
            for (int i = 0; i < iterations; i++) {
                path_selector(i % 3);
                int result = lib_compute(i, i * 2);
                if (result % 7 == 0) {
                    rarely_called();
                }
            }
            break;
            
        default:
            // Default mode: Mix of everything
            for (int i = 0; i < iterations; i++) {
                hot_function(i);
                if (i % 50 == 0) {
                    cold_function();
                    lib_cold_function();
                }
                if (i % 25 == 0) {
                    medium_function(i);
                    lib_medium_function(i);
                }
            }
            lib_rarely_used();
            break;
    }
    
    // Process some data
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i * mode;
    }
    process_data(data, 100);
    
    return 0;
}

void hot_function(int iterations) {
    // This function is called many times
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += i;
        helper_function(i);
    }
    
    if (iterations % 2 == 0) {
        // Even iteration path
        for (int j = 0; j < 50; j++) {
            sum += j * j;
        }
    } else {
        // Odd iteration path
        for (int j = 0; j < 25; j++) {
            sum -= j;
        }
    }
}

void cold_function(void) {
    // Rarely called function
    printf("Cold function executed\n");
    static int call_count = 0;
    call_count++;
    
    // Some branching logic
    if (call_count % 3 == 0) {
        printf("Third call\n");
    } else if (call_count % 2 == 0) {
        printf("Even call\n");
    }
}

void medium_function(int count) {
    // Moderately called function
    int result = 0;
    for (int i = 0; i < count % 100; i++) {
        result += i;
        if (i % 10 == 0) {
            result *= 2;
        }
    }
    
    // Conditional execution
    if (result > 1000) {
        printf("Large result: %d\n", result);
    } else if (result < 0) {
        printf("Negative result\n");
    }
}

void rarely_called(void) {
    // Very rarely called
    printf("Rare function called\n");
    
    // Nested loops (rarely executed)
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (i == j) {
                printf("Diagonal: %d,%d\n", i, j);
            }
        }
    }
}

void path_selector(int mode) {
    // Function with multiple paths
    switch (mode) {
        case 0:
            hot_function(10);
            break;
        case 1:
            medium_function(50);
            break;
        case 2:
            cold_function();
            break;
        default:
            rarely_called();
            break;
    }
}

void process_data(int *data, int size) {
    // Process array data
    int total = 0;
    for (int i = 0; i < size; i++) {
        total += data[i];
        
        // Conditional inside loop
        if (data[i] % 3 == 0) {
            total += data[i] * 2;
        } else if (data[i] % 5 == 0) {
            total -= data[i];
        }
    }
    
    // More conditions
    if (total > 10000) {
        printf("Large total: %d\n", total);
    } else if (total < 0) {
        printf("Negative total\n");
    } else {
        printf("Moderate total: %d\n", total);
    }
}

void helper_function(int x) {
    // Small helper function
    if (x % 2 == 0) {
        x *= 2;
    } else {
        x /= 2;
    }
}
