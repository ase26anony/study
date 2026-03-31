#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_gcc_cleanup.c"
#define TEMP_OBJ_FILE "test_gcc_cleanup.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *f = fopen(TEMP_SOURCE_FILE, "w");
    if (!f) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(f, "/* Test file for GCC cleanup coverage */\n");
    fprintf(f, "#include <stdio.h>\n\n");
    fprintf(f, "int main(void) {\n");
    fprintf(f, "    printf(\"Hello from test program\\n\");\n");
    fprintf(f, "    return 0;\n");
    fprintf(f, "}\n");
    
    fclose(f);
    return 0;
}

/* Execute GCC with given arguments using fork/exec */
static int run_gcc(const char *description, char *const argv[]) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        execvp("gcc", argv);
        /* If execvp returns, it failed */
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            fprintf(stderr, "%s: GCC exited with status %d\n", 
                    description, WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "%s: GCC terminated by signal %d\n", 
                    description, WTERMSIG(status));
            return -1;
        }
    }
    return 0;
}

/* Build argument array for GCC */
static char **build_gcc_args(const char *arg1, ...) {
    /* Count arguments */
    int count = 1; /* Start with "gcc" */
    va_list args;
    const char *arg;
    
    va_start(args, arg1);
    arg = arg1;
    while (arg) {
        count++;
        arg = va_arg(args, const char *);
    }
    va_end(args);
    
    /* Allocate array */
    char **argv = malloc((count + 1) * sizeof(char *));
    if (!argv) return NULL;
    
    /* Fill array */
    argv[0] = "gcc";
    argv[1] = (char *)arg1;
    
    va_start(args, arg1);
    for (int i = 2; i < count; i++) {
        argv[i] = (char *)va_arg(args, const char *);
    }
    va_end(args);
    
    argv[count] = NULL;
    return argv;
}

int main(void) {
    int ret = 0;
    
    /* Create test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    fprintf(stderr, "Testing GCC cleanup logic with various options...\n\n");
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    fprintf(stderr, "Test 1: -dumpdir with -save-temps\n");
    char **argv1 = build_gcc_args(
        "-dumpdir", "./dumpdir_test/",
        "-save-temps",
        "-c", TEMP_SOURCE_FILE,
        "-o", TEMP_OBJ_FILE,
        NULL
    );
    if (argv1) {
        run_gcc("Test 1", argv1);
        free(argv1);
    }
    
    /* Test 2: dumpbase and dumpbase-ext without dumpdir */
    fprintf(stderr, "\nTest 2: -dumpbase and -dumpbase-ext\n");
    char **argv2 = build_gcc_args(
        "-dumpbase", "myprog",
        "-dumpbase-ext", ".c",
        "-save-temps=cwd",
        "-c", TEMP_SOURCE_FILE,
        NULL
    );
    if (argv2) {
        run_gcc("Test 2", argv2);
        free(argv2);
    }
    
    /* Test 3: All dump options combined */
    fprintf(stderr, "\nTest 3: All dump options with custom output\n");
    char **argv3 = build_gcc_args(
        "-dumpdir", "./full_dump/",
        "-dumpbase", "full_test",
        "-dumpbase-ext", ".c",
        "-save-temps",
        "-o", "custom_output.o",
        "-c", TEMP_SOURCE_FILE,
        NULL
    );
    if (argv3) {
        run_gcc("Test 3", argv3);
        free(argv3);
    }
    
    /* Test 4: Different save-temps variant */
    fprintf(stderr, "\nTest 4: save-temps=obj with dumpdir\n");
    char **argv4 = build_gcc_args(
        "-dumpdir", "./obj_dump/",
        "-save-temps=obj",
        "-c", TEMP_SOURCE_FILE,
        "-o", "obj_temp.o",
        NULL
    );
    if (argv4) {
        run_gcc("Test 4", argv4);
        free(argv4);
    }
    
    /* Test 5: Minimal options to trigger basic cleanup */
    fprintf(stderr, "\nTest 5: Minimal compilation\n");
    char **argv5 = build_gcc_args(
        "-c", TEMP_SOURCE_FILE,
        NULL
    );
    if (argv5) {
        run_gcc("Test 5", argv5);
        free(argv5);
    }
    
    /* Test 6: Invalid option to test cleanup after error */
    fprintf(stderr, "\nTest 6: Invalid option (testing cleanup after error)\n");
    char **argv6 = build_gcc_args(
        "-dumpdir", "./error_dump/",
        "-invalid-opt",
        "-c", TEMP_SOURCE_FILE,
        NULL
    );
    if (argv6) {
        run_gcc("Test 6", argv6);
        free(argv6);
    }
    
    /* Clean up temporary files */
    unlink(TEMP_SOURCE_FILE);
    unlink(TEMP_OBJ_FILE);
    unlink("custom_output.o");
    unlink("obj_temp.o");
    
    /* Also clean up any .i, .s files created by save-temps */
    unlink("test_gcc_cleanup.i");
    unlink("test_gcc_cleanup.s");
    unlink("myprog.i");
    unlink("myprog.s");
    unlink("full_test.i");
    unlink("full_test.s");
    
    fprintf(stderr, "\nAll GCC invocations completed.\n");
    
    return ret;
}
