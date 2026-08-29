---
name: handoff
description: Compact the current conversation into a handoff document for another agent to pick up.
argument-hint: "What will the next session be used for?"
disable-model-invocation: true
---

Write a **handoff**: a Markdown document that lets a fresh agent continue this work without replaying the thread.

**Pointers over copies.** Point at specs, plans, ADRs, issues, commits, and diffs by path or URL. Put in the handoff only what those artifacts do not already hold (session state, decisions still in chat, next moves).

**Redact secrets.** Strip API keys, passwords, tokens, and personally identifiable information; replace with placeholders when a value must be mentioned.

1. **Fix the focus.** If the user passed arguments, that is the next session's focus. If not, take the focus as continuing the current work (infer from the thread; do not ask). Done when you can state the focus in one sentence.

2. **Draft the handoff.** Fill every section in Document structure below, tailored to that focus. Suggested skills: name skills already used in this thread, plus any the focus clearly needs; each entry is a skill the next agent should read and follow. Done when every template section is present and non-empty (use `None` only where nothing applies).

3. **Write and report.** Save to `$TMPDIR/handoff-<slug>.md` (fall back to `/tmp` if `TMPDIR` is unset); slug from the focus. Tell the user the absolute path. Done when the file exists outside the workspace and the path is in your reply.

## Document structure

Write the handoff using the template below. Keep it short enough that a fresh agent can act on the focus in one read.

<handoff-template>

# Handoff: <focus, one line>

**Focus:** <one sentence — what the next session should accomplish>

## State

Where the work stands right now: what's done, in progress, and blocked. Facts only.

## Next actions

Ordered, concrete steps for the next agent. First item is the immediate next move.

## Artifacts

Paths or URLs the next agent should open (specs, plans, ADRs, issues, key files, PRs). One line each; no pasted contents.

## Suggested skills

Skills to read and follow, one per line as `skill-name` — why. Prefer skills already used in this thread and any the focus requires.

## Open questions

Unresolved decisions or unknowns. `None` if the path forward is clear.

</handoff-template>
