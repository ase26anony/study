#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Declarations for functions in lib.c
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void rarely_called(void);
void varying_function(int mode);
void branchy_function(int value);
void loop_function(int iterations);
void mixed_function(int seed);

// Local functions
static void local_hot(int n) {
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
}

static void local_cold(void) {
    printf("This is rarely executed\n");
}

static void local_medium(int x) {
    int sum = 0;
    for (int i = 0; i < x; i++) {
        sum += i * i;
    }
    printf("Sum of squares up to %d: %d\n", x, sum);
}

void execute_mode_1(void) {
    printf("=== MODE 1: High execution frequency ===\n");
    
    // Hot paths
    hot_function(1000);
    local_hot(500);
    loop_function(800);
    
    // Medium paths
    medium_function(100);
    local_medium(50);
    
    // Rare paths (executed once)
    cold_function();
    local_cold();
    rarely_called();
    
    // Branch coverage
    branchy_function(10);
    branchy_function(25);
    branchy_function(50);
    
    mixed_function(1);
}

void execute_mode_2(void) {
    printf("=== MODE 2: Different execution pattern ===\n");
    
    // Different hot paths
    hot_function(200);  // Less iterations
    local_hot(100);
    loop_function(300);
    
    // More medium paths
    medium_function(200);
    local_medium(100);
    
    // Execute rare paths more often
    cold_function();
    cold_function();  // Twice!
    local_cold();
    
    // Different branch coverage
    branchy_function(5);
    branchy_function(75);
    branchy_function(99);
    
    mixed_function(2);
    varying_function(1);
    varying_function(2);
}

void execute_mode_3(void) {
    printf("=== MODE 3: Balanced execution ===\n");
    
    // Balanced execution
    hot_function(500);
    local_hot(250);
    loop_function(400);
    
    medium_function(150);
    local_medium(75);
    
    // Execute all functions at least once
    cold_function();
    local_cold();
    rarely_called();
    varying_function(0);
    varying_function(1);
    varying_function(2);
    
    branchy_function(33);
    mixed_function(3);
}

int main(int argc, char *argv[]) {
    int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode %d\n", mode);
    
    switch (mode) {
        case 1:
            execute_mode_1();
            break;
        case 2:
            execute_mode_2();
            break;
        case 3:
            execute_mode_3();
            break;
        default:
            printf("Unknown mode %d, using mode 1\n", mode);
            execute_mode_1();
            break;
    }
    
    return 0;
}
