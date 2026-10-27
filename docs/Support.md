# Application support

Strug support ended on 2026-10-27. Current engine development targets Beatemup;
Strug is a historical application and is no longer a required reference or
compatibility target. Future Glade changes do not need to preserve APIs or
behaviour solely for Strug.

The last Glade version supported for Strug is:

- Commit: `aa1c8163a6d0c08f101e12b3f08f10aa93794620`
- Title: Add Context-managed fixed updates for game states

This is the support boundary chosen by the owner, not a new build or runtime
verification of Strug. Strug records the same checkpoint in its root
`GLADE_SUPPORT.md`. Its dependency checkout is separate and was not updated as
part of ending support.

Existing descriptions of legacy APIs and behaviour remain technical records of
the implementation, rather than a commitment to ongoing Strug compatibility.
Removing legacy systems or changing engine APIs remains separately scoped work.
