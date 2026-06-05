---
name: codex-computer-use-fix
description: Diagnose and fix Codex Computer Use missing, unavailable, not showing, or broken on Windows/macOS. Use when a user says Codex Computer Use is gone, cannot be used, does not appear in Plugins or Settings, desktop control is unavailable, Chrome/Browser bundled plugins disappeared, or asks whether the PowerShell openai-bundled marketplace command is the solution.
---

# Codex Computer Use Fix

## Goal

Help users recover Codex Computer Use or explain why it is unavailable. Prefer local diagnostics before giving generic advice.

## Workflow

1. Use `openai-docs` if available for current Codex product facts.
2. Check the local Codex install and state:

```powershell
Get-AppxPackage 'OpenAI.Codex' | Select-Object Name, Version, InstallLocation | Format-List
Test-Path "$env:USERPROFILE\.codex\computer-use"
codex plugin marketplace list
```

3. On Windows, check whether the bundled plugin source exists:

```powershell
$p = Get-AppxPackage 'OpenAI.Codex'
$m = Join-Path $p.InstallLocation 'app\resources\plugins\openai-bundled'
Test-Path $m
Get-ChildItem $m -Force
```

4. If `codex plugin marketplace list` does not include `openai-bundled` and the bundled path exists, explain that this likely means Codex's bundled plugin marketplace cache was not registered or refreshed after an update.
5. Offer this repair command, or run it when the user clearly wants the fix applied:

```powershell
$p = Get-AppxPackage 'OpenAI.Codex'
$m = Join-Path $p.InstallLocation 'app\resources\plugins\openai-bundled'
codex plugin marketplace remove openai-bundled
codex plugin marketplace add $m
```

6. Ask the user to restart Codex, then check `/plugins`, Browser Use settings, Chrome plugin setup, and Computer Use settings.

## Interpretation

- If `openai-bundled` is missing from the marketplace but exists under the Codex app install path, the bundled marketplace re-add command is usually the right fix.
- If `openai-bundled` is present, diagnose permissions, region availability, account or workspace policy, stale app state, and whether the user is on a supported current Codex app version.
- If the bundled path does not exist, advise updating or reinstalling Codex rather than adding a nonexistent marketplace.
- Computer Use is distinct from Browser Use and the Chrome plugin. Browser/Chrome can control web flows; Computer Use controls desktop apps visually.

## Safety

Do not use destructive commands. Avoid changing unrelated plugin marketplaces or user config. Warn that Computer Use can view and interact with apps outside the workspace, so users should review permission prompts and keep tasks narrow.
