/* test.c - Simple test program for gcov-dump */
int main() {
    int x = 0;
    int y = 1;
    
    if (x) {
        return 1;  /* This branch won't be taken */
    }
    
    if (y) {
        return 0;  /* This branch will be taken */
    }
    
    return 2;  /* Unreachable code */
}
