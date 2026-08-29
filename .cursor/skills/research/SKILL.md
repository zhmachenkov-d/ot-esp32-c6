---
name: research
description: Investigate a question against high-trust primary sources and capture the findings as a Markdown file in the repo. Use when the user wants a topic researched, docs or API facts gathered, or reading legwork delegated to a sub-agent.
argument-hint: "What should I research?"
---

Dispatch a background `Task` (`generalPurpose`, `run_in_background: true`) so you keep working while it reads. Research runs in that sub-agent, not inline in this conversation.

1. **Fix the brief.** Question from the user's args or message. Output path: wherever this repo already keeps research notes; if none, `.scratch/research-<slug>.md` (create `.scratch/` if needed). Done when you can state the question in one sentence and the output path.

2. **Spawn.** Launch one `Task` with `subagent_type: generalPurpose` and `run_in_background: true`. Prompt includes the question, the output path, and: perform the Research job below; return the path plus a one-line summary when finished. Tell the user where the file will land. Done when the Task is running and the path is in your reply.

3. **When it finishes.** Confirm the file exists; give the user the path and the one-line summary. Done when that path is in your reply.

## Research job

For the sub-agent:

1. Investigate the question against **high-trust primary sources** (official docs, source code, specs, first-party APIs)—not secondary write-ups. Follow every claim back to the source that owns it.
2. Write one Markdown file at the given path:

```markdown
# <question, short>

## Findings

- <claim> — <source URL or repo path>

## Sources

- <URL or path> — <what it is>
```

Every claim in Findings carries a citation. Sources lists each distinct source once.

**Done** when the file exists at the path, every finding is cited, and you return the path plus a one-line summary.
