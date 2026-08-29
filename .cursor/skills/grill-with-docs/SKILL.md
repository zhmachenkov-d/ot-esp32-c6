---
name: grill-with-docs
description: >-
  Grill a plan or design and capture glossary/ADRs as decisions settle. Use when
  the user asks to grill with docs, grill and document, or stress-test a design
  while writing CONTEXT.md or ADRs.
argument-hint: "What plan or design should I grill and document?"
---

Read and follow [../grilling/SKILL.md](../grilling/SKILL.md) and [../domain-modeling/SKILL.md](../domain-modeling/SKILL.md).

Run **grilling** rounds as the spine. As each answer settles a term or decision, apply **domain-modeling** inline in that same turn (glossary update, ADR offer when its gates hold)—do not batch docs for the end.

**Done** when the grilling frontier is empty, crystallised language and any warranted ADRs are written, _and_ the user explicitly confirms the settled decisions.
