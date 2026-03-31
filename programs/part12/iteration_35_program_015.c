#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function prototypes from other compilation units
void hot_function(int iterations);
void cold_function(void);
void medium_function(int value);
void conditional_function(int a, int b);

// Global counter for hot blocks
static int global_counter = 0;

// Main function with multiple execution paths
int main(int argc, char *argv[]) {
    int input_value = 0;
    
    // Read input from command line or file
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        // Try to read from file
        FILE *fp = fopen("input.txt", "r");
        if (fp) {
            fscanf(fp, "%d", &input_value);
            fclose(fp);
        }
    }
    
    printf("Running with input: %d\n", input_value);
    
    // Branch 1: Very hot path (high execution count)
    if (input_value > 1000) {
        // This creates a hot block
        for (int i = 0; i < input_value; i++) {
            global_counter++;
            if (i % 100 == 0) {
                hot_function(10);
            }
        }
    }
    // Branch 2: Medium path
    else if (input_value > 100) {
        for (int i = 0; i < input_value / 10; i++) {
            medium_function(i);
        }
    }
    // Branch 3: Cold path
    else if (input_value > 0) {
        cold_function();
    }
    // Branch 4: Very cold path (rarely executed)
    else {
        printf("Zero or negative input - cold path\n");
    }
    
    // Another conditional based on input parity
    if (input_value % 2 == 0) {
        conditional_function(input_value, input_value * 2);
    } else {
        conditional_function(input_value, input_value / 2);
    }
    
    // Switch statement for additional coverage
    switch (input_value % 4) {
        case 0:
            printf("Case 0 executed\n");
            break;
        case 1:
            printf("Case 1 executed\n");
            break;
        case 2:
            printf("Case 2 executed\n");
            break;
        case 3:
            printf("Case 3 executed\n");
            break;
    }
    
    return 0;
}
