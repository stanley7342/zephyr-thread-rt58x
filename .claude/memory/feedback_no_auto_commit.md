---
name: No auto commit
description: User does not want automatic commits — always ask before committing
type: feedback
---

Do not automatically commit changes after editing files. Only commit when the user explicitly asks (e.g. "幫我 commit", "commit this").

**Why:** User prefers to control when commits are made.

**How to apply:** After making code changes, stop and wait. Never run `git add` + `git commit` without a direct user request.
