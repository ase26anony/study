/* gengtype-coverage-test.c - ISO C99 compliant test generator for gengtype parser */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Test 1: Complex balanced constructs with all delimiter types */
static const char *test_balanced = 
"/* Test file for gengtype parser - Balanced constructs */\n"
"\n"
"/* Type definitions with nested parentheses */\n"
"%typedef struct Node {\n"
"  int data;\n"
"  struct Node *next;\n"
"  void (*callback)(struct Node *self, int (*comparator)(int, int));\n"
"  /* Function pointer with complex signature */\n"
"  char *(*allocator)(size_t (*size_calc)(void *), void *context);\n"
"} Node;\n"
"\n"
"/* Array types with nested brackets */\n"
"%typedef struct Matrix {\n"
"  double values[3][3][3];  /* 3D array */\n"
"  int (*operations[5])(double matrix[3][3]);  /* Array of function pointers */\n"
"} Matrix;\n"
"\n"
"/* Union with nested structures */\n"
"%union ComplexValue {\n"
"  struct { int real; int imag; } complex;\n"
"  struct { float arr[2][2]; } matrix;\n"
"  void *(*func)(int arg1, char *arg2[10]);\n"
"};\n"
"\n"
"/* GC roots with nested types */\n"
"GC roots {\n"
"  struct {\n"
"    Node *head;\n"
"    Matrix transforms[10];\n"
"    union ComplexValue values[5];\n"
"  } root_struct;\n"
"};\n"
"\n"
"/* Enum with function pointer type */\n"
"%typedef enum Operation {\n"
"  OP_ADD,\n"
"  OP_SUB,\n"
"  OP_MUL,\n"
"  OP_DIV\n"
"} Operation;\n"
"\n"
"/* Macro with balanced delimiters */\n"
"#define VECTOR_TYPE(T, N) struct { T data[N]; size_t (*size)(void); }\n"
"#define CALLBACK_TYPE(R, A) R (*)(A args[1], void *context)\n"
"\n"
"/* Complex nested type using macros */\n"
"%typedef VECTOR_TYPE(struct { int x; int y; }, 100) PointVector;\n"
"%typedef CALLBACK_TYPE(int, char *) StringCallback;\n"
"\n"
"/* Another deeply nested example */\n"
"%struct TreeNode {\n"
"  int value;\n"
"  struct TreeNode *children[4];\n"
"  void (*visitor)(struct TreeNode *node, \n"
"                  void (*action)(int, char *), \n"
"                  int depth);\n"
"  union {\n"
"    struct { int metadata[5]; } data;\n"
"    char *(*formatter)(struct TreeNode *);\n"
"  } extra;\n"
"};\n";

/* Test 2: Unbalanced delimiters for error handling */
static const char *test_unbalanced = 
"/* Test file with unbalanced delimiters */\n"
"\n"
"/* Missing closing brace */\n"
"%struct Unbalanced1 {\n"
"  int x;\n"
"  char y;\n"
"  /* No closing brace here - parser should handle */\n"
"\n"
"/* Extra closing brace */\n"
"%struct Unbalanced2 {\n"
"  int a;\n"
"};\n"
"}  /* Extra */\n"
"\n"
"/* Unbalanced parentheses in function pointer */\n"
"%typedef void (*BadFuncPtr)(int a, char b;  /* Missing ) */\n"
"\n"
"/* Unbalanced brackets */\n"
"%struct BadArray {\n"
"  int matrix[3][3;  /* Missing ] */\n"
"};\n"
"\n"
"/* Nested unbalanced */\n"
"%struct DeepUnbalanced {\n"
"  void (*func)(int arr[5][5, char *str);  /* Mixed unbalanced */\n"
"};\n";

/* Test 3: Comments and macros interleaving with delimiters */
static const char *test_comments_macros =
"/* Test: Delimiters inside comments and macros */\n"
"\n"
"/* Block comment with delimiters: { [ ( ) ] } */\n"
"/* Line comment with: void (*func)(int) */\n"
"\n"
"#define WRAP_STRUCT(T) struct Wrapped_##T { T value; void (*print)(T); }\n"
"#define ARRAY_WRAPPER(N) struct { int data[N]; size_t (*get_size)(void); }\n"
"\n"
"/* Use the macros */\n"
"%typedef WRAP_STRUCT(struct { int x; int y; }) PointWrapper;\n"
"%typedef ARRAY_WRAPPER(/* Comment inside macro arg */ 10) Array10;\n"
"\n"
"/* Multi-line macro with balanced delimiters */\n"
"#define COMPLEX_PTR(T) \\\n"
"  struct { \\\n"
"    T *ptr; \\\n"
"    void (*deleter)(T *array[10]); \\\n"
"    int (*comparator)(T a, T b); \\\n"
"  }\n"
"\n"
"/* Delimiters in disabled code */\n"
"#if 0\n"
"  /* This should be skipped */\n"
"  %struct Skipped {\n"
"    int (*bad_func)(char array[100);  /* Unbalanced in skipped code */\n"
"  };\n"
"#endif\n"
"\n"
"/* Active code with similar pattern */\n"
"%struct Active {\n"
"  int (*good_func)(char array[100]);  /* Balanced */\n"
"};\n";

/* Test 4: GT-specific annotations with embedded delimiters */
static const char *test_gt_annotations =
"/* GT-specific annotations with balanced delimiters */\n"
"\n"
"/* Struct with GC pointers */\n"
"%struct GcStruct {\n"
"  int id;\n"
"  %maybe_undef char *name;  /* GC pointer */\n"
"  struct GcStruct *next;    /* Another GC pointer */\n"
"  void (*callback)(struct GcStruct *self);\n"
"};\n"
"\n"
"/* Union with nested annotations */\n"
"%union TaggedUnion {\n"
"  %struct IntPair { int x; int y; } pair;\n"
"  %struct StringData { char *str; size_t len; } string;\n"
"  void (*handler)(%union TaggedUnion *self);\n"
"};\n"
"\n"
"/* Typedef chain with function pointers */\n"
"%typedef int (*Comparator)(const void *, const void *);\n"
"%typedef struct {\n"
"  Comparator cmp;\n"
"  void *data[10];\n"
"  void (*sort)(void *array[], Comparator);\n"
"} SortContext;\n"
"\n"
"/* GC roots with complex structure */\n"
"GC roots {\n"
"  %struct RootContainer {\n"
"    GcStruct *first;\n"
"    TaggedUnion values[5];\n"
"    SortContext *sorter;\n"
"    void (*iterate)(struct RootContainer *,\n"
"                    void (*visit)(void *));\n"
"  } roots;\n"
"};\n"
"\n"
"/* Nested in typedef */\n"
"%typedef struct Outer {\n"
"  %struct Inner { int a; int b; } inner;\n"
"  %union Choice { int i; float f; } choice;\n"
"  void (*processor)(%struct Inner *, %union Choice *);\n"
"} OuterType;\n";

/* Combined test with all patterns */
static const char *combined_test =
"/* COMBINED TEST - All patterns together */\n"
"\n"
"/* 1. Balanced nesting */\n"
"%struct MasterType {\n"
"  /* Parentheses in function pointers */\n"
"  void (*func1)(int (*callback)(int, int), char *args[10]);\n"
"  \n"
"  /* Brackets in arrays */\n"
"  int matrix[3][3][3];\n"
"  \n"
"  /* Braces in nested struct */\n"
"  struct {\n"
"    union {\n"
"      int i;\n"
"      float f;\n"
"      void (*func)(struct { int x; });\n"
"    } u;\n"
"  } nested;\n"
"};\n"
"\n"
"/* 2. With macros */\n"
"#define NESTED_PTR(T) T *(*getter)(T *array[10])\n"
"%typedef struct WithMacro {\n"
"  NESTED_PTR(struct { int id; char name[50]; });\n"
"} WithMacro;\n"
"\n"
"/* 3. In comments (should be ignored) */\n"
"/* Ignored: { [ ( ) ] } */\n"
"// Also ignored: void (*ignored)(int)\n"
"\n"
"/* 4. GT annotations */\n"
"GC roots {\n"
"  %struct RootType {\n"
"    MasterType *master;\n"
"    WithMacro *macro;\n"
"    void (*initializer)(RootType *,\n"
"                        int params[5],\n"
"                        void (*hook)(void));\n"
"  } root;\n"
"};\n"
"\n"
"/* 5. Edge case: empty balanced pairs */\n"
"%struct EmptyPairs {\n"
"  void (*empty_func)();\n"
"  int empty_array[0];\n"
"  struct {} empty_struct;\n"
"};\n"
"\n"
"/* 6. Deep nesting */\n"
"%typedef struct Level1 {\n"
"  struct Level2 {\n"
"    struct Level3 {\n"
"      int (*level3_func)(struct Level4 { int x; } *);\n"
"      char level3_array[5][5];\n"
"    } level3;\n"
"    union Level2Union {\n"
"      int a;\n"
"      float b[10];\n"
"    } u;\n"
"  } level2;\n"
"} Level1;\n";

/* Parser simulation to directly exercise the logic */
static void simulate_parser(const char *input) {
    const char *p = input;
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;
    int in_comment = 0;
    int in_line_comment = 0;
    int in_string = 0;
    int in_char = 0;
    char prev = '\0';
    
    while (*p) {
        if (!in_comment && !in_line_comment && !in_string && !in_char) {
            if (*p == '/' && p[1] == '*') {
                in_comment = 1;
                p += 2;
                continue;
            } else if (*p == '/' && p[1] == '/') {
                in_line_comment = 1;
                p += 2;
                continue;
            } else if (*p == '"' && prev != '\\') {
                in_string = 1;
            } else if (*p == '\'' && prev != '\\') {
                in_char = 1;
            } else {
                /* This simulates the switch statement logic */
                switch (*p) {
                    case '(':
                        paren_depth++;
                        /* Simulate consume_balanced('(', ')') */
                        break;
                    case ')':
                        if (paren_depth > 0) paren_depth--;
                        break;
                    case '[':
                        bracket_depth++;
                        /* Simulate consume_balanced('[', ']') */
                        break;
                    case ']':
                        if (bracket_depth > 0) bracket_depth--;
                        break;
                    case '{':
                        brace_depth++;
                        /* Simulate consume_balanced('{', '}') */
                        break;
                    case '}':
                        if (brace_depth > 0) brace_depth--;
                        break;
                    default:
                        /* Simulate advance() */
                        break;
                }
            }
        } else if (in_comment) {
            if (prev == '*' && *p == '/') {
                in_comment = 0;
            }
        } else if (in_line_comment) {
            if (*p == '\n') {
                in_line_comment = 0;
            }
        } else if (in_string) {
            if (*p == '"' && prev != '\\') {
                in_string = 0;
            }
        } else if (in_char) {
            if (*p == '\'' && prev != '\\') {
                in_char = 0;
            }
        }
        
        prev = *p;
        p++;
    }
}

/* Write test file and optionally invoke gengtype */
static int write_and_test(const char *filename, const char *content, 
                         int invoke_parser) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Error creating %s: %s\n", filename, strerror(errno));
        return -1;
    }
    
    fputs(content, f);
    fclose(f);
    
    if (invoke_parser) {
        /* In a real test environment, this would invoke gengtype */
        printf("Created test file: %s\n", filename);
        printf("To test with actual gengtype: ./gengtype -p %s\n", filename);
        
        /* Simulate the parser logic locally */
        simulate_parser(content);
    }
    
    return 0;
}

int main(void) {
    const char *temp_dir = "/tmp";
    char path[256];
    int ret = 0;
    
    printf("Generating gengtype parser coverage tests...\n");
    
    /* Test 1: Balanced constructs */
    snprintf(path, sizeof(path), "%s/test_balanced.gt", temp_dir);
    ret |= write_and_test(path, test_balanced, 1);
    
    /* Test 2: Unbalanced constructs */
    snprintf(path, sizeof(path), "%s/test_unbalanced.gt", temp_dir);
    ret |= write_and_test(path, test_unbalanced, 1);
    
    /* Test 3: Comments and macros */
    snprintf(path, sizeof(path), "%s/test_comments_macros.gt", temp_dir);
    ret |= write_and_test(path, test_comments_macros, 1);
    
    /* Test 4: GT annotations */
    snprintf(path, sizeof(path), "%s/test_gt_annotations.gt", temp_dir);
    ret |= write_and_test(path, test_gt_annotations, 1);
    
    /* Combined test */
    snprintf(path, sizeof(path), "%s/test_combined.gt", temp_dir);
    ret |= write_and_test(path, combined_test, 1);
    
    if (ret == 0) {
        printf("\nAll test files generated successfully.\n");
        printf("To build and run with coverage instrumentation:\n");
        printf("1. g++ -O0 -fprofile-arcs -ftest-coverage -I. -I../../include \\\n");
        printf("   -o gengtype-instr gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
        printf("2. ./gengtype-instr -p /tmp/test_combined.gt\n");
        printf("3. gcov gengtype-parse.cc\n");
    }
    
    return ret;
}
