/* main.c */
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/stat.h>
#include <ftw.h>

/* Function to remove directories recursively */
static int remove_callback(const char *fpath, const struct stat *sb, 
                          int typeflag, struct FTW *ftwbuf) {
    int rv = remove(fpath);
    if (rv) perror(fpath);
    return rv;
}

int rmrf(const char *path) {
    return nftw(path, remove_callback, 64, FTW_DEPTH | FTW_PHYS);
}

int main() {
    int status;
    
    /* Create test files */
    FILE *fp = fopen("test.c", "w");
    if (fp) {
        fprintf(fp, "int foo(void) { return 42; }\n");
        fclose(fp);
    }
    
    fp = fopen("test2.c", "w");
    if (fp) {
        fprintf(fp, "int bar(void) { return 43; }\n");
        fclose(fp);
    }
    
    /* Create dump directories */
    mkdir("./testdump", 0755);
    mkdir("./otherdump", 0755);
    
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
    
    /* Cleanup temporary files */
    printf("\n=== Cleaning up ===\n");
    remove("test.c");
    remove("test2.c");
    remove("test.i");
    remove("test.s");
    remove("test.o");
    remove("test2.o");
    rmrf("./testdump");
    rmrf("./otherdump");
    
    return 0;
}
