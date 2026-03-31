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
void recursive_function(int depth);
void loop_variations(int type);

// External functions from lib.c
extern void lib_hot_function(int n);
extern void lib_cold_function(void);
extern void lib_complex_function(int mode);
extern int lib_calculate(int a, int b);

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line argument for mode
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    // Set different iteration counts based on mode
    if (mode == 1) {
        iterations = 10000;  // Many iterations for hot paths
    } else if (mode == 2) {
        iterations = 100;    // Fewer iterations
    } else if (mode == 3) {
        iterations = 5000;   // Medium iterations
    }
    
    srand(time(NULL));
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    // Vary execution paths based on mode
    if (mode == 1) {
        // Mode 1: Focus on hot functions
        for (int i = 0; i < iterations; i++) {
            hot_function(i);
            if (i % 10 == 0) {
                lib_hot_function(i);
            }
        }
        medium_function(iterations / 10);
    } else if (mode == 2) {
        // Mode 2: More balanced execution
        for (int i = 0; i < iterations / 2; i++) {
            hot_function(i);
            cold_function();
            if (i % 5 == 0) {
                lib_hot_function(i);
            }
        }
        medium_function(iterations / 5);
        rarely_called();
    } else {
        // Mode 3: Different pattern
        for (int i = 0; i < iterations; i++) {
            if (i % 3 == 0) {
                hot_function(i);
            } else if (i % 7 == 0) {
                cold_function();
            }
            lib_complex_function(i % 4);
        }
        path_selector(mode);
    }
    
    // Always execute some common paths
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i * mode;
    }
    process_data(data, 100);
    
    // Vary recursive depth based on mode
    recursive_function(mode * 2);
    
    // Different loop types
    loop_variations(mode);
    
    // Call library functions
    int result = lib_calculate(mode, iterations);
    printf("Final result: %d\n", result);
    
    return 0;
}

void hot_function(int iterations) {
    int sum = 0;
    // Hot loop - executed many times
    for (int i = 0; i < 100; i++) {
        sum += i * iterations;
        if (sum > 1000000) {
            sum = 0;  // Reset to avoid overflow
        }
    }
    
    // Conditional that's usually true
    if (iterations % 2 == 0) {
        sum *= 2;
    } else {
        sum /= 2;
    }
}

void cold_function(void) {
    // Rarely executed function
    static int call_count = 0;
    call_count++;
    
    // Complex but rarely used logic
    if (call_count < 5) {
        printf("Cold function called %d times\n", call_count);
    }
    
    // Nested conditionals
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0) {
            // Do something
            int x = i * i;
            (void)x;
        }
    }
}

void medium_function(int count) {
    // Medium frequency function
    int total = 0;
    for (int i = 0; i < count; i++) {
        total += i;
        if (i % 50 == 0) {
            total -= i / 2;
        }
    }
    
    // Switch statement for variety
    switch (count % 4) {
        case 0:
            total *= 2;
            break;
        case 1:
            total /= 2;
            break;
        case 2:
            total += 100;
            break;
        default:
            total -= 50;
            break;
    }
}

void rarely_called(void) {
    // Very rarely called
    printf("This function is rarely called\n");
    
    // Deeply nested logic
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == j) {
                for (int k = 0; k < 3; k++) {
                    if (k != i) {
                        // Complex but rarely executed
                        int value = i * j * k;
                        (void)value;
                    }
                }
            }
        }
    }
}

void path_selector(int mode) {
    // Function with multiple execution paths
    switch (mode) {
        case 1:
            printf("Path 1 selected\n");
            break;
        case 2:
            printf("Path 2 selected\n");
            break;
        case 3:
            printf("Path 3 selected\n");
            break;
        default:
            printf("Default path\n");
            break;
    }
    
    // Conditional chain
    if (mode < 2) {
        // Path A
    } else if (mode < 4) {
        // Path B
    } else {
        // Path C
    }
}

void process_data(int *data, int size) {
    // Process array data
    for (int i = 0; i < size; i++) {
        if (data[i] > 50) {
            data[i] = data[i] * 2;
        } else {
            data[i] = data[i] / 2;
        }
        
        // Nested condition
        if (i % 20 == 0) {
            for (int j = 0; j < 10; j++) {
                data[i] += j;
            }
        }
    }
}

void recursive_function(int depth) {
    // Recursive function
    if (depth <= 0) {
        return;
    }
    
    // Vary recursion pattern
    if (depth % 2 == 0) {
        recursive_function(depth - 1);
    } else {
        recursive_function(depth - 2);
    }
    
    // Some computation
    int local = depth * 10;
    (void)local;
}

void loop_variations(int type) {
    // Different loop structures
    if (type == 1) {
        // Simple loop
        for (int i = 0; i < 100; i++) {
            // Do work
            int temp = i * type;
            (void)temp;
        }
    } else if (type == 2) {
        // While loop
        int i = 0;
        while (i < 50) {
            i += type;
        }
    } else {
        // Do-while loop
        int i = 0;
        do {
            i++;
        } while (i < 30);
    }
}
