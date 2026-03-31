## Comparison logic:
1. **First compare high parts** as unsigned values
2. **Only if high parts are equal**, compare low parts
3. This implements **lexicographic comparison** for the double-width integer

## Missing part:
The function should end with:
