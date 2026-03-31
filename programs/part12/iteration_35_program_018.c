#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function declarations from other compilation units
extern void func_a(int count);
extern void func_b(int count);
extern void func_c(int count);
extern void hot_function(int iterations);
extern void cold_function(void);

// Global variables for different execution paths
static int global_counter = 0;

// Main function with multiple conditional paths
int main(int argc, char *argv[]) {
    int input_value = 0;
    
    // Parse input from command line or file
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
    
    // Complex branching structure
    if (input_value < 0) {
        // Path 1: Negative values
        printf("Negative path\n");
        func_a(10);
        cold_function();
    } else if (input_value == 0) {
        // Path 2: Zero
        printf("Zero path\n");
        func_b(5);
        cold_function();
    } else if (input_value > 0 && input_value <= 100) {
        // Path 3: Small positive
        printf("Small positive path\n");
        func_c(input_value);
        
        // Nested condition
        if (input_value % 2 == 0) {
            printf("Even number\n");
            global_counter += 2;
        } else {
            printf("Odd number\n");
            global_counter += 1;
        }
    } else {
        // Path 4: Large positive - create hot blocks
        printf("Large positive path - creating hot blocks\n");
        
        // This will create hot execution counts
        hot_function(input_value);
        
        // Multiple loops with different characteristics
        for (int i = 0; i < input_value / 10; i++) {
            global_counter++;
            if (i % 3 == 0) {
                func_a(1);
            } else if (i % 3 == 1) {
                func_b(1);
            } else {
                func_c(1);
            }
        }
        
        // Another hot loop
        int j = 0;
        while (j < input_value / 20) {
            global_counter += j;
            j++;
        }
    }
    
    // Final switch statement for additional coverage
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
    
    printf("Final counter: %d\n", global_counter);
    return 0;
}
