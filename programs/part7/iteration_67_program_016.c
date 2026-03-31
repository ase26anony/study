This program combines:

1. **Explicit constructors/operators** (`ExplicitClass`, `Container`)
2. **Optional types** (`std::optional`, `TaggedUnion`)
3. **Special storage** (`__attribute__((section))`, `__thread`, `register`)
4. **Non-standard array bounds** (GNU extension `-5 ... 5`)
5. **Mutable/const members** in classes
6. **Picture strings** (though DWARF may not generate this for C++)
7. **Mixed function prototypes** (K&R and ANSI)
8. **Thread-local with scaling hints** (`aligned(64)`)
9. **String length attributes** via `FixedString` typedef
10. **Complex nested types** with templates and namespaces
11. **`volatile` and `asm`** to prevent elimination
12. **Both C and C++ constructs** via `extern "C"`

Compile with:
