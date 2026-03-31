This line uses **copy-initialization** syntax. Even though `ExplicitClass(42)` creates a temporary, the copy constructor is `explicit`, so this should fail to compile because:
- The copy constructor is `explicit`
- Copy-initialization requires the copy constructor to be non-`explicit`

3. **Working Lines**:
