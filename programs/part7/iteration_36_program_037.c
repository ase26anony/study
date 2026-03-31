**Explanation of each test case:**

1. **RT_EXTERN**: `"C" { ... }` without `extern` triggers expectation for `extern` keyword in linkage specification.

2. **RT_STATIC_ASSERT**: `(true, "message")` without `static_assert` triggers expectation for `static_assert` keyword.

3. **RT_DECLTYPE**: Trailing return type `-> (x)` without `decltype` triggers expectation for `decltype` keyword.

4. **RT_OPERATOR**: `TestStruct::+(...)` without `operator` triggers expectation for `operator` keyword in operator overload.

5. **RT_CLASS**: `MissingClassKeyword { ... }` without `class` triggers expectation for `class` keyword in class definition.

6. **RT_TEMPLATE**: `<typename T>` without preceding `template` triggers expectation for `template` keyword.

7. **RT_NAMESPACE**: `MyNamespace { ... }` without `namespace` triggers expectation for `namespace` keyword.

8. **RT_USING**: `namespace std;` without `using` triggers expectation for `using` keyword in using-directive.

9. **RT_ASM**: `volatile ("mov ...")` without `asm` triggers expectation for `asm` keyword in inline assembly.

10. **RT_TRY**: `{ ... } catch (...)` without `try` triggers expectation for `try` keyword.

**Compilation recommendations for coverage:**
