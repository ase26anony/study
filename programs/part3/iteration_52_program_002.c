#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define TEMP_SOURCE_FILE "test_coverage_source.c"
#define TEMP_OBJECT_FILE "test_coverage_output.o"

/* Create a simple valid C source file */
static int create_test_source(void) {
    FILE *fp = fopen(TEMP_SOURCE_FILE, "w");
    if (!fp) {
        perror("Failed to create test source file");
        return -1;
    }
    
    fprintf(fp, "/* Test file for GCC coverage */\n");
    fprintf(fp, "#include <stdio.h>\n\n");
    fprintf(fp, "int main(void) {\n");
    fprintf(fp, "    printf(\"Hello from coverage test!\\n\");\n");
    fprintf(fp, "    return 0;\n");
    fprintf(fp, "}\n");
    
    fclose(fp);
    return 0;
}

/* Execute GCC with given arguments and wait for completion */
static int run_gcc(const char *args) {
    printf("[EXEC] gcc %s\n", args);
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }
    
    if (pid == 0) {
        /* Child process */
        char *argv[] = {"gcc", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
        char cmd_copy[1024];
        int argc = 1;  /* Start with "gcc" */
        
        /* Parse the arguments string into argv array */
        strncpy(cmd_copy, args, sizeof(cmd_copy) - 1);
        cmd_copy[sizeof(cmd_copy) - 1] = '\0';
        
        char *token = strtok(cmd_copy, " ");
        while (token != NULL && argc < 10) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;
        
        execvp("gcc", argv);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        /* Parent process */
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            return -1;
        }
        
        if (WIFEXITED(status)) {
            printf("[STATUS] Exit code: %d\n\n", WEXITSTATUS(status));
            return WEXITSTATUS(status);
        } else {
            printf("[STATUS] Process terminated abnormally\n\n");
            return -1;
        }
    }
}

int main(void) {
    int overall_status = 0;
    
    /* Create test source file */
    if (create_test_source() != 0) {
        return EXIT_FAILURE;
    }
    
    printf("=== Starting GCC coverage test ===\n\n");
    
    /* Test 1: Basic compilation with dumpdir and save-temps */
    printf("--- Test 1: -dumpdir with -save-temps ---\n");
    char cmd1[512];
    snprintf(cmd1, sizeof(cmd1),
             "-dumpdir ./dump_output/ -dumpbase myprog -dumpbase-ext .c "
             "-save-temps=cwd -c %s -o %s",
             TEMP_SOURCE_FILE, TEMP_OBJECT_FILE);
    if (run_gcc(cmd1) != 0) {
        overall_status = 1;
    }
    
    /* Test 2: dumpbase and dumpbase-ext without dumpdir */
    printf("--- Test 2: -dumpbase and -dumpbase-ext without -dumpdir ---\n");
    char cmd2[512];
    snprintf(cmd2, sizeof(cmd2),
             "-dumpbase test2 -dumpbase-ext .c -c %s -o test2.o",
             TEMP_SOURCE_FILE);
    if (run_gcc(cmd2) != 0) {
        overall_status = 1;
    }
    
    /* Test 3: All options combined */
    printf("--- Test 3: All dump options with custom output ---\n");
    char cmd3[512];
    snprintf(cmd3, sizeof(cmd3),
             "-dumpdir ./full_dump/ -dumpbase fulltest -dumpbase-ext .c "
             "-save-temps -o custom_output.o -c %s",
             TEMP_SOURCE_FILE);
    if (run_gcc(cmd3) != 0) {
        overall_status = 1;
    }
    
    /* Test 4: Different save-temps option */
    printf("--- Test 4: Different save-temps variant ---\n");
    char cmd4[512];
    snprintf(cmd4, sizeof(cmd4),
             "-dumpdir ./dump4/ -dumpbase variant -dumpbase-ext .c "
             "-save-temps=obj -c %s -o variant.o",
             TEMP_SOURCE_FILE);
    if (run_gcc(cmd4) != 0) {
        overall_status = 1;
    }
    
    /* Test 5: Minimal options to trigger basic cleanup */
    printf("--- Test 5: Minimal options ---\n");
    char cmd5[512];
    snprintf(cmd5, sizeof(cmd5),
             "-dumpbase minimal -c %s",
             TEMP_SOURCE_FILE);
    if (run_gcc(cmd5) != 0) {
        overall_status = 1;
    }
    
    /* Test 6: Error case with invalid option (should still trigger cleanup) */
    printf("--- Test 6: Error case with invalid option ---\n");
    char cmd6[512];
    snprintf(cmd6, sizeof(cmd6),
             "-dumpdir ./error_dump/ -dumpbase error -dumpbase-ext .c "
             "-invalid-opt -c %s",
             TEMP_SOURCE_FILE);
    if (run_gcc(cmd6) != 0) {
        /* Expected to fail, but cleanup should still occur */
        printf("(Expected failure with invalid option)\n\n");
    }
    
    /* Test 7: Empty dumpdir with trailing slash handling */
    printf("--- Test 7: Empty dumpdir with trailing dash ---\n");
    char cmd7[512];
    snprintf(cmd7, sizeof(cmd7),
             "-dumpdir \"\" -dumpbase emptydir -dumpbase-ext .c "
             "-save-temps -c %s -o empty.o",
             TEMP_SOURCE_FILE);
    if (run_gcc(cmd7) != 0) {
        overall_status = 1;
    }
    
    /* Cleanup temporary files */
    printf("=== Cleaning up temporary files ===\n");
    if (unlink(TEMP_SOURCE_FILE) != 0) {
        perror("Failed to remove source file");
    }
    if (unlink(TEMP_OBJECT_FILE) != 0 && errno != ENOENT) {
        perror("Failed to remove object file");
    }
    if (unlink("test2.o") != 0 && errno != ENOENT) {
        perror("Failed to remove test2.o");
    }
    if (unlink("custom_output.o") != 0 && errno != ENOENT) {
        perror("Failed to remove custom_output.o");
    }
    if (unlink("variant.o") != 0 && errno != ENOENT) {
        perror("Failed to remove variant.o");
    }
    if (unlink("empty.o") != 0 && errno != ENOENT) {
        perror("Failed to remove empty.o");
    }
    
    /* Cleanup intermediate files from save-temps */
    char *intermediate_files[] = {
        "myprog.i", "myprog.s", "myprog.o",
        "test2.i", "test2.s",
        "fulltest.i", "fulltest.s",
        "variant.i", "variant.s",
        "minimal.i", "minimal.s", "minimal.o",
        "error.i", "error.s", "error.o",
        "emptydir.i", "emptydir.s",
        NULL
    };
    
    for (int i = 0; intermediate_files[i] != NULL; i++) {
        if (unlink(intermediate_files[i]) != 0 && errno != ENOENT) {
            /* Ignore missing files */
        }
    }
    
    printf("\n=== Test completed ===\n");
    printf("Overall status: %s\n", overall_status == 0 ? "SUCCESS" : "SOME TESTS FAILED");
    
    return overall_status;
}
