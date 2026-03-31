GPU Grid/Teams
┌─────────┬─────────┬─────────┐
│ Team 0  │ Team 1  │ Team 2  │  ← Outer j-loop distributed
└─────────┴─────────┴─────────┘
    ↓        ↓        ↓
  SIMD     SIMD     SIMD      ← Inner i-loop vectorized
  lanes    lanes    lanes
