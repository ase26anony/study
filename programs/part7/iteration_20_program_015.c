/* test.c - Simple test program for gcov-dump */
int main() {
    int x = 0;
    int y = 1;
    
    if (x) {
        return 1;
    }
    
    if (y) {
        return 0;
    }
    
    return 2;
}
