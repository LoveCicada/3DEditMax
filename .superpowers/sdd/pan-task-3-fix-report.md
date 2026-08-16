# Pan Task 3 Fix Report: TransformPanel collapsible spinboxes

## Status

**DONE**

## Problem

`makeSpin()` parented `QDoubleSpinBox` widgets to `TransformPanel` (`this`), but `makeCheckableGroup` only toggles visibility of direct children of each `QGroupBox`. Unchecking World/View/Projection hid form labels while spinboxes stayed visible.

## Fix

Added `QWidget* parent` to `makeSpin()`; each spinbox is now created with its owning group box (`world`, `view`, or `proj`) as parent. `ObjectPanel` already parented controls correctly — unchanged.

## Verification

- Build: PASS (MSVC via `vcvars64.bat`, Ninja Debug)
- CTest: PASS (`1/1 Test #1: 3deditmax_tests Passed`)

## Commit

`1846d28` — Fix TransformPanel collapsible groups to hide spinboxes.
