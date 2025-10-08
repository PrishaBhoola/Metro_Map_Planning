Metro Map Planning — SAT encoding guide

This document gives a concrete plan and recommendations for implementing the CNF encoder (the meat of the assignment) in C++.

Contract (inputs/outputs)

- Input: <name>.city as described in the assignment (scenario, N M K J, K start/end pairs, optional P popular cells).
- Output (run1): <name>.satinput — a DIMACS CNF file where variables are positive integers and clauses end with 0.
- Auxiliary output: <name>.vars — a human-readable mapping from variable IDs to boolean meanings (useful for decoding).
- Input (run2): <name>.satoutput — the output from MiniSat; use it and the var map to produce <name>.metromap file.

Basic variable ideas

- Cell occupancy variables: X(k,x,y) = true iff metro line k uses cell (x,y). Map to an integer id via a function var_X(k,x,y).
- Edge/Dir variables: E(k,x,y,d) = true iff line k uses the outgoing edge from (x,y) in direction d (d in {L,R,U,D}). Using edges can help enforce path continuity and degree constraints.
- Direction at cell: Dir(k,x,y) may be implied by E variables.
- Turn indicator: T(k,x,y) = true iff line k makes a turn at cell (x,y) — used to limit number of turns per line.
- Order/flow variables: To ensure the X variables form a single simple path from sk to ek for each k, you can encode a flow or ordering. Two approaches:
  1. Flow with unit capacity: treat start as source with one outgoing flow and end as sink; edges used by k carry flow. This needs integer flow variables or edge-use booleans plus conservation clauses.
  2. Path ordering: assign integer positions P(k,x,y,p) meaning cell (x,y) is position p along line k. This can be large (O(N*M*K\*L)).

Recommended encoding (compact, standard):

1. Use edge variables E(k,x,y,d) for 4 directions. Add clauses to ensure that if E(k,x,y,d) is true then the target cell is occupied by k and the opposite edge is true.
2. Degree constraints per cell for each line: for an internal visited cell, exactly two incident edges are used; at starts/ends exactly one incident edge is used.
3. Global cell-uniqueness: for every cell (x,y), across all lines k, at most one line uses it. Use at-most-one across K items.
4. Path connectivity and acyclicity: With degree=2 at internals and start/end degree 1, you still can form cycles disconnected from endpoints. To exclude cycles, enforce flow from start: introduce flow variables F(k,x,y) representing reachable from start via used edges, or use directed edges and enforce that only edges consistent with flow from start are allowed. A standard trick: create reachability by BFS ordering or capture with unit flow from start to end — add an integer flow variable per edge, but for SAT encode unit flow by splitting each used edge into 'forward' flow boolean and conservation clauses. This is the trickiest part; consider
   - Use the 'reachability via propagation' SAT encoding: declare R(k,x,y) booleans for reachability from sk along used edges; add R(sk)=true; For any used outgoing edge E(k,x,y,d) to (x',y'), require R(x,y)->R(x',y') and require that every occupied cell except start has at least one incoming edge from a reachable cell; and force R(end) true. That enforces a path connected to start reaching end while ruling out disconnected cycles.

Turn counting

- Define T(k,x,y) boolean that is true when two incident edges on (x,y) used by k are not collinear (one horizontal and one vertical). Write clauses that set T(k,x,y) iff (E left/right used) and (E up/down used). Then limit sum of T(k,_,_) <= J via a cardinality constraint.

At-most-one and cardinality encodings

- For small sets (K small) pairwise at-most-one is OK: for variables a,b in set, add (-a -b).
- For larger sets, use Sinz's sequential counter (linear number of clauses and aux vars). It's widely-used and straightforward to implement. Use it for "sum of turns <= J" and for per-cell uniqueness if K is large.

Practical variable mapping example

- Reserve contiguous ranges for each variable kind. For example:
  id = base_X + k*(N*M) + y*N + x for X(k,x,y)
  id = base_E + k*(N*M*4) + (y*N+x)*4 + d for E(k,x,y,d)
  id = base_T + k*(N*M) + y*N + x for T(k,x,y)
  id = base_R + k*(N*M) + y*N + x for R(k,x,y)
- Keep `vars.txt` with lines: id name

File workflow and scripts

- `compile.sh` compiles sources into `bin/run1` and `bin/run2`.
- `run1.sh test` runs `bin/run1 test.city test.satinput test.vars`. It must not call minisat.
- TA will run `minisat test.satinput test.satoutput` externally.
- `run2.sh test` runs `bin/run2 test.city test.satoutput test.metromap test.vars`.

Testing locally

- Create a small hand-crafted `.city` file and run through the scripts.
- For debugging, write small helper flags to dump clauses or variable counts.

References

- MiniSat user guide: http://www.dwheeler.com/essays/minisat-user-guide.html
- Sinz sequential counter: "Towards an Optimal CNF Encoding of Boolean Cardinality Constraints" (Sinz 2005)

Next steps

- Implement the variable maps and clause generators in `src/run1.cpp` following the recommended mapping.
- Implement decoding in `src/run2.cpp` to parse `vars.txt` and minisat's `satoutput` to reconstruct the assigned variables and print paths.

If you want, I can now:

- Expand `src/run1.cpp` to include variable-id helper functions and scaffolding for building Sinz encodings, or
- Provide a worked example encoding for a small N\*M,K,J instance in C++ that you can adapt.

Tell me which you'd prefer.
