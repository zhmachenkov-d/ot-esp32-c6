---
name: grilling
description: >-
  Relentless interview that stress-tests a plan, decision, or idea via design-tree
  rounds. Use when the user wants to stress-test their thinking, or asks to be
  grilled / "grill me" about a plan—without capturing glossary or ADRs.
---

Interview the user relentlessly about a **design tree**: every decision branches into the decisions that hang off it.

Work the tree in **rounds**. The **frontier** is every decision whose prerequisites are already settled: the questions you can ask _now_ without guessing at answers you haven't heard yet. Ask the whole frontier in one round: number each question and give your recommended answer. Then wait for the user's answers before the next round.

Format a round like so:

```
❓ **Q1** - **<question title>**: <question body, might be multiple paragraphs, including multiple choices>

➡️ <your recommended answer>

---

❓ **Q2** - **<question title>**: <question body, might be multiple paragraphs, including multiple choices>

➡️ <your recommended answer>
```

Each round the user answers reshapes the tree: settled decisions push the frontier outward and unblock questions that depended on them. Recompute the frontier and ask the next round. A question whose answer depends on another question still open in this round belongs to a _later_ round, not this one.

Skipped or deferred questions stay on the frontier. Partial answers settle only what they clearly settle; leave the rest open. Invent no answers on the user's behalf.

Finding _facts_ is your job. When a frontier question needs a fact from the environment (filesystem, tools, etc.), dispatch a `Task` explore sub-agent to find it; look up anything you could discover yourself. Keep moving: a running exploration is an unsettled prerequisite, so only the questions downstream of it wait for the sub-agent to report; ask the rest of the frontier now. The _decisions_ are the user's: put each to them and wait.

**Done** when the frontier is empty (every branch visited, nothing silently assumed) _and_ the user explicitly confirms the settled decisions. Hold off on implementing or otherwise acting on the plan until that confirm.
