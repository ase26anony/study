/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with conditional */
    for (volatile int i = 0; i < x; ++i) {
        if (i % 2 == 0) {
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Nested loop */
        for (volatile int j = 0; j < 3; ++j) {
            result += j;
        }
    }
    
    /* Switch statement */
    switch (y) {
        case 1:
            result += 100;
            break;
        case 2:
            result += 200;
            break;
        case 3:
            result += 300;
            break;
        default:
            result += 50;
    }
    
    return result;
}

/* Function 2: Different control flow pattern */
int func2(volatile int n) {
    int sum = 0;
    volatile int i = 0;
    
    /* While loop */
    while (i < n) {
        sum += i * i;
        
        /* Conditional with else-if chain */
        if (i < 5) {
            sum += 1;
        } else if (i < 10) {
            sum += 2;
        } else {
            sum += 3;
        }
        
        i++;
    }
    
    /* Do-while loop */
    do {
        sum -= 1;
        i--;
    } while (i > 0);
    
    return sum;
}

/* Function 3: Recursive-like pattern */
int func3(volatile int depth, volatile int max) {
    static int calls = 0;
    calls++;
    
    if (depth >= max) {
        return calls;
    }
    
    int total = 0;
    for (volatile int i = 0; i < 2; ++i) {
        total += func3(depth + 1, max);
    }
    
    return total + calls;
}

/* Function 4: Hot/cold path simulation */
int func4(volatile int threshold) {
    int value = 0;
    
    /* Hot path - frequently executed */
    for (volatile int i = 0; i < 100; ++i) {
        value += i;
        
        /* Cold path - rarely executed */
        if (i > threshold) {
            value *= 2;
            /* Additional cold code */
            for (volatile int j = 0; j < 5; ++j) {
                value -= j;
            }
        }
    }
    
    return value;
}

/* Main function with different execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    volatile int iterations = 5;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    int total = 0;
    
    /* Different execution paths based on mode */
    switch (mode) {
        case 1:
            /* Path 1: Call all functions lightly */
            total += func1(3, 1);
            total += func2(4);
            total += func3(0, 2);
            total += func4(80);  /* Mostly hot path */
            break;
            
        case 2:
            /* Path 2: Different mix */
            total += func1(5, 3);
            total += func2(8);
            total += func3(0, 3);
            total += func4(50);  /* Mix of hot/cold */
            break;
            
        case 3:
            /* Path 3: Heavy execution */
            for (volatile int i = 0; i < iterations; ++i) {
                total += func1(10, 2);
                total += func2(12);
            }
            total += func4(20);  /* More cold path execution */
            break;
            
        default:
            /* Default path */
            total += func1(2, 1);
            total += func2(3);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
