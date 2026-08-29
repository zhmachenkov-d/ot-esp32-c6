---
name: domain-modeling
description: Build and sharpen a project's domain model (glossary, ubiquitous language, ADRs). Use when changing domain language, writing or editing a CONTEXT.md, or recording or editing an ADR.
---

# Domain Modeling

Actively build and sharpen the project's domain model as you design: challenge terms, invent edge-case scenarios, and write the glossary and decisions down the moment they crystallise. This skill is for *changing* the model. Reading `CONTEXT.md` for vocabulary is a one-line habit any skill can do; it does not need this skill.

Layout, lazy creation, and single vs multi-context rules: [CONTEXT-FORMAT.md](./CONTEXT-FORMAT.md).

## During the session

### Challenge against the glossary

When a `CONTEXT.md` exists and the user uses a term that conflicts with it, call it out immediately. "Your glossary defines 'cancellation' as X, but you seem to mean Y. Which is it?" If no glossary exists yet, skip this and capture terms under Update when they resolve.

### Sharpen fuzzy language

When the user uses vague or overloaded terms, propose a precise canonical term. "You're saying 'account': do you mean the Customer or the User? Those are different things."

### Discuss concrete scenarios

When domain relationships are being discussed, stress-test them with specific scenarios. Invent scenarios that probe edge cases and force the user to be precise about the boundaries between concepts.

### Cross-reference with code

When the user states how something works, check whether the code agrees. If you find a contradiction, surface it: "Your code cancels entire Orders, but you just said partial cancellation is possible. Which is right?"

### Update CONTEXT.md inline

When a term is resolved, update `CONTEXT.md` right there. Don't batch these up: capture them as they happen. Use the format in [CONTEXT-FORMAT.md](./CONTEXT-FORMAT.md).

`CONTEXT.md` is a glossary of ubiquitous language only. Keep specs, scratch notes, and implementation decisions elsewhere.

### Offer ADRs sparingly

Only offer an ADR when the gates in [ADR-FORMAT.md](./ADR-FORMAT.md) all hold; use that file for location, numbering, and template. If any gate is missing, skip the ADR.
