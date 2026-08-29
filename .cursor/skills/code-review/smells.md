# Smell baseline

A curated Fowler set recast for C/C++. It always applies on the Standards axis, even when the repo documents nothing.

**The repo overrides.** A documented repo standard always wins; where it endorses something the baseline would flag, suppress the smell.

**Always a judgement call.** Each smell is a labelled heuristic ("possible Feature Envy"), never a hard violation. Documented-standard breaches can be hard. Skip anything tooling already enforces.

Each smell reads *what it is* → *how to fix*:

- **Mysterious Name**: a function, type, macro, or enumerator whose name doesn't reveal what it does or holds. → rename it; if no honest name comes, the design's murky.
- **Duplicated Code**: the same logic shape appears in more than one hunk or file. → extract a `static` helper (or a shared `.c`) and call it from both.
- **Feature Envy**: a function that reads and writes another translation unit's struct fields more than its own. → move it into that module, or add a function there and call it.
- **Data Clumps**: the same few params keep travelling together (`buf`+`len`, handle+flags). → bundle them into a struct, pass that.
- **Primitive Obsession**: an `int`, `char*`, or `bool` standing in for a domain concept (IDs, units, error/state). → `typedef`, `enum`, or a small struct.
- **Repeated Switches**: the same `switch`/`if`-cascade on the same enum or tag recurs across the change. → one function-pointer table (or one shared dispatch) both sites call.
- **Shotgun Surgery**: one logical change forces scattered edits across many `.c`/`.h` (or `.cpp`/`.hpp`) files. → gather what changes together into one module.
- **Divergent Change**: one translation unit is edited for several unrelated reasons. → split so each module changes for one reason.
- **Speculative Generality**: macros, `void*` ctx, extra params, templates, or `#ifdef`s added for needs the spec doesn't have. → delete them; inline back until a real need shows.
- **Pointer Chains**: long `a->b->c->d` walks the caller shouldn't depend on. → hide the walk behind one function in the first object's module.
- **Middle Man**: a function or translation unit that mostly just forwards. → cut it, call the real target.
- **Refused Bequest**: a C++ override, or a C callback/vtable, that ignores most of what it inherits. → drop the inheritance/vtable; use a narrower concrete API.
