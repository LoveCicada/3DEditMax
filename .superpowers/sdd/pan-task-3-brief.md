### Task 3: Light layout looseness

Plan: docs/superpowers/plans/2026-08-16-viewport-pan-layout.md Task 3

- MainWindow: resizeDocks defaults left ~280-320, right ~360-400, bottom ~140
- TransformPanel (and object groups if present): QGroupBox setCheckable(true), hide contents when unchecked
- app.qss: slightly reduce rigid min-height/padding on docks; keep palette
- docs/ui/01-lab-shell-layout.md already says RMB pan — verify; update if needed
- Commit: Loosen lab shell docks with defaults and collapsible transform sections.
- Local only; ctest; no render/teaching core changes
