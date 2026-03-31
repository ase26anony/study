#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

void compile_with_cleanup(const char *source, const char *dumpdir, const char *dumpbase) {
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process: compile
        execlp("gcc", "gcc", "-save-temps", 
               "-dumpdir", dumpdir,
               "-dumpbase", dumpbase,
               "-c", source,
               NULL);
        
        // If execlp fails
        fprintf(stderr, "Failed to compile %s: %s\n", source, strerror(errno));
        _exit(1);
    } else if (pid > 0) {
        // Parent process: wait and cleanup
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Successfully compiled %s\n", source);
            
            // Optional: Clean up temporary files after successful compilation
            // char cmd[256];
            // snprintf(cmd, sizeof(cmd), "rm -f %s.i %s.s", dumpbase, dumpbase);
            // system(cmd);
        } else {
            fprintf(stderr, "Compilation failed for %s\n", source);
        }
    } else {
        fprintf(stderr, "fork failed: %s\n", strerror(errno));
    }
}

int main() {
    // Check if source files exist
    if (access("test.c", F_OK) != 0) {
        fprintf(stderr, "test.c not found\n");
        return 1;
    }
    
    if (access("test2.c", F_OK) != 0) {
        fprintf(stderr, "test2.c not found\n");
        return 1;
    }
    
    // Show gcc info
    system("gcc --help 2>&1 | head -5");
    system("gcc --version");
    
    // Compile with better control
    compile_with_cleanup("test.c", "./testdump/", "mytest");
    compile_with_cleanup("test2.c", "./otherdump/", "other");
    
    // Final cleanup (optional)
    printf("\nTemporary files remain in testdump/ and otherdump/\n");
    printf("Run 'rm -rf testdump otherdump' to clean up\n");
    
    return 0;
}
