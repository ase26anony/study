// Compiler must handle:
1. Many live ranges across loops
2. Multiple exit paths (normal exit + early return)
3. Variables used in unpredictable patterns
4. Mix of integer and float operations
5. Volatile assembly preventing many optimizations
