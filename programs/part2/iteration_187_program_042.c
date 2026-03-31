#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Forward declarations for functions in lib.c */
void hot_function_a(int iterations);
void cold_function_b(void);
void medium_function_c(int value);
void rarely_called_d(void);
void variable_function_e(int mode);

/* Functions in main.c */
void very_hot_function(int n) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
}

void moderately_called(int limit) {
    int sum = 0;
    for (int i = 0; i < limit; i++) {
        sum += i;
        if (sum > 1000) {
            printf("Sum exceeded 1000 at i=%d\n", i);
            break;
        }
    }
    printf("Final sum: %d\n", sum);
}

void rarely_executed(void) {
    printf("This function is rarely called\n");
    /* Some branching logic */
    if (rand() % 100 > 90) {
        printf("Lucky branch!\n");
    }
}

void function_with_switch(int mode) {
    switch (mode) {
        case 1:
            printf("Mode 1: High frequency\n");
            break;
        case 2:
            printf("Mode 2: Medium frequency\n");
            break;
        case 3:
            printf("Mode 3: Low frequency\n");
            break;
        default:
            printf("Mode %d: Default case\n", mode);
            break;
    }
}

void recursive_function(int depth) {
    if (depth <= 0) {
        printf("Recursion base case\n");
        return;
    }
    printf("Depth: %d\n", depth);
    recursive_function(depth - 1);
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 1000;
    
    srand(time(NULL));
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    /* Vary execution based on mode to create different profiles */
    switch (mode) {
        case 1:
            /* Mode 1: Focus on hot functions */
            very_hot_function(iterations * 10);
            hot_function_a(iterations * 5);
            moderately_called(iterations / 2);
            break;
            
        case 2:
            /* Mode 2: More balanced execution */
            very_hot_function(iterations);
            hot_function_a(iterations);
            medium_function_c(iterations);
            moderately_called(iterations);
            if (iterations % 7 == 0) {
                rarely_called_d();
            }
            break;
            
        case 3:
            /* Mode 3: More cold paths */
            very_hot_function(iterations / 10);
            cold_function_b();
            rarely_executed();
            variable_function_e(mode);
            recursive_function(5);
            break;
            
        default:
            /* Default: Mix of everything */
            very_hot_function(iterations);
            hot_function_a(iterations / 2);
            cold_function_b();
            medium_function_c(iterations * 2);
            rarely_called_d();
            moderately_called(iterations);
            function_with_switch(mode % 4);
            break;
    }
    
    /* Always call some functions */
    function_with_switch(mode);
    
    return 0;
}
