/* test_prog.c - Main program for coverage testing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function declarations from other modules */
extern void func1(int iterations);
extern void func2(int iterations);
extern void func3(int iterations);
extern void hot_function(int count);
extern void cold_function(void);

/* Global variables for different execution paths */
static int global_counter = 0;

/* Main function with multiple branches */
int main(int argc, char *argv[]) {
    int input_value = 0;
    
    /* Parse command line argument */
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        /* Read from file if no argument */
        FILE *fp = fopen("input.txt", "r");
        if (fp) {
            fscanf(fp, "%d", &input_value);
            fclose(fp);
        }
    }
    
    /* Complex branching structure */
    if (input_value <= 0) {
        printf("Path A: Negative or zero input\n");
        func1(10);
        cold_function();
    } else if (input_value <= 100) {
        printf("Path B: Small positive input\n");
        func2(input_value);
        
        /* Nested if-else */
        if (input_value % 2 == 0) {
            printf("Even number\n");
            global_counter += 2;
        } else {
            printf("Odd number\n");
            global_counter += 1;
        }
    } else if (input_value <= 1000) {
        printf("Path C: Medium input\n");
        func3(input_value / 10);
        
        /* Switch statement for more branches */
        switch (input_value % 4) {
            case 0:
                printf("Case 0\n");
                break;
            case 1:
                printf("Case 1\n");
                break;
            case 2:
                printf("Case 2\n");
                hot_function(1000);
                break;
            case 3:
                printf("Case 3\n");
                break;
            default:
                printf("Default case\n");
        }
    } else {
        printf("Path D: Large input - HOT PATH\n");
        /* Create hot blocks with high execution counts */
        hot_function(input_value);
        
        /* Loop with many iterations */
        for (int i = 0; i < input_value / 100; i++) {
            global_counter++;
            if (i % 100 == 0) {
                func1(1);
            }
        }
    }
    
    /* Final processing with conditional */
    if (global_counter > 50) {
        printf("High counter: %d\n", global_counter);
        hot_function(global_counter);
    } else {
        printf("Low counter: %d\n", global_counter);
        cold_function();
    }
    
    return 0;
}
