/* test_prog.c - Main program for coverage testing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function declarations from other modules */
extern void func1(int count);
extern void func2(int count);
extern void func3(int count);
extern void hot_function(int iterations);
extern void cold_function(void);

int main(int argc, char *argv[]) {
    int input_value = 0;
    int iterations = 1000;
    
    /* Read input from command line or file */
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        /* Try to read from file */
        FILE *fp = fopen("input.txt", "r");
        if (fp) {
            fscanf(fp, "%d", &input_value);
            fclose(fp);
        }
    }
    
    printf("Running with input: %d\n", input_value);
    
    /* Complex branching logic to generate varied coverage */
    if (input_value < 0) {
        /* Path 1: Negative values */
        printf("Negative path\n");
        func1(100);
        cold_function();
    } else if (input_value == 0) {
        /* Path 2: Zero */
        printf("Zero path\n");
        func2(50);
        cold_function();
    } else if (input_value > 0 && input_value <= 100) {
        /* Path 3: Small positive */
        printf("Small positive path\n");
        func3(input_value);
        if (input_value % 2 == 0) {
            hot_function(5000);  /* Hot block */
        } else {
            cold_function();     /* Cold block */
        }
    } else if (input_value > 100 && input_value <= 1000) {
        /* Path 4: Medium positive */
        printf("Medium positive path\n");
        for (int i = 0; i < input_value; i++) {
            if (i % 3 == 0) {
                func1(1);
            } else if (i % 3 == 1) {
                func2(1);
            } else {
                func3(1);
            }
        }
        hot_function(10000);  /* Very hot block */
    } else {
        /* Path 5: Large positive */
        printf("Large positive path\n");
        hot_function(50000);  /* Extremely hot block */
        for (int i = 0; i < 100; i++) {
            func1(10);
            func2(10);
            func3(10);
        }
    }
    
    /* Additional switch statement for more branches */
    switch (input_value % 5) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            func1(5);
            break;
        case 3:
            printf("Case 3\n");
            func2(5);
            break;
        case 4:
            printf("Case 4\n");
            func3(5);
            break;
    }
    
    return 0;
}
