/* main.c */
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

/* test.c content */
const char *test_c_content = 
"int foo(void) { return 42; }\n";

/* test2.c content */
const char *test2_c_content = 
"int bar(void) { return 43; }\n";

int create_test_files(void) {
    FILE *fp;
    
    /* Create test.c */
    fp = fopen("test.c", "w");
    if (!fp) {
        perror("Failed to create test.c");
        return -1;
    }
    fprintf(fp, "%s", test_c_content);
    fclose(fp);
    
    /* Create test2.c */
    fp = fopen("test2.c", "w");
    if (!fp) {
        perror("Failed to create test2.c");
        return -1;
    }
    fprintf(fp, "%s", test2_c_content);
    fclose(fp);
    
    return 0;
}

void cleanup_files(void) {
    /* Remove generated files */
    remove("test.c");
    remove("test2.c");
    remove("mytest.i");
    remove("mytest.s");
    remove("mytest.o");
    remove("other.o");
    
    /* Try to remove dump directories (they might be empty) */
    rmdir("./testdump");
    rmdir("./otherdump");
}

int main() {
    int status;
    
    /* Create test files first */
    if (create_test_files() != 0) {
        return EXIT_FAILURE;
    }
    
    /* Invoke gcc with help flag */
    printf("=== GCC Help (first 5 lines) ===\n");
    status = system("gcc --help 2>&1 | head -5");
    if (status == -1) {
        perror("system() failed");
    }
    
    /* Invoke gcc with version flag */
    printf("\n=== GCC Version ===\n");
    status = system("gcc --version");
    if (status == -1) {
        perror("system() failed");
    }
    
    /* Compile a simple file with save-temps and dumpdir flags */
    printf("\n=== Compiling test.c with verbose output (last 10 lines) ===\n");
    status = system("gcc -save-temps -dumpdir ./testdump -dumpbase mytest -c test.c -v 2>&1 | tail -10");
    if (status == -1) {
        perror("system() failed");
    }
    
    /* Compile another file with different dumpdir */
    printf("\n=== Compiling test2.c ===\n");
    status = system("gcc -dumpdir ./otherdump -dumpbase other -c test2.c 2>&1");
    if (status == -1) {
        perror("system() failed");
    }
    
    /* Clean up generated files */
    cleanup_files();
    
    return 0;
}
