#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations for functions in lib.c */
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void rarely_called(void);
void varying_function(int mode);

/* Functions in main.c */
void process_data(int size, int mode) {
    int i;
    if (mode == 1) {
        /* Hot path - executed many times */
        for (i = 0; i < size * 100; i++) {
            if (i % 7 == 0) {
                printf("Processing data point %d\n", i);
            }
        }
    } else if (mode == 2) {
        /* Medium path */
        for (i = 0; i < size * 10; i++) {
            if (i % 3 == 0) {
                printf("Medium processing %d\n", i);
            }
        }
    } else {
        /* Cold path */
        printf("Minimal processing for mode %d\n", mode);
    }
}

void analyze_results(int depth) {
    int i, j;
    if (depth > 0) {
        for (i = 0; i < depth; i++) {
            for (j = 0; j < i; j++) {
                if ((i + j) % 2 == 0) {
                    printf("Analysis %d,%d\n", i, j);
                }
            }
        }
    } else {
        printf("No analysis needed\n");
    }
}

void control_flow(int value) {
    switch (value) {
        case 1:
            printf("Case 1 executed\n");
            break;
        case 2:
            printf("Case 2 executed\n");
            break;
        case 3:
            printf("Case 3 executed\n");
            break;
        default:
            printf("Default case executed\n");
            break;
    }
}

void recursive_function(int n) {
    if (n <= 0) {
        printf("Recursion base case\n");
        return;
    }
    printf("Recursive call %d\n", n);
    recursive_function(n - 1);
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    /* Vary execution based on mode to create different profiles */
    if (mode == 1) {
        /* Profile 1: Heavy execution */
        process_data(10, 1);
        hot_function(iterations);
        medium_function(50);
        analyze_results(8);
        control_flow(1);
        control_flow(2);
        recursive_function(5);
    } else if (mode == 2) {
        /* Profile 2: Medium execution */
        process_data(5, 2);
        hot_function(iterations / 2);
        cold_function();
        medium_function(20);
        analyze_results(4);
        control_flow(3);
        control_flow(4);
    } else {
        /* Profile 3: Light execution */
        process_data(2, 3);
        rarely_called();
        varying_function(mode);
        analyze_results(1);
        control_flow(5);
    }
    
    return 0;
}
