#include <stdio.h>

extern int helper(void);
extern int problematic(void);

int main() {
    printf("Result from helper: %d\n", helper());
    printf("Result from problematic: %d\n", problematic());
    return 0;
}
