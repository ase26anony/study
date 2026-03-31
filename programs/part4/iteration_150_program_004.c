/* main.c */
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/stat.h>

void create_test_files() {
    /* Create test.c */
    FILE *f1 = fopen("test.c", "w");
    if (f1) {
        fprintf(f1, "int foo(void) { return 42; }\n");
        fclose(f1);
    }
    
    /* Create test2.c */
    FILE *f2 = fopen("test2.c", "w");
    if (f2) {
        fprintf(f2, "int bar(void) { return 43; }\n");
        fclose(f2);
    }
}

void create_directories() {
    mkdir("./testdump", 0755);
    mkdir("./otherdump", 0755);
}

void cleanup() {
    /* Remove generated files */
    remove("test.c");
    remove("test2.c");
    remove("mytest.i");
    remove("mytest.s");
    remove("mytest.o");
    remove("other.o");
    
    /* Remove dump directories if empty */
    rmdir("./testdump");
    rmdir("./otherdump");
}

int main() {
    int status;
    
    /* Create necessary directories */
    create_directories();
    
    /* Create test files */
    create_test_files();
    
    printf("=== GCC Help (first 5 lines) ===\n");
    status = system("gcc --help 2>&1 | head -5");
    
    printf("\n=== GCC Version ===\n");
    status = system("gcc --version");
    
    printf("\n=== Compiling test.c with save-temps ===\n");
    status = system("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    
    printf("\n=== Compiling test2.c ===\n");
    status = system("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    
    /* Cleanup */
    cleanup();
    
    return 0;
}
