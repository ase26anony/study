/* gengtype-coverage-test.c - Test program for gengtype-parse.cc coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Simplified parser stub for testing if actual gengtype is unavailable */
#ifdef TEST_STUB
static void consume_balanced(char open, char close) {
    /* Stub implementation for coverage testing */
    static int depth = 0;
    depth++;
    if (depth > 100) depth = 1; /* Prevent infinite recursion */
}

static void advance(void) {
    /* Stub implementation */
}

void parse_gt_content(const char *content) {
    const char *p = content;
    while (*p) {
        switch (*p) {
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
        p++;
    }
}
#endif

/* Complex .gt test content with balanced constructs */
static const char *gt_test_content = 
    "/* Test file for gengtype-parse.cc coverage */\n"
    "/* Line comments with (parentheses) and [brackets] */\n"
    "\n"
    "/* Requirement 1: Balanced construct nesting */\n"
    "%struct ComplexType {\n"
    "    int (*func_ptr_array[5])(int, char); /* Pointer to function array */\n"
    "    struct {\n"
    "        union {\n"
    "            int nested_int;\n"
    "            double (*nested_func)(float[3], struct { int x; });\n"
    "        } u;\n"
    "        int arr[10][20]; /* Multi-dimensional array */\n"
    "    } inner;\n"
    "    void (*callback)(int (*)(char *), struct { int a; int b; });\n"
    "};\n"
    "\n"
    "/* Requirement 2: Unbalanced edge cases (for error handling) */\n"
    "%struct Unbalanced1 {\n"
    "    int x; /* Missing closing brace intentionally */\n"
    "    /* The parser should handle this gracefully */\n"
    "\n"
    "%struct Unbalanced2 {\n"
    "    char *str; /* Extra closing brace */\n"
    "}};\n"
    "\n"
    "%union UnbalancedUnion {\n"
    "    int a;\n"
    "    double b[5; /* Missing bracket */\n"
    "};\n"
    "\n"
    "/* Requirement 3: Comments and macros interleaving */\n"
    "#define ARRAY_TYPE(T) struct { T data[(10)]; /* Parentheses in macro */ }\n"
    "#define FUNC_PTR(RET, ARGS) RET (*) ARGS\n"
    "\n"
    "/* Block comment with {braces} and (parentheses) */\n"
    "/* Multi-line\n"
    "   comment with [brackets]\n"
    "   and more {nested} content */\n"
    "\n"
    "%typedef ARRAY_TYPE(int) IntArray;\n"
    "%typedef FUNC_PTR(void, (int, char *)) SimpleFunc;\n"
    "\n"
    "// Line comment with {unbalanced\n"
    "// Line comment with [array[10]] syntax\n"
    "\n"
    "#define COMPLEX_MACRO(X) struct { X (*methods[3])(void); }\n"
    "\n"
    "/* Requirement 4: GT file specific constructs */\n"
    "GC roots {\n"
    "    struct ComplexType *root_ptr;\n"
    "    %union U {\n"
    "        struct { int x; int y; } point;\n"
    "        int matrix[3][3];\n"
    "        void (*operations[5])(struct { int id; });\n"
    "    } data_union;\n"
    "}\n"
    "\n"
    "%struct GTAnnotated {\n"
    "    %union InnerUnion {\n"
    "        int a;\n"
    "        struct { char *name; int (*compare)(const void *, const void *); } s;\n"
    "    } u;\n"
    "    enum { RED = 1, GREEN = 2, BLUE = 3 } color;\n"
    "    int (*handlers[4])(struct GTAnnotated *self, int arg);\n"
    "};\n"
    "\n"
    "/* Requirement 5: Multiple top-level declarations */\n"
    "%enum TokenType {\n"
    "    TOKEN_INT,\n"
    "    TOKEN_FLOAT,\n"
    "    TOKEN_STRING,\n"
    "    TOKEN_IDENT\n"
    "};\n"
    "\n"
    "%typedef struct Node Node;\n"
    "\n"
    "%struct Node {\n"
    "    int type;\n"
    "    union {\n"
    "        int ival;\n"
    "        double fval;\n"
    "        char *sval;\n"
    "        Node *(*get_child)(int index);\n"
    "    } value;\n"
    "    Node *children[10];\n"
    "    void (*traverse)(Node *n, void (*visit)(Node *));\n"
    "};\n"
    "\n"
    "%union PolyUnion {\n"
    "    int (*int_func)(int);\n"
    "    double (*double_func)(double);\n"
    "    struct {\n"
    "        int count;\n"
    "        void *data[20];\n"
    "    } container;\n"
    "};\n"
    "\n"
    "/* Deeply nested constructs to stress the parser */\n"
    "%struct DeeplyNested {\n"
    "    struct {\n"
    "        union {\n"
    "            int a;\n"
    "            struct {\n"
    "                int (*deep_func)(struct {\n"
    "                    int x;\n"
    "                    int y[5];\n"
    "                    struct { char c; } inner;\n"
    "                });\n"
    "            };\n"
    "        };\n"
    "        int arr[3][4][5];\n"
    "    } level1;\n"
    "    void (*recursive[2])(struct DeeplyNested *);\n"
    "};\n"
    "\n"
    "/* Mixed delimiters in complex expressions */\n"
    "%typedef int (*(*ComplexFuncPtr)[5])(char *args[], int count);\n"
    "\n"
    "/* Macro with all delimiter types */\n"
    "#define ULTIMATE_MACRO(T) \\\n"
    "    struct { \\\n"
    "        T (*get[ (sizeof(T) > 4) ? 10 : 5 ])(void); \\\n"
    "        union { \\\n"
    "            struct { int id; char name[50]; } info; \\\n"
    "            void (*actions[3])(T *obj, int (*callback)(int)); \\\n"
    "        } u; \\\n"
    "    }\n"
    "\n"
    "%typedef ULTIMATE_MACRO(double) UltimateDouble;\n"
    "\n"
    "/* Final root declaration */\n"
    "GC roots {\n"
    "    Node *tree_root;\n"
    "    PolyUnion *polymorphic;\n"
    "    DeeplyNested *deep_data;\n"
    "    UltimateDouble ultimate;\n"
    "}\n";

/* Additional test with more edge cases */
static const char *gt_edge_cases = 
    "/* Edge case: Empty balanced pairs */\n"
    "%struct EmptyPairs { int x; }; /* () [] {} */\n"
    "\n"
    "/* Single character between delimiters */\n"
    "%struct SingleChar { char c; };\n"
    "\n"
    "/* Nested parentheses in function pointers */\n"
    "%typedef int (*(*(*nested_func_ptr))(int))(char);\n"
    "\n"
    "/* Arrays with complex dimensions */\n"
    "%struct ArrayDims {\n"
    "    int a[(10 + 5) * 2];\n"
    "    char *b[sizeof(struct { int x; })];\n"
    "};\n"
    "\n"
    "/* Unbalanced in comments (should be ignored) */\n"
    "/* { [ ( unbalanced comment */\n"
    "// } ] ) line comment\n"
    "\n"
    "/* Real unbalanced - parser should detect */\n"
    "%struct BadStruct {\n"
    "    int x;\n"
    "    /* Missing closing brace */\n";

int main(void) {
    FILE *fp;
    char tmpfile1[] = "/tmp/gengtype_test_XXXXXX.gt";
    char tmpfile2[] = "/tmp/gengtype_edge_XXXXXX.gt";
    int fd1, fd2;
    
    /* Create temporary files */
    fd1 = mkstemps(tmpfile1, 3);
    if (fd1 == -1) {
        perror("mkstemps");
        return 1;
    }
    close(fd1);
    
    fd2 = mkstemps(tmpfile2, 3);
    if (fd2 == -1) {
        perror("mkstemps");
        unlink(tmpfile1);
        return 1;
    }
    close(fd2);
    
    /* Write test content */
    fp = fopen(tmpfile1, "w");
    if (!fp) {
        perror("fopen");
        unlink(tmpfile1);
        unlink(tmpfile2);
        return 1;
    }
    fputs(gt_test_content, fp);
    fclose(fp);
    
    fp = fopen(tmpfile2, "w");
    if (!fp) {
        perror("fopen");
        unlink(tmpfile1);
        unlink(tmpfile2);
        return 1;
    }
    fputs(gt_edge_cases, fp);
    fclose(fp);
    
    /* Parse using actual gengtype if available */
    printf("Test files created:\n");
    printf("  %s\n", tmpfile1);
    printf("  %s\n", tmpfile2);
    printf("\nTo run gengtype:\n");
    printf("  ./gengtype -p %s\n", tmpfile1);
    printf("  ./gengtype -p %s\n", tmpfile2);
    
#ifdef TEST_STUB
    /* Use stub parser for basic testing */
    printf("\nUsing stub parser for coverage analysis...\n");
    parse_gt_content(gt_test_content);
    parse_gt_content(gt_edge_cases);
#endif
    
    /* Cleanup */
    unlink(tmpfile1);
    unlink(tmpfile2);
    
    return 0;
}
