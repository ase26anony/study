/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int status;
    
    /* Create test source files first */
    FILE *fp1 = fopen("test.c", "w");
    if (fp1) {
        fprintf(fp1, "int foo(void) { return 42; }\n");
        fclose(fp1);
    }
    
    FILE *fp2 = fopen("test2.c", "w");
    if (fp2) {
        fprintf(fp2, "int bar(void) { return 43; }\n");
        fclose(fp2);
    }
    
    printf("=== GCC Help (first 5 lines) ===\n");
    status = system("gcc --help 2>&1 | head -5");
    if (status == -1) {
        perror("system failed");
    }
    
    printf("\n=== GCC Version ===\n");
    status = system("gcc --version");
    if (status == -1) {
        perror("system failed");
    }
    
    printf("\n=== Compiling test.c with verbose output ===\n");
    status = system("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    if (status == -1) {
        perror("system failed");
    }
    
    printf("\n=== Compiling test2.c ===\n");
    status = system("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    if (status == -1) {
        perror("system failed");
    }
    
    /* Cleanup */
    remove("test.c");
    remove("test2.c");
    
    return 0;
}
