**Explanation of each test case:**

1. **RT_EXTERN**: `"C" { ... }` without `extern` triggers expectation for `extern` keyword in linkage specification.

2. **RT_STATIC_ASSERT**: `(sizeof(int) == 4, "message")` without `static_assert` triggers expectation for `static_assert` keyword.

3. **RT_DECLTYPE**: `auto func() -> (int)` with parentheses instead of a type triggers expectation for `decltype` in trailing return type.

4. **RT_OPERATOR**: `+() { ... }` without `operator` triggers expectation for `operator` keyword in overload declaration.

5. **RT_CLASS**: `MyClass { ... }` without `class`/`struct` triggers expectation for class-key.

6. **RT_TEMPLATE**: `<typename T>` without `template` triggers expectation for `template` keyword.

7. **RT_NAMESPACE**: `MyNS { ... }` without `namespace` triggers expectation for `namespace` keyword.

8. **RT_USING**: `namespace std;` without `using` triggers expectation for `using` keyword in using-directive.

9. **RT_ASM**: `volatile ("nop")` without `asm` triggers expectation for `asm` keyword in inline assembly.

10. **RT_TRY**: `{ ... } catch (...)` without `try` triggers expectation for `try` keyword before the block.

**Compilation recommendations for coverage:**
