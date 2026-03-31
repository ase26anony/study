GPU/Device Execution:
┌─────────────────────────────────────────┐
│ Team 0 (j=0..N/num_teams)               │
│   Threads execute inner loop with SIMD  │
├─────────────────────────────────────────┤
│ Team 1 (j=N/num_teams..2N/num_teams)    │
│   Threads execute inner loop with SIMD  │
├─────────────────────────────────────────┤
│ ...                                     │
└─────────────────────────────────────────┘
