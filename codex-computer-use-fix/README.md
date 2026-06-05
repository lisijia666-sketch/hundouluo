# Codex Computer Use Fix

这个仓库用于帮助修复或诊断 **Codex Computer Use 不显示、不能用、插件消失、桌面控制不可用** 等问题。

它包含两种用法：

1. 给 Codex 安装一个技能，让以后遇到类似问题时自动按流程诊断。
2. 给 Windows 用户直接运行 PowerShell 脚本，重新注册 Codex 自带的 `openai-bundled` 插件市场。

## 适用场景

当你遇到这些情况时可以用：

- Codex 里看不到 **Computer Use**
- Codex 更新后 **Browser / Chrome / Computer Use** 插件消失
- `/plugins` 里没有内置插件
- 视频里说“一行 PowerShell 修复”，但你不知道是否安全
- Codex 不能控制桌面应用、Chrome 或浏览器
- `codex plugin marketplace list` 里没有 `openai-bundled`

## 原理

Codex Windows App 自带一组 bundled 插件，一般在安装目录里：

```text
app\resources\plugins\openai-bundled
```

如果 Codex 更新后本地插件市场没有刷新，可能会出现内置插件不显示的问题。修复方式是把这个 bundled 插件市场重新添加到 Codex：

```powershell
$p = Get-AppxPackage 'OpenAI.Codex'
$m = Join-Path $p.InstallLocation 'app\resources\plugins\openai-bundled'
codex plugin marketplace remove openai-bundled
codex plugin marketplace add $m
```

## 方法一：直接运行修复脚本（Windows）

在 PowerShell 中运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\repair-openai-bundled.ps1
```

脚本会做这些事：

- 检查 Codex Windows App 是否安装
- 找到 Codex 安装目录
- 检查 `openai-bundled` 插件市场路径是否存在
- 显示当前插件市场列表
- 移除旧的 `openai-bundled` 注册项
- 重新添加 Codex 自带的 `openai-bundled`
- 提醒你重启 Codex

运行后重启 Codex，再检查：

- `/plugins`
- Settings / Browser Use
- Settings / Computer Use
- Chrome plugin setup

## 方法二：安装 Codex 技能

如果你想让 Codex 以后自动处理这类问题，可以安装本仓库里的 skill。

### 用 Codex 的 skill installer 安装

在 Codex 里让它执行：

```text
Use $skill-installer to install this skill from https://github.com/lisijia666-sketch/hundouluo/tree/master/codex-computer-use-fix/skills/codex-computer-use-fix
```

或者使用本地 helper 脚本：

```powershell
python "$env:USERPROFILE\.codex\skills\.system\skill-installer\scripts\install-skill-from-github.py" --repo lisijia666-sketch/hundouluo --ref master --path codex-computer-use-fix/skills/codex-computer-use-fix
```

安装后重启 Codex。

### 手动安装

把这个目录复制到：

```text
%USERPROFILE%\.codex\skills\codex-computer-use-fix
```

目录结构应该是：

```text
%USERPROFILE%\.codex\skills\codex-computer-use-fix\SKILL.md
%USERPROFILE%\.codex\skills\codex-computer-use-fix\agents\openai.yaml
```

然后重启 Codex。

## 安装后怎么调用

安装并重启 Codex 后，直接对 Codex 说类似的话：

```text
我的 Codex Computer Use 用不了，帮我诊断并修复
```

或：

```text
Codex Computer Use 不显示了，是不是 openai-bundled 插件市场没刷新？
```

也可以显式调用：

```text
Use $codex-computer-use-fix to diagnose why Codex Computer Use is missing.
```

## 诊断命令

如果你不想立刻修复，可以先运行这些命令确认问题：

```powershell
Get-AppxPackage 'OpenAI.Codex' | Select-Object Name, Version, InstallLocation | Format-List
Test-Path "$env:USERPROFILE\.codex\computer-use"
codex plugin marketplace list
```

再检查 bundled 路径：

```powershell
$p = Get-AppxPackage 'OpenAI.Codex'
$m = Join-Path $p.InstallLocation 'app\resources\plugins\openai-bundled'
Test-Path $m
Get-ChildItem $m -Force
```

如果 `codex plugin marketplace list` 里没有 `openai-bundled`，但 `$m` 路径存在，那通常就是插件市场没有注册或更新后缓存没刷新。

## 注意事项

- 这个修复主要针对 Windows Codex App。
- 如果 `openai-bundled` 路径不存在，应该更新或重装 Codex，而不是强行添加。
- 如果 `openai-bundled` 已经存在但 Computer Use 仍不可用，继续检查地区、账号权限、企业策略、Codex 版本、系统权限和应用重启状态。
- Computer Use 可以看见屏幕并操作应用，请只给明确、窄范围的任务授权。
- Browser Use、Chrome plugin 和 Computer Use 是不同能力：Browser/Chrome 偏网页流程，Computer Use 偏桌面应用视觉操作。

## 仓库内容

```text
codex-computer-use-fix/
  README.md
  scripts/
    repair-openai-bundled.ps1
  skills/
    codex-computer-use-fix/
      SKILL.md
      agents/
        openai.yaml
```

## 免责声明

本仓库只是本地诊断和修复流程，不是 OpenAI 官方支持渠道。如果修复后仍不可用，请更新 Codex、重启电脑，并联系 OpenAI 支持或检查你的账号/地区/组织策略。
