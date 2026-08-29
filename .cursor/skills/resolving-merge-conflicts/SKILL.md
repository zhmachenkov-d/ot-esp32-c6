---
name: resolving-merge-conflicts
description: Resolve in-progress git merge or rebase conflicts. Use when conflict markers appear (<<<<<<<), merge/rebase stops on conflicts, or the user asks to resolve merge or rebase conflicts.
---

1. **See the current state** of the merge/rebase. Check status, history, and the conflicting files. Done when you can list every conflicted path and whether this is a merge or a rebase.

2. **Find the primary sources** for each conflict. Read the commit messages, PRs, and issues/tickets that produced each side; understand why each change was made. Done when you can state each side's intent in one sentence.

3. **Resolve each hunk.** Preserve both intents where possible. Where incompatible, pick the one matching the merge's stated goal and note the trade-off. Keep existing behaviour; invent nothing new. Resolve to completion unless the user explicitly asks to `--abort`. Done when conflict markers are gone and `git status` shows no unmerged paths.

4. **Run automated checks.** Discover the project's checks from the environment (CI config, package scripts, Makefile) and run them—typically typecheck, then tests, then format. Fix anything the merge broke. Done when the checks you found pass.

5. **Finish the merge/rebase.** Stage everything and commit (message shape: follow `git-workflow` if present). If rebasing, continue until all commits are rebased. Done when status is clean and no rebase/merge is in progress.
