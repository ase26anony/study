#include <stdio.h>
#include <stdlib.h>

void func1(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

void func2(int a, int b) {
    for (int i = 0; i < a; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    switch (b) {
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            break;
        default:
            printf("Default case\n");
    }
}

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    
    printf("Input value: %d\n", input);
    
    func1(input);
    func2(input, input % 3);
    
    return 0;
}
