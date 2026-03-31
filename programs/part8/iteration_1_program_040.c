/* gengtype_coverage_test.c - ISO C99 compliant test generator for gengtype parser */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* Test 1: Complex balanced constructs with all delimiter types */
static const char *test1_content =
    "/* Test file for gengtype parser coverage */\n"
    "\n"
    "/* Nested parentheses in function pointers */\n"
    "%typedef int (*complex_func_t)(int (*callback)(int, char**), void*);\n"
    "\n"
    "/* Multiple levels of parentheses nesting */\n"
    "%typedef void (*(**signal_handler_ptr)(int signo))(int);\n"
    "\n"
    "/* Arrays with complex dimensions */\n"
    "%struct Matrix {\n"
    "    double data[10][20][30];  /* 3D array */\n"
    "    int (*row_ops[5])(double matrix[][20], int size);\n"
    "};\n"
    "\n"
    "/* Union with nested structures */\n"
    "%union DataValue {\n"
    "    struct {  /* Anonymous struct */\n"
    "        int type;\n"
    "        union {  /* Nested union */\n"
    "            int ival;\n"
    "            double dval;\n"
    "            char* sval;\n"
    "        } u;\n"
    "    } tagged;\n"
    "    void* ptr_array[4][2];  /* 2D pointer array */\n"
    "};\n"
    "\n"
    "/* GC roots with complex types */\n"
    "GC roots {\n"
    "    struct Matrix* global_matrix;\n"
    "    %union DataValue* config_values[10];\n"
    "    int (*comparators[3])(const void*, const void*);\n"
    "}\n";

/* Test 2: Edge cases with unbalanced delimiters */
static const char *test2_content =
    "/* Test unbalanced constructs - should trigger error handling */\n"
    "\n"
    "/* Missing closing brace */\n"
    "%struct Unbalanced1 {\n"
    "    int x;\n"
    "    char y;\n"
    "    /* No closing brace here */\n"
    "\n"
    "/* Missing closing parenthesis in function pointer */\n"
    "%typedef void (*bad_funcptr(int x, char y);\n"
    "\n"
    "/* Missing closing bracket in array */\n"
    "int bad_array[10[5];  /* Nested without proper closing */\n"
    "\n"
    "/* But then continue with valid code to test recovery */\n"
    "%struct RecoveryTest {\n"
    "    int valid_field;\n"
    "    char* valid_ptr;\n"
    "};\n";

/* Test 3: Comments and macros interleaving with delimiters */
static const char *test3_content =
    "/* Test with comments containing delimiter-like characters */\n"
    "\n"
    "/* Comment with fake delimiters: { [ ( ) ] } */\n"
    "#define ARRAY_TYPE(T) struct { T data[10]; }\n"
    "#define FUNC_PTR(RET, ARGS) RET (*)(ARGS)\n"
    "\n"
    "// Line comment with { nested [ brackets ( inside ) ] }\n"
    "%typedef ARRAY_TYPE(int) IntArray;\n"
    "\n"
    "/* Multi-line comment with\n"
    "   { complex [ nesting ( patterns ) ] }\n"
    "   across lines */\n"
    "%struct CommentTest {\n"
    "    // Line comment inside struct\n"
    "    int field1;  /* with block comment { containing } delimiters */\n"
    "    \n"
    "    /* Nested comment /* inside */ another */\n"
    "    char* field2[5];  /* Array declaration */\n"
    "};\n"
    "\n"
    "#define COMPLEX_MACRO(X) \\\n"
    "    struct { \\\n"
    "        X (*processor)(X input[10]); \\\n"
    "        int flags; \\\n"
    "    }\n"
    "\n"
    "%typedef COMPLEX_MACRO(double) DoubleProcessor;\n";

/* Test 4: GT-specific annotations with embedded delimiters */
static const char *test4_content =
    "/* GT-specific constructs with balanced delimiters */\n"
    "\n"
    "%union GT_Union {\n"
    "    struct {\n"
    "        int tag;\n"
    "        union {\n"
    "            int ival;\n"
    "            double dval[2][3];\n"
    "            struct {\n"
    "                char* name;\n"
    "                void* data;\n"
    "            } record;\n"
    "        } value;\n"
    "    } tagged;\n"
    "    void* ptr;\n"
    "};\n"
    "\n"
    "%struct GT_Struct {\n"
    "    %union GT_Union* union_ptr;\n"
    "    int (*methods[3])(%struct GT_Struct* self, int arg);\n"
    "    struct {\n"
    "        int x, y;\n"
    "    } point;\n"
    "};\n"
    "\n"
    "/* GC roots with GT types */\n"
    "GC roots {\n"
    "    %union GT_Union root_union;\n"
    "    %struct GT_Struct* struct_array[5];\n"
    "    void (*callbacks[2])(%union GT_Union*);\n"
    "}\n"
    "\n"
    "%typedef %struct GT_Struct* GT_StructPtr;\n"
    "%typedef int (*GT_Callback)(%union GT_Union*, %struct GT_Struct*);\n";

/* Test 5: Maximum nesting complexity */
static const char *test5_content =
    "/* Maximum nesting test */\n"
    "\n"
    "%typedef int (*(*(*nested_funcptr[2])())[3])(int, char**);\n"
    "\n"
    "%struct UltimateNest {\n"
    "    struct {\n"
    "        union {\n"
    "            int a;\n"
    "            struct {\n"
    "                double b[2][2];\n"
    "                char* c;\n"
    "            } inner;\n"
    "        } u;\n"
    "    } s;\n"
    "    \n"
    "    void* (*(*func_table[4])(int))[2];\n"
    "    \n"
    "    %union {\n"
    "        int x;\n"
    "        struct {\n"
    "            char y[10];\n"
    "            void* z[5];\n"
    "        } data;\n"
    "    } anonymous_union;\n"
    "};\n"
    "\n"
    "/* Mixed delimiters in single declaration */\n"
    "%typedef struct {\n"
    "    int (*compare)(const void* a[10], const void* b[10]);\n"
    "    void (*sort)(void* array[], int size,\n"
    "                 int (*cmp)(const void*, const void*));\n"
    "    struct {\n"
    "        int min, max;\n"
    "    } bounds;\n"
    "} SortOperations;\n";

/* Combined test with all patterns */
static const char *combined_test_content =
    "/* COMBINED TEST - All patterns together */\n"
    "\n"
    "/* 1. Balanced constructs */\n"
    "%typedef void (*(*signal)(int))(int);\n"
    "int matrix[3][4][5];\n"
    "%struct S { int x; char y; };\n"
    "\n"
    "/* 2. Unbalanced (for error handling) */\n"
    "%struct Bad { int x;  /* missing } */\n"
    "\n"
    "/* 3. In comments */\n"
    "/* { [ ( nested ) ] } in comment */\n"
    "%struct Good { int recovered; };\n"
    "\n"
    "/* 4. GT annotations */\n"
    "%union U { struct { int a; } s; int b[5]; };\n"
    "\n"
    "/* 5. Maximum nesting */\n"
    "int (*(*complex)[5])(int, char**);\n"
    "\n"
    "GC roots {\n"
    "    %struct S* root_struct;\n"
    "    %union U union_array[3];\n"
    "    void (*funcs[2])(int);\n"
    "}\n";

/* Write test content to file */
static int write_test_file(const char *filename, const char *content) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return 0;
    }
    
    fputs(content, f);
    fclose(f);
    return 1;
}

/* Simplified parser simulation to directly test the switch logic */
static void simulate_parser_logic(const char *input) {
    /* This simulates the tokenization and switch logic from gengtype-parse.cc */
    const char *p = input;
    int paren_depth = 0;
    int bracket_depth = 0;
    int brace_depth = 0;
    
    while (*p) {
        switch (*p) {
            case '(':
                paren_depth++;
                /* Simulate consume_balanced('(', ')') */
                {
                    const char *q = p + 1;
                    int depth = 1;
                    while (*q && depth > 0) {
                        if (*q == '(') depth++;
                        else if (*q == ')') depth--;
                        q++;
                    }
                    p = q - 1;
                }
                break;
                
            case '[':
                bracket_depth++;
                /* Simulate consume_balanced('[', ']') */
                {
                    const char *q = p + 1;
                    int depth = 1;
                    while (*q && depth > 0) {
                        if (*q == '[') depth++;
                        else if (*q == ']') depth--;
                        q++;
                    }
                    p = q - 1;
                }
                break;
                
            case '{':
                brace_depth++;
                /* Simulate consume_balanced('{', '}') */
                {
                    const char *q = p + 1;
                    int depth = 1;
                    while (*q && depth > 0) {
                        if (*q == '{') depth++;
                        else if (*q == '}') depth--;
                        q++;
                    }
                    p = q - 1;
                }
                break;
                
            case ')':
                if (paren_depth > 0) paren_depth--;
                break;
                
            case ']':
                if (bracket_depth > 0) bracket_depth--;
                break;
                
            case '}':
                if (brace_depth > 0) brace_depth--;
                break;
                
            case '/':
                /* Skip comments */
                if (p[1] == '*') {
                    p += 2;
                    while (*p && !(*p == '*' && p[1] == '/')) p++;
                    if (*p) p += 2;
                } else if (p[1] == '/') {
                    p += 2;
                    while (*p && *p != '\n') p++;
                }
                break;
                
            case '\'':
            case '\"':
                /* Skip character and string literals */
                {
                    char quote = *p++;
                    while (*p && *p != quote) {
                        if (*p == '\\' && p[1]) p += 2;
                        else p++;
                    }
                    if (*p == quote) p++;
                }
                break;
                
            default:
                /* Advance to next character */
                break;
        }
        p++;
    }
    
    printf("Parser simulation complete. Depth counts: paren=%d, bracket=%d, brace=%d\n",
           paren_depth, bracket_depth, brace_depth);
}

/* Main test driver */
int main(void) {
    const char *test_files[] = {
        "test1_balanced.gt",
        "test2_unbalanced.gt",
        "test3_comments.gt",
        "test4_gt_annotations.gt",
        "test5_nesting.gt",
        "test6_combined.gt"
    };
    
    const char *test_contents[] = {
        test1_content,
        test2_content,
        test3_content,
        test4_content,
        test5_content,
        combined_test_content
    };
    
    int num_tests = sizeof(test_files) / sizeof(test_files[0]);
    int i;
    
    printf("Generating gengtype test files for parser coverage...\n");
    
    /* Write all test files */
    for (i = 0; i < num_tests; i++) {
        printf("Creating %s... ", test_files[i]);
        if (write_test_file(test_files[i], test_contents[i])) {
            printf("OK\n");
            
            /* Simulate parser logic on each file */
            printf("Simulating parser on %s...\n", test_files[i]);
            simulate_parser_logic(test_contents[i]);
        } else {
            printf("FAILED\n");
        }
    }
    
    /* Create a comprehensive test that includes all patterns */
    printf("\nCreating comprehensive test file...\n");
    FILE *f = fopen("comprehensive.gt", "w");
    if (f) {
        for (i = 0; i < num_tests - 1; i++) {
            fputs(test_contents[i], f);
            fputs("\n\n", f);
        }
        fclose(f);
        printf("Created comprehensive.gt\n");
    }
    
    printf("\nTest files generated. To run actual gengtype:\n");
    printf("  ./gengtype -p comprehensive.gt\n");
    printf("  ./gengtype -r test1_balanced.gt\n");
    printf("  ./gengtype -r test2_unbalanced.gt  # Should show error handling\n");
    
    /* Cleanup would normally happen here, but we keep files for inspection */
    printf("\nTest files remain for inspection. Coverage analysis:\n");
    printf("1. Compile gengtype with: g++ -O0 -fprofile-arcs -ftest-coverage \\\n");
    printf("   -I. -I../../include -o gengtype-instr gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
    printf("2. Run: ./gengtype-instr -p comprehensive.gt\n");
    printf("3. Generate coverage: gcov gengtype-parse.cc\n");
    
    return 0;
}
