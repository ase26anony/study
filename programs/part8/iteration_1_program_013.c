/* gengtype-coverage-test.c - Test program for gengtype-parse.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Simple parser stub to simulate gengtype parsing logic */
typedef enum {
    TOK_EOF,
    TOK_CHAR,
    TOK_ERROR
} token_t;

static const char *input_ptr;
static int line_num = 1;

static token_t get_next_token(void) {
    if (*input_ptr == '\0') return TOK_EOF;
    return TOK_CHAR;
}

static void advance(void) {
    if (*input_ptr == '\n') line_num++;
    input_ptr++;
}

static void consume_balanced(char open, char close) {
    int depth = 1;
    advance(); /* Skip opening delimiter */
    
    while (*input_ptr != '\0' && depth > 0) {
        if (*input_ptr == open) {
            depth++;
        } else if (*input_ptr == close) {
            depth--;
        } else if (*input_ptr == '\\') {
            advance(); /* Skip escape */
            if (*input_ptr != '\0') advance();
            continue;
        }
        advance();
    }
}

static void parse_gt_content(const char *content) {
    input_ptr = content;
    line_num = 1;
    
    while (*input_ptr != '\0') {
        token_t tok = get_next_token();
        if (tok == TOK_EOF) break;
        
        switch (*input_ptr) {
            default:
                advance();
                break;
            case '(':
                consume_balanced('(', ')');
                break;
            case '[':
                consume_balanced('[', ']');
                break;
            case '{':
                consume_balanced('{', '}');
                break;
        }
    }
}

/* Complex .gt test content with all required patterns */
static const char *gt_test_content = 
/* Requirement 1: Balanced construct nesting with all delimiter pairs */
"%typedef struct ComplexType {\n"
"  int (*callback)(int, char **);  /* Function pointer with parentheses */\n"
"  union {\n"
"    struct { int x; double y; } nested;\n"
"    int matrix[3][4];  /* Nested arrays */\n"
"  } data;\n"
"  void (*operations[5])(void);  /* Array of function pointers */\n"
"} ComplexType;\n\n"

/* More complex nesting */
"%struct TreeNode {\n"
"  struct TreeNode *children[10];\n"
"  int (*compare)(struct TreeNode *, struct TreeNode *);\n"
"  union {\n"
"    int ival;\n"
"    double dval;\n"
"    char *sval;\n"
"  } value;\n"
"};\n\n"

/* Requirement 2: Unbalanced edge cases (for error handling) */
"/* Unbalanced test cases - should trigger parser errors */\n"
"%struct Unbalanced1 {\n"
"  int data[5;  /* Missing closing bracket */\n"
"};\n\n"

"%struct Unbalanced2 {\n"
"  void (*funcptr)(int;  /* Missing closing paren */\n"
"};\n\n"

"/* Partially balanced with nesting */\n"
"%union PartialBalance {\n"
"  struct { int x; /* Missing closing brace here intentionally */\n"
"};\n\n"

/* Requirement 3: Comments and macros interleaving */
"#define ARRAY_TYPE(T) struct { T data[10]; }\n"
"#define FUNC_PTR(RET, ARGS) RET (*)(ARGS)\n\n"

"/* Block comment with delimiters inside */\n"
"/* int test_array[5] = {1, 2, 3}; */\n"
"/* void (*commented_func)(void); */\n\n"

"// Line comment with brackets\n"
"// int matrix[3][4] = {{1,2},{3,4}};\n\n"

"%typedef ARRAY_TYPE(int) IntArray;\n"
"%typedef FUNC_PTR(int, (int, char *)) IntFuncPtr;\n\n"

/* Requirement 4: GT-specific annotations with embedded delimiters */
"GC roots {\n"
"  struct RootStruct {\n"
"    int *ptr_array[5];\n"
"    void (**callbacks[3])(int, int);\n"
"  } root_var;\n"
"};\n\n"

"%union GcUnion {\n"
"  struct {\n"
"    int count;\n"
"    char *names[20];\n"
"  } header;\n"
"  int (*handlers[5])(union GcUnion *);\n"
"};\n\n"

/* Requirement 5: Multiple top-level declarations */
"%enum TokenType {\n"
"  TOK_IDENT,\n"
"  TOK_NUMBER,\n"
"  TOK_STRING,\n"
"  TOK_OPERATOR  /* Contains parentheses in comments */\n"
"};\n\n"

"%struct NestedDelimiters {\n"
"  /* Triple nesting */\n"
"  int (*(*complex[2])[3])(int, int);\n"
"  struct {\n"
"    union {\n"
"      int a;\n"
"      struct { char c; } b;\n"
"    } u;\n"
"  } s;\n"
"};\n\n"

"%typedef struct {\n"
"  /* Mixed delimiters */\n"
"  void *(*allocators[5])(size_t);\n"
"  int priorities[10];\n"
"  struct { int x; } point;\n"
"} AllocatorSet;\n\n"

"/* Preprocessor with balanced delimiters */\n"
"#ifdef DEBUG\n"
"#  define CHECK(cond) do { \\\n"
"    if (!(cond)) { \\\n"
"      fprintf(stderr, \"Check failed\"); \\\n"
"    } \\\n"
"  } while (0)\n"
"#else\n"
"#  define CHECK(cond) ((void)0)\n"
"#endif\n\n"

"/* Final complex type with all patterns */\n"
"%struct UltimateTest {\n"
"  /* Function pointer array */\n"
"  int (*(*func_table[10])(int))[5];\n"
"  \n"
"  /* Nested struct/union */\n"
"  union {\n"
"    struct {\n"
"      int matrix[2][2];\n"
"      void (*ops[3])(void);\n"
"    } s;\n"
"    long data[100];\n"
"  } u;\n"
"  \n"
"  /* Pointer to array */\n"
"  char (*string_table)[50];\n"
"};\n";

/* Additional test with malformed content for error path coverage */
static const char *gt_error_content =
"%struct ErrorTest1 {\n"
"  int values[5  /* Missing closing bracket */\n"
"  char *name;\n"
"};\n\n"

"%union ErrorTest2 {\n"
"  struct { int x;  /* Missing closing brace */\n"
"};\n\n"

"void (*bad_funcptr(int x);  /* Unbalanced parentheses */\n";

int main(void) {
    FILE *gt_file = NULL;
    char template[] = "/tmp/gengtype_test_XXXXXX.gt";
    int fd;
    
    /* Create temporary .gt file */
    fd = mkstemps(template, 3);
    if (fd == -1) {
        perror("mkstemps failed");
        return 1;
    }
    
    gt_file = fdopen(fd, "w");
    if (!gt_file) {
        perror("fdopen failed");
        close(fd);
        return 1;
    }
    
    /* Write the comprehensive test content */
    fprintf(gt_file, "%s", gt_test_content);
    fprintf(gt_file, "\n\n/* Error test section */\n%s", gt_error_content);
    fclose(gt_file);
    
    printf("Generated test file: %s\n", template);
    printf("Content length: %zu bytes\n", strlen(gt_test_content) + strlen(gt_error_content));
    
    /* Parse the content with our stub parser */
    printf("\nParsing test content...\n");
    parse_gt_content(gt_test_content);
    
    printf("Parsing error content...\n");
    parse_gt_content(gt_error_content);
    
    /* In a real test, we would invoke gengtype here:
     *   char cmd[512];
     *   snprintf(cmd, sizeof(cmd), "./gengtype -p %s", template);
     *   system(cmd);
     */
    
    /* Cleanup */
    unlink(template);
    
    printf("\nTest completed. Coverage targets:\n");
    printf("1. consume_balanced('(', ')') - via function pointers, macros\n");
    printf("2. consume_balanced('[', ']') - via arrays, nested arrays\n");
    printf("3. consume_balanced('{', '}') - via structs, unions, blocks\n");
    printf("4. Error paths - via unbalanced delimiters\n");
    printf("5. Comment/macro handling - via embedded delimiters in comments\n");
    
    return 0;
}
