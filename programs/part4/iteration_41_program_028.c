This script provides comprehensive coverage by:

1. **Sequential testing**: Each section tests specific groups of variables
2. **Cleanup between tests**: Files are cleaned up to ensure fresh state
3. **Error handling**: Uses `|| true` to continue after expected failures
4. **Multiple invocations**: Each `gcc` call triggers initialization/cleanup
5. **Combined scenarios**: Section 10 combines multiple flags to test complex state transitions

To run the test:
