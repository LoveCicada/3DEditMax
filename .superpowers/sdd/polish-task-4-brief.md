### Task 4: QSS theme (DX11-Study palette)

**Files:**
- Create: `src/app/app.qss` (or embed string)
- Modify: `src/main.cpp` or `MainWindow` 鈥?`QApplication::setStyle("Fusion")` + load QSS
- Modify: `CMakeLists.txt` if copying `app.qss` next to exe
- Modify: Transform / matrix / tutorial panels lightly for World/View/Projection/MVP accent (GroupBox title or left color bar)

Palette from DX11-Study `:root`:

| Token | Hex |
|-------|-----|
| bg-deep | `#0f0f1a` |
| bg-panel | `#1a1a2e` |
| bg-section | `#16213e` |
| bg-cell | `#0d0d1a` |
| border | `#2a2a4a` |
| text | `#ccd6f6` |
| text-dim | `#8892b0` |
| highlight | `#61dafb` |
| World | `#ff6b6b` |
| View | `#4ecdc4` |
| Projection | `#f7b731` |
| MVP | `#a55eea` |

- [ ] **Step 1:** Apply Fusion + QSS: 4鈥?px radius on buttons, docks, spinboxes, combos; mono for matrix cells.
- [ ] **Step 2:** Demo button gold border `#f7b731`. Hover/active use `#61dafb`.
- [ ] **Step 3:** Section accents for World / View / Projection / MVP.
- [ ] **Step 4:** Manual: dark rounded chrome; docks opaque but color-matched.
- [ ] **Step 5:** Commit: `Apply DX11-Study-inspired Fusion QSS theme.`

---

