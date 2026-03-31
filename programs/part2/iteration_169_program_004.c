/* test.c - Contains intentional warnings and errors for testing */
#include <stdio.h>

/* Deliberate warning: unused parameter */
int unused_function(int unused_param) {
    return 0;
}

/* Function with potential warning */
int potentially_uninitialized() {
    int x;  /* May be used uninitialized */
    if (0) {
        x = 5;
    }
    return x;  /* Warning: may be used uninitialized */
}

/* Main function with syntax error in one version */
#ifdef DELIBERATE_ERROR
int main( {  /* Missing closing parenthesis - syntax error */
    printf("This should fail to compile\n");
    return 0;
}
#else
int main() {
    printf("Test program\n");
    int result = potentially_uninitialized();
    return result;
}
#endif
