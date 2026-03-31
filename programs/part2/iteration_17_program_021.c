/* test_gcc_cleanup.c - Test program to cover driver cleanup block */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define GCC_PATH "./gcc/xgcc"  /* Adjust based on your build directory */
#define TMP_DIR "/tmp/gcc_test_cover"

static void create_temp_source(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "int main() { return 0; }\n");
    fclose(f);
}

static void run_gcc_with_flags(const char **argv, int argc) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    
    if (pid == 0) {  /* Child process */
        /* Set environment variables to affect driver state */
        setenv("GCC_EXEC_PREFIX", "/usr/local/lib/gcc/", 1);
        setenv("COMPILER_PATH", "/usr/bin:/usr/local/bin", 1);
        setenv("LIBRARY_PATH", "/usr/lib:/usr/local/lib", 1);
        
        /* Execute GCC driver */
        execv(GCC_PATH, (char * const *)argv);
        perror("execv");
        exit(1);
    } else {  /* Parent process */
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("GCC exited with status %d\n", WEXITSTATUS(status));
        }
    }
}

int main(int argc, char **argv) {
    /* Create temporary directory */
    mkdir(TMP_DIR, 0755);
    
    /* Create minimal source file */
    char src_path[256];
    snprintf(src_path, sizeof(src_path), "%s/test.c", TMP_DIR);
    create_temp_source(src_path);
    
    /* Test 1: Comprehensive compilation with many state-altering flags */
    printf("=== Test 1: Comprehensive compilation ===\n");
    const char *test1_args[] = {
        GCC_PATH,
        "-save-temps",                    /* Sets save_temps_flag */
        "-dumpdir", TMP_DIR,              /* Allocates dumpdir */
        "-dumpbase", "test_dump",         /* Allocates dumpbase */
        "-dumpbase-ext", ".ext",          /* Allocates dumpbase_ext */
        "--sysroot=/opt/mysysroot",       /* Sets target_system_root, target_system_root_changed */
        "-fuse-ld=gold",                  /* Sets use_ld */
        "-ftime-report",                  /* Sets report_times_to_file */
        "-v",                             /* Sets verbose_only_flag */
        "-mtune=cortex-a72",              /* Affects spec_machine */
        "-march=armv8-a",                 /* Further affects spec_machine */
        "-specs=/dev/null",               /* Triggers spec processing */
        "-B/tmp/dummy",                   /* Adds prefix to compiler execution path */
        "-idirafter", "/tmp/include",     /* Additional include path */
        "-L/tmp/lib",                     /* Additional library path */
        "-o", "/tmp/test_output.o",       /* Output file */
        src_path,                         /* Source file */
        NULL
    };
    run_gcc_with_flags(test1_args, sizeof(test1_args)/sizeof(test1_args[0]) - 1);
    
    /* Test 2: Help and version flags combination */
    printf("\n=== Test 2: Help and version flags ===\n");
    const char *test2_args[] = {
        GCC_PATH,
        "--help=common",                  /* Sets print_help_list */
        "--version",                      /* Sets print_version */
        "-v",                             /* verbose_only_flag */
        "-###",                           /* May set print_subprocess_help */
        src_path,
        NULL
    };
    run_gcc_with_flags(test2_args, sizeof(test2_args)/sizeof(test2_args[0]) - 1);
    
    /* Test 3: Different sysroot and dump options */
    printf("\n=== Test 3: Alternative sysroot and dump options ===\n");
    const char *test3_args[] = {
        GCC_PATH,
        "-save-temps=obj",                /* Different save_temps_flag value */
        "-dumpdir", "/var/tmp",
        "-dumpbase", "alt_dump",
        "-isysroot", "/opt/alternate_sysroot",  /* Alternative sysroot flag */
        "-fuse-ld=bfd",
        "-ftime-report",
        "-wrapper", "/usr/bin/wrapper",   /* Additional driver option */
        "-Xassembler", "-v",              /* Pass to assembler */
        "-Xlinker", "--verbose",          /* Pass to linker */
        "-o", "/tmp/test_output2.o",
        src_path,
        NULL
    };
    run_gcc_with_flags(test3_args, sizeof(test3_args)/sizeof(test3_args[0]) - 1);
    
    /* Test 4: Error case to ensure cleanup still happens */
    printf("\n=== Test 4: Error case ===\n");
    const char *test4_args[] = {
        GCC_PATH,
        "-save-temps",
        "-dumpdir", TMP_DIR,
        "-dumpbase", "error_test",
        "--sysroot=/nonexistent/sysroot",
        "-fuse-ld=invalid",               /* Should cause error but still trigger cleanup */
        "-o", "/tmp/test_output3.o",
        src_path,
        NULL
    };
    run_gcc_with_flags(test4_args, sizeof(test4_args)/sizeof(test4_args[0]) - 1);
    
    /* Cleanup temporary files */
    unlink(src_path);
    char *rm_cmd;
    asprintf(&rm_cmd, "rm -rf %s", TMP_DIR);
    system(rm_cmd);
    free(rm_cmd);
    
    printf("\nAll tests completed. Check coverage with:\n");
    printf("  gcov -b gcc.cc\n");
    
    return 0;
}
