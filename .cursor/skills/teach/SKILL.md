---
name: teach
description: Teach the user a new skill or concept, within this workspace.
disable-model-invocation: true
argument-hint: "What would you like to learn about?"
---

The user has asked you to teach them something. This is a stateful request—they intend to learn the topic over multiple sessions. Treat the current directory as the teaching workspace.

## Teaching Workspace

- `MISSION.md`: why the user is learning this. Format: [MISSION-FORMAT.md](./MISSION-FORMAT.md).
- `RESOURCES.md`: high-trust knowledge sources and communities. Format: [RESOURCES-FORMAT.md](./RESOURCES-FORMAT.md).
- `GLOSSARY.md`: canonical terms for this workspace. Format: [GLOSSARY-FORMAT.md](./GLOSSARY-FORMAT.md).
- `./learning-records/*.md`: non-obvious insights that steer future sessions (`0001-<dash-case-name>.md`). Format: [LEARNING-RECORD-FORMAT.md](./LEARNING-RECORD-FORMAT.md).
- `./lessons/*.html`: primary teaching unit—one tightly-scoped HTML lesson each (`0001-<dash-case-name>.html`).
- `./reference/*.html`: compressed cheat sheets for quick revisit (syntax, algorithms, poses, glossaries as HTML). Beautiful and print-friendly.
- `./assets/*`: reusable lesson **components**. See [Assets](#assets).
- `NOTES.md`: user preferences and working notes.

## Process

1. **Ground the mission.** If `MISSION.md` is missing or vague, interview the user on why they want to learn this; write or update it. Confirm with the user before changing an existing mission; capture a mission shift as a learning record. Done when `MISSION.md` states a concrete real-world outcome.

2. **Stock resources.** Before relying on parametric knowledge, find high-trust sources and record them in `RESOURCES.md`. Prefer this until the file is well-populated. Done when the next lesson's claims can be cited from listed sources (or gaps are listed explicitly).

3. **Pick the next lesson.** Use the user's request if they named one; otherwise choose from mission + learning-records + glossary what sits in their zone of proximal development—challenged just enough. Done when you can name one tightly-scoped skill to teach and why it fits.

4. **Write the lesson.** Follow [Lessons](#lessons) and [Assets](#assets). Teach only the knowledge needed for that skill, then practice via a tight feedback loop. Cite sources; open the lesson file for the user when you can. Update `./reference/` and `GLOSSARY.md` when a unit compresses cleanly. Done when the HTML file exists under `./lessons/`, ties to the mission, and the user can complete it in one short sitting.

5. **Capture what stuck.** Write a learning record when the user demonstrates non-trivial understanding, discloses prior knowledge, corrects a misconception, or shifts the mission—per [LEARNING-RECORD-FORMAT.md](./LEARNING-RECORD-FORMAT.md). Record teaching preferences in `NOTES.md`. Done when new decision-grade insights (if any) are on disk.

**Wisdom:** when a question needs real-world judgment, answer briefly, then point to a high-reputation community from `RESOURCES.md` (or find one). Respect opt-outs recorded there.

## Lessons

One self-contained HTML file per lesson under `./lessons/`. Beautiful typography (think Tufte); short enough for working memory; one tangible win; in the ZPD and tied to the mission.

- Link via HTML anchors to other lessons and `./reference/` docs.
- Recommend one primary high-trust source to read or watch.
- Remind the user they can ask follow-up questions—you are their teacher.
- **Knowledge** first (difficulty is the enemy of understanding), then **skill** practice (effortful retrieval builds storage strength, not mere fluency): quizzes, in-browser tasks, or guided real-world steps. Feedback should be immediate; for quizzes, equal-length answer options so formatting gives no clue.
- Design for storage strength: retrieval practice, spacing, and interleaving (skills practice only).

## Assets

Before authoring, read `./assets/` and reuse components (stylesheets, quiz widgets, simulators, diagram helpers). New reusable pieces go in `./assets/` and are linked—never inlined where a second lesson would duplicate them. A shared stylesheet is the first component every workspace earns.
