<p align="right">
  <strong>简体中文</strong> · <a href="CI-sync-main.md">English</a>
</p>

# 上游同步（CI / Upstream Sync）

本礼物仓库默认禁用 `.github/workflows/sync-main.yml`：删除定时计划，任务同时要求
仓库是 fork 且显式设置仓库变量 `ENABLE_UPSTREAM_SYNC=true`。
此独立仓库未启用该变量。产品分支包含礼物应用，不应被上游硬件测试 demo 替换。
下文保留上游流水线设计作工程参考，并非本仓库已启用的同步任务。

本文件与 `.github/workflows/sync-main.yml` 一同维护，工作流行为变化时必须同步更新。

## 触发条件

- **schedule**：礼物仓库已移除。
- **workflow_dispatch**：保留入口，但未满足 fork 和显式变量两个条件时不会执行同步。

> 上游版本原本仅在 fork 时运行；此礼物仓库进一步禁用了任务执行。

## 流水线做了什么

1. **Checkout 目标仓库**：`actions/checkout` 显式设置 `ref: main`，即使手动触发选择其他分支，也检出当前 fork 的 `main`，并关闭 Git 凭证持久化。同步 Action 在 fetch 前需要本地 `main`；否则 `git checkout main` 可能把仓库内的 `main/` 目录作为路径处理，并未切换分支。
2. **同步上游**：使用固定到完整 commit SHA 的 `aormsby/fork-sync-with-upstream-action`（对应 v3.4.3），把 `FoloToy/ai-passport` 的 `main` 同步到本 fork 的 `main`。`target_repo_token` 使用自动生成且仅具 `contents: write` 权限的 `GITHUB_TOKEN`，无需手动配置。
3. **失败检查**：同步失败时输出提示——上游 workflow 文件变更可能导致 GitHub 暂停自动同步，需手动 Sync Fork 一次。

## 注意事项

- 同步目标与上游分支均为 `main`，与 fork 用户约定（`main` 仅允许修改根目录 `README.md` 与 `docs/assets/`）配合使用：`main` 保持与上游最新基线同步、不产生冲突。
- **在 `main` 直接开发的例外**：如果用户执意要在 `main` 分支直接开发，必须**停用/关闭本 workflow**（Actions 页面 → Disable），否则每日自动同步会把上游改动强行合入 `main`，产生冲突或覆盖本地开发内容。
- 若同步失败，查看 Actions 日志确认是否为上游 workflow 文件变更所致；必要时按提示手动在 GitHub 页面 Sync Fork。
- 升级 Action 时必须从官方仓库核对目标版本对应的完整 commit SHA，并同步更新 workflow 行尾的版本注释。

## 相关文件

- `.github/workflows/sync-main.yml`：本流水线定义。
