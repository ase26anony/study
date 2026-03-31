#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function declarations from other modules
void func_a(int count);
void func_b(int count);
void func_c(int count);
void process_file(const char* filename);

// Global counter for hot/cold analysis
static long global_counter = 0;

// Main function with multiple execution paths
int main(int argc, char *argv[]) {
    int input_value = 0;
    
    // Parse command line arguments
    if (argc < 2) {
        printf("Usage: %s <value> [filename]\n", argv[0]);
        return 1;
    }
    
    input_value = atoi(argv[1]);
    
    // Branch 1: Different ranges trigger different execution paths
    if (input_value < 0) {
        // Cold path - rarely executed
        printf("Negative value path\n");
        for (int i = 0; i < 10; i++) {
            global_counter++;
        }
    } else if (input_value < 100) {
        // Medium path
        printf("Medium value path\n");
        for (int i = 0; i < 1000; i++) {
            global_counter++;
        }
        func_a(input_value);
    } else if (input_value < 1000) {
        // Hot path - frequently executed
        printf("Hot value path\n");
        for (int i = 0; i < 1000000; i++) {
            global_counter++;
        }
        func_b(input_value);
    } else {
        // Very hot path
        printf("Very hot path\n");
        for (int i = 0; i < 5000000; i++) {
            global_counter++;
        }
        func_c(input_value);
    }
    
    // Additional conditional execution
    if (input_value % 2 == 0) {
        printf("Even value\n");
        // Nested loop for more coverage
        for (int i = 0; i < input_value % 100; i++) {
            global_counter += i;
        }
    } else {
        printf("Odd value\n");
    }
    
    // Process file if provided
    if (argc > 2) {
        process_file(argv[2]);
    }
    
    // Switch statement for additional branch coverage
    switch (input_value % 4) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            break;
        case 3:
            printf("Case 3\n");
            break;
    }
    
    printf("Final counter: %ld\n", global_counter);
    return 0;
}
