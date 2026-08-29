---
name: code-review
description: "Two-axis review (Standards vs Spec) of changes since a fixed point. Use when the user wants a standards-and-spec review of a branch, PR, or WIP, or asks to review since X."
argument-hint: "commit, branch, tag, or merge-base"
---

Review the diff since a **fixed point** on two axes:

- **Standards**: documented repo standards plus the smell baseline.
- **Spec**: the originating issue / spec.

## Process

### 1. Pin the fixed point

Ref is what the user named (SHA, branch, tag, `main`, `HEAD~5`, PR base). If they named a PR (URL or number), `gh pr view` it: its base is the ref, its body is a spec source. If they named nothing, use the repo's default branch (`git rev-parse --abbrev-ref origin/HEAD`, else `main`, else `master`).

Resolve: `git merge-base HEAD <ref>`. Confirm `git rev-parse` succeeds.

Diff command: `git diff <merge-base>` (working tree, so WIP is included). If they asked for committed-only, `git diff <merge-base> HEAD` instead. Also `git log <merge-base>..HEAD --oneline`.

Review the current worktree. If they named a PR and HEAD is not that PR's head, say so.

Done when the merge-base resolves and the diff is non-empty. A bad ref or empty diff fails here, not inside the sub-agents.

### 2. Identify the spec source

In this order:

1. The PR body from step 1, if any.
2. Issue/PR refs in the commit messages (`#123`, `Closes #45`), fetched with `gh issue view` / `gh pr view`.
3. A path the user passed.
4. A spec file under `docs/`, `specs/`, or `.scratch/` matching the branch name or feature.
5. If nothing is found, ask. If they say there isn't one, skip Spec.

Done when you have spec contents or a skip.

### 3. Identify the standards sources

Every in-repo doc that states how to write code, including `.cursor/rules/`. Done when the path list is complete (it may be empty).

### 4. Spawn both sub-agents in parallel

Launch `generalPurpose` sub-agents in one message: Standards always; Spec unless skipped. Pass the git commands, not the diff.

**Standards** gets: the diff command, the commit list, the standards-source paths, and "Read [smells.md](smells.md) (`.cursor/skills/code-review/smells.md`) and apply it. Report, per file/hunk where relevant, (a) every place the diff violates a documented standard: cite the standard (file + the rule); and (b) any baseline smell you spot: name it and quote the hunk. Under 400 words."

**Spec** gets: the diff command, the commit list, the spec path or fetched contents, and "Report: (a) requirements the spec asked for that are missing or partial; (b) behaviour in the diff that wasn't asked for (scope creep); (c) requirements that look implemented but where the implementation looks wrong. Quote the spec line for each finding. Under 400 words."

If a sub-agent fails, retry it once; if it fails again, put the error under that heading and still present the other axis.

### 5. Aggregate

Present the reports under `## Standards` and `## Spec`, verbatim or lightly cleaned, so one axis cannot mask the other (a change can pass Standards and fail Spec, or the reverse). If Spec was skipped, `## Spec` is `no spec available`.

End with a one-line summary: findings per axis, and the worst issue within each (if any).
