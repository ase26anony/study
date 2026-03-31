#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function declarations
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void rarely_called(void);
void always_called(void);
void conditional_function(int value);
void loop_function(int outer, int inner);
void switch_function(int val);
void recursive_function(int depth, int max);
void file_io_simulation(void);

// External functions from lib.c
extern void lib_hot_function(int iterations);
extern void lib_cold_function(void);
extern void lib_medium_function(int count);
extern int lib_complex_calculation(int a, int b);
extern void lib_conditional_path(int mode);

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    // Parse command line argument for mode
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    // Set different iteration counts based on mode
    switch (mode) {
        case 1:
            iterations = 10000;  // Many iterations for hot paths
            printf("Running in mode 1 (hot paths)\n");
            break;
        case 2:
            iterations = 100;    // Fewer iterations
            printf("Running in mode 2 (medium paths)\n");
            break;
        case 3:
            iterations = 10;     // Very few iterations
            printf("Running in mode 3 (cold paths)\n");
            break;
        default:
            iterations = 5000;
            printf("Running in default mode\n");
            break;
    }
    
    // Seed random number generator
    srand(time(NULL));
    
    // Always execute these
    always_called();
    
    // Execute functions with different frequencies based on mode
    if (mode == 1) {
        // Hot path - execute hot functions many times
        for (int i = 0; i < iterations; i++) {
            hot_function(i);
            lib_hot_function(i % 100);
            
            if (i % 10 == 0) {
                medium_function(i);
                lib_medium_function(i % 50);
            }
            
            if (i % 100 == 0) {
                cold_function();
                lib_cold_function();
            }
        }
        
        // Execute loop function with many iterations
        loop_function(100, 50);
        
        // Execute recursive function
        recursive_function(0, 10);
        
    } else if (mode == 2) {
        // Medium path - balanced execution
        for (int i = 0; i < iterations; i++) {
            if (i % 2 == 0) {
                hot_function(i);
            } else {
                medium_function(i);
            }
            
            if (i % 5 == 0) {
                lib_hot_function(i);
            }
            
            if (i % 20 == 0) {
                cold_function();
                lib_cold_function();
            }
        }
        
        loop_function(50, 20);
        recursive_function(0, 5);
        
    } else {
        // Cold path - minimal execution
        for (int i = 0; i < iterations; i++) {
            if (i % 10 == 0) {
                medium_function(i);
            }
            
            if (i % 50 == 0) {
                hot_function(i);
                lib_hot_function(i);
            }
        }
        
        loop_function(10, 5);
        recursive_function(0, 3);
    }
    
    // Execute conditional functions based on random values
    for (int i = 0; i < iterations / 10; i++) {
        int val = rand() % 100;
        conditional_function(val);
        lib_conditional_path(val % 3);
        
        // Execute switch function
        switch_function(val % 5);
    }
    
    // Execute rarely called functions
    if (mode == 1 && iterations > 5000) {
        rarely_called();
        file_io_simulation();
    }
    
    // Complex calculation
    int result = lib_complex_calculation(mode, iterations);
    printf("Final result: %d\n", result);
    
    return 0;
}

void hot_function(int iterations) {
    int sum = 0;
    // Hot loop - executed many times
    for (int i = 0; i < 100; i++) {
        sum += i * iterations;
    }
    
    // Conditional inside hot loop
    if (sum > 1000000) {
        sum /= 2;
    } else {
        sum *= 2;
    }
}

void cold_function(void) {
    // Rarely executed function
    static int call_count = 0;
    call_count++;
    
    // Complex but rarely executed logic
    int result = 0;
    for (int i = 0; i < 10; i++) {
        result += i * call_count;
    }
}

void medium_function(int count) {
    // Medium frequency function
    int array[10];
    
    for (int i = 0; i < 10; i++) {
        array[i] = i * count;
    }
    
    // Nested loop
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            sum += array[i] + j;
        }
    }
}

void rarely_called(void) {
    // Very rarely called function
    printf("Rare function called!\n");
    
    // Deeply nested conditionals
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            // Path 1
            for (int j = 0; j < 2; j++) {
                if (j == 1) {
                    printf("Nested rare path\n");
                }
            }
        } else if (i == 1) {
            // Path 2
            printf("Alternative rare path\n");
        } else {
            // Path 3
            printf("Default rare path\n");
        }
    }
}

void always_called(void) {
    // Always executed function
    printf("Program starting...\n");
}

void conditional_function(int value) {
    // Function with multiple conditional paths
    if (value < 20) {
        // Path A
        int temp = value * 2;
        if (temp > 30) {
            temp /= 2;
        }
    } else if (value < 50) {
        // Path B
        for (int i = 0; i < value % 10; i++) {
            printf(".");
        }
        printf("\n");
    } else if (value < 80) {
        // Path C
        int result = 1;
        for (int i = 1; i <= (value % 7); i++) {
            result *= i;
        }
    } else {
        // Path D
        printf("High value: %d\n", value);
    }
}

void loop_function(int outer, int inner) {
    // Complex nested loops
    int matrix[10][10];
    
    for (int i = 0; i < outer && i < 10; i++) {
        for (int j = 0; j < inner && j < 10; j++) {
            matrix[i][j] = i * j;
            
            if (matrix[i][j] > 50) {
                matrix[i][j] = 50;
            }
        }
    }
    
    // Process matrix
    int total = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            total += matrix[i][j];
        }
    }
}

void switch_function(int val) {
    // Switch statement with multiple cases
    switch (val) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            // Fall through
        case 2:
            printf("Case 1 or 2\n");
            break;
        case 3:
            printf("Case 3\n");
            for (int i = 0; i < 3; i++) {
                printf("Loop in case 3\n");
            }
            break;
        case 4:
            printf("Case 4\n");
            if (val == 4) {
                printf("Nested if in case 4\n");
            }
            break;
        default:
            printf("Default case\n");
            break;
    }
}

void recursive_function(int depth, int max) {
    // Recursive function
    if (depth >= max) {
        return;
    }
    
    printf("Recursion depth: %d\n", depth);
    
    // Recursive call
    recursive_function(depth + 1, max);
    
    // Post-recursion processing
    if (depth % 2 == 0) {
        printf("Even depth: %d\n", depth);
    }
}

void file_io_simulation(void) {
    // Simulate file I/O operations
    FILE *fp = tmpfile();
    if (fp) {
        for (int i = 0; i < 10; i++) {
            fprintf(fp, "Line %d\n", i);
        }
        fclose(fp);
    }
}
