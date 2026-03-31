/* gengtype-coverage-test.c - Comprehensive test for gengtype parser coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Parser simulation for coverage testing */
void simulate_parser(const char *input) {
    const char *p = input;
    int paren_depth = 0, bracket_depth = 0, brace_depth = 0;
    int in_comment = 0, in_line_comment = 0, in_string = 0, in_char = 0;
    char prev = 0;
    
    while (*p) {
        if (!in_comment && !in_line_comment && !in_string && !in_char) {
            switch (*p) {
                case '(':
                    paren_depth++;
                    printf("Consuming '(' at depth %d\n", paren_depth);
                    break;
                case ')':
                    if (paren_depth > 0) paren_depth--;
                    printf("Consuming ')' at depth %d\n", paren_depth);
                    break;
                case '[':
                    bracket_depth++;
                    printf("Consuming '[' at depth %d\n", bracket_depth);
                    break;
                case ']':
                    if (bracket_depth > 0) bracket_depth--;
                    printf("Consuming ']' at depth %d\n", bracket_depth);
                    break;
                case '{':
                    brace_depth++;
                    printf("Consuming '{' at depth %d\n", brace_depth);
                    break;
                case '}':
                    if (brace_depth > 0) brace_depth--;
                    printf("Consuming '}' at depth %d\n", brace_depth);
                    break;
                case '/':
                    if (*(p + 1) == '*') {
                        in_comment = 1;
                        p++;
                    } else if (*(p + 1) == '/') {
                        in_line_comment = 1;
                        p++;
                    }
                    break;
                case '"':
                    in_string = !in_string;
                    break;
                case '\'':
                    in_char = !in_char;
                    break;
                default:
                    /* Advance without special handling */
                    break;
            }
        } else {
            /* Handle comment/string termination */
            if (in_comment && prev == '*' && *p == '/') {
                in_comment = 0;
            } else if (in_line_comment && *p == '\n') {
                in_line_comment = 0;
            } else if (in_string && *p == '"' && prev != '\\') {
                in_string = 0;
            } else if (in_char && *p == '\'' && prev != '\\') {
                in_char = 0;
            }
        }
        prev = *p;
        p++;
    }
    
    printf("Final depths: parens=%d, brackets=%d, braces=%d\n", 
           paren_depth, bracket_depth, brace_depth);
}

/* Generate complex .gt test file content */
const char *generate_gt_test_content(void) {
    static char buffer[16384];
    char *p = buffer;
    
    /* Requirement 1: Balanced construct nesting with all delimiter types */
    p += sprintf(p, "/* Test file for gengtype parser coverage */\n\n");
    
    /* Complex type with all balanced delimiters */
    p += sprintf(p, "%%typedef struct ComplexType {\n");
    p += sprintf(p, "  int (*compare)(const void *, const void *);  /* Function pointer */\n");
    p += sprintf(p, "  union {\n");
    p += sprintf(p, "    struct { int x; int y; } point;\n");
    p += sprintf(p, "    int array[3][2];\n");
    p += sprintf(p, "  } data;\n");
    p += sprintf(p, "  void (*operations[5])(int, float);  /* Array of function pointers */\n");
    p += sprintf(p, "} ComplexType;\n\n");
    
    /* Nested structures with multiple delimiter types */
    p += sprintf(p, "%%struct TreeNode {\n");
    p += sprintf(p, "  struct TreeNode *children[10];\n");
    p += sprintf(p, "  int (*visit)(struct TreeNode *);\n");
    p += sprintf(p, "  union {\n");
    p += sprintf(p, "    int ival;\n");
    p += sprintf(p, "    float fval;\n");
    p += sprintf(p, "    char *sval;\n");
    p += sprintf(p, "  } value;\n");
    p += sprintf(p, "};\n\n");
    
    /* Requirement 2: Unbalanced edge cases (commented out for valid parsing) */
    p += sprintf(p, "/* UNBALANCED TEST CASES (should trigger errors):\n");
    p += sprintf(p, "%%struct Unbalanced1 {\n");
    p += sprintf(p, "  int x;\n");
    p += sprintf(p, "  /* Missing closing brace here */\n");
    p += sprintf(p, "\n");
    p += sprintf(p, "%%struct Unbalanced2 {\n");
    p += sprintf(p, "  int arr[10;  /* Missing closing bracket */\n");
    p += sprintf(p, "};\n");
    p += sprintf(p, "\n");
    p += sprintf(p, "%%struct Unbalanced3 {\n");
    p += sprintf(p, "  void (*func)(int;  /* Missing closing paren */\n");
    p += sprintf(p, "};\n");
    p += sprintf(p, "*/\n\n");
    
    /* Requirement 3: Comments and macros interleaving with delimiters */
    p += sprintf(p, "#define ARRAY_TYPE(T) struct { T data[10]; }\n");
    p += sprintf(p, "#define FUNC_PTR(RET, ARGS) RET (*) ARGS\n");
    p += sprintf(p, "\n");
    
    p += sprintf(p, "%%typedef ARRAY_TYPE(int) IntArray;\n");
    p += sprintf(p, "%%typedef FUNC_PTR(int, (int, char *)) StringProcessor;\n");
    p += sprintf(p, "\n");
    
    /* Delimiters inside comments */
    p += sprintf(p, "/* This comment contains various delimiters: { [ ( ) ] } */\n");
    p += sprintf(p, "%%struct CommentTest {\n");
    p += sprintf(p, "  int value;  // Line comment with [brackets] and {braces}\n");
    p += sprintf(p, "  /* Nested comment /* with (*more*) delimiters */ */\n");
    p += sprintf(p, "};\n\n");
    
    /* Requirement 4: GT-specific annotations with embedded delimiters */
    p += sprintf(p, "GC roots {\n");
    p += sprintf(p, "  struct RootStruct {\n");
    p += sprintf(p, "    int (*handlers[5])(void);\n");
    p += sprintf(p, "    union {\n");
    p += sprintf(p, "      struct { int a; int b; } pair;\n");
    p += sprintf(p, "      int list[10];\n");
    p += sprintf(p, "    } data;\n");
    p += sprintf(p, "  } root;\n");
    p += sprintf(p, "}\n\n");
    
    p += sprintf(p, "%%union TaggedUnion {\n");
    p += sprintf(p, "  struct { int type; void *data; } header;\n");
    p += sprintf(p, "  int (*operations[3])(struct TaggedUnion *);\n");
    p += sprintf(p, "  char buffer[256];\n");
    p += sprintf(p, "};\n\n");
    
    /* Requirement 5: Multiple top-level declarations */
    p += sprintf(p, "/* Enum with complex initializers */\n");
    p += sprintf(p, "%%enum ErrorCodes {\n");
    p += sprintf(p, "  ERR_NONE = 0,\n");
    p += sprintf(p, "  ERR_PARSE = (1 << 0),\n");
    p += sprintf(p, "  ERR_MEM = (1 << 1),\n");
    p += sprintf(p, "  ERR_IO = (1 << 2)\n");
    p += sprintf(p, "};\n\n");
    
    p += sprintf(p, "/* Another struct with deeply nested constructs */\n");
    p += sprintf(p, "%%struct DeepNest {\n");
    p += sprintf(p, "  struct {\n");
    p += sprintf(p, "    union {\n");
    p += sprintf(p, "      int (*func1)(int[10]);\n");
    p += sprintf(p, "      void (*func2)(struct { int x; });\n");
    p += sprintf(p, "    } callbacks;\n");
    p += sprintf(p, "    int matrix[3][4][5];\n");
    p += sprintf(p, "  } inner;\n");
    p += sprintf(p, "};\n\n");
    
    /* Pointer-to-pointer-to-function syntax */
    p += sprintf(p, "%%typedef int (*(*ComplexFuncPtr)[5])(float, double);\n\n");
    
    /* Array of structs containing function pointers */
    p += sprintf(p, "%%struct CallbackManager {\n");
    p += sprintf(p, "  struct {\n");
    p += sprintf(p, "    int id;\n");
    p += sprintf(p, "    void (*callback)(int, void *);\n");
    p += sprintf(p, "  } handlers[8];\n");
    p += sprintf(p, "};\n\n");
    
    /* Final test with all delimiter types mixed */
    p += sprintf(p, "/* Ultimate test mixing all delimiter types */\n");
    p += sprintf(p, "%%struct UltimateTest {\n");
    p += sprintf(p, "  int (*(*func_table[3])[2])(char *);  /* Complex declaration */\n");
    p += sprintf(p, "  union {\n");
    p += sprintf(p, "    struct { int (*op)(int[5]); } ops;\n");
    p += sprintf(p, "    void (*alt_ops[4])(struct { int x; });\n");
    p += sprintf(p, "  } u;\n");
    p += sprintf(p, "  char data[100];\n");
    p += sprintf(p, "};\n");
    
    return buffer;
}

/* Generate unbalanced test content */
const char *generate_unbalanced_test_content(void) {
    static char buffer[8192];
    char *p = buffer;
    
    p += sprintf(p, "/* Test file with unbalanced delimiters */\n\n");
    
    /* Missing closing brace */
    p += sprintf(p, "%%struct MissingBrace {\n");
    p += sprintf(p, "  int x;\n");
    p += sprintf(p, "  int y;\n");
    /* Intentionally no closing brace */
    
    /* Missing closing bracket */
    p += sprintf(p, "\n%%struct MissingBracket {\n");
    p += sprintf(p, "  int arr[10;  /* Missing ] */\n");
    p += sprintf(p, "};\n");
    
    /* Missing closing parenthesis */
    p += sprintf(p, "\n%%struct MissingParen {\n");
    p += sprintf(p, "  void (*func)(int;  /* Missing ) */\n");
    p += sprintf(p, "};\n");
    
    /* Mismatched delimiters */
    p += sprintf(p, "\n%%struct Mismatched {\n");
    p += sprintf(p, "  int a);  /* Extra ) */\n");
    p += sprintf(p, "  char b[5};  /* Wrong delimiter */\n");
    p += sprintf(p, "};\n");
    
    return buffer;
}

int main(void) {
    FILE *fp;
    char temp_filename[] = "/tmp/gengtype_test_XXXXXX.gt";
    char unbalanced_filename[] = "/tmp/gengtype_unbalanced_XXXXXX.gt";
    int fd;
    
    printf("=== GCC gengtype Parser Coverage Test ===\n\n");
    
    /* Create temporary file for balanced test */
    fd = mkstemps(temp_filename, 3);
    if (fd == -1) {
        perror("mkstemps");
        return 1;
    }
    close(fd);
    
    fp = fopen(temp_filename, "w");
    if (!fp) {
        perror("fopen");
        return 1;
    }
    
    const char *content = generate_gt_test_content();
    fprintf(fp, "%s", content);
    fclose(fp);
    
    printf("Generated balanced test file: %s\n", temp_filename);
    printf("File size: %zu bytes\n\n", strlen(content));
    
    /* Create temporary file for unbalanced test */
    fd = mkstemps(unbalanced_filename, 3);
    if (fd == -1) {
        perror("mkstemps");
        unlink(temp_filename);
        return 1;
    }
    close(fd);
    
    fp = fopen(unbalanced_filename, "w");
    if (!fp) {
        perror("fopen");
        unlink(temp_filename);
        return 1;
    }
    
    const char *unbalanced_content = generate_unbalanced_test_content();
    fprintf(fp, "%s", unbalanced_content);
    fclose(fp);
    
    printf("Generated unbalanced test file: %s\n", unbalanced_filename);
    printf("File size: %zu bytes\n\n", strlen(unbalanced_content));
    
    /* Simulate parser on balanced content */
    printf("=== Simulating Parser on Balanced Content ===\n");
    simulate_parser(content);
    
    printf("\n=== Simulating Parser on Unbalanced Content ===\n");
    simulate_parser(unbalanced_content);
    
    /* Instructions for actual gengtype execution */
    printf("\n=== To Run Actual gengtype ===\n");
    printf("1. Build gengtype with coverage instrumentation:\n");
    printf("   g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include \\\n");
    printf("       -o gengtype-instr gengtype-parse.cc gengtype.cc gengtype-state.cc\n\n");
    
    printf("2. Run gengtype on the test files:\n");
    printf("   ./gengtype-instr -p %s\n", temp_filename);
    printf("   ./gengtype-instr -p %s  (should show errors)\n", unbalanced_filename);
    printf("\n");
    
    printf("3. Generate coverage report:\n");
    printf("   lcov --capture --directory . --output-file coverage.info\n");
    printf("   genhtml coverage.info --output-directory coverage_report\n");
    
    /* Cleanup */
    printf("\n=== Test Files Created ===\n");
    printf("Balanced:   %s\n", temp_filename);
    printf("Unbalanced: %s\n", unbalanced_filename);
    printf("\nThese files can be used directly with gengtype for coverage analysis.\n");
    
    return 0;
}
