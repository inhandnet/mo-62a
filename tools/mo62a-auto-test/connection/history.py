"""连接历史管理 — 读写 ~/.mo62a_history.json

唯一性规则：IP + 用户名 + 密码三者同时相同才视为同一条记录；
任意一项不同即为新设备，单独存储。
最多保留 MAX_ENTRIES 条，按最近使用时间排序，超出时删除最旧的。
"""

from __future__ import annotations

import json
from datetime import datetime
from pathlib import Path

HISTORY_FILE = Path.home() / ".mo62a_history.json"
MAX_ENTRIES  = 5


def _key(ip: str, username: str, password: str) -> str:
    return f"{ip}|{username}|{password}"


def load() -> list[dict]:
    """加载历史记录，返回按 last_used 降序排列的列表。"""
    if not HISTORY_FILE.exists():
        return []
    try:
        data = json.loads(HISTORY_FILE.read_text(encoding="utf-8"))
        return data if isinstance(data, list) else []
    except Exception:
        return []


def save(ip: str, username: str, password: str, hostname: str = "") -> None:
    """连接成功后调用，保存或更新一条历史记录。"""
    entries = load()
    key = _key(ip, username, password)

    # 移除已存在的相同记录（更新场景）
    entries = [e for e in entries if _key(e["ip"], e["username"], e["password"]) != key]

    # 插入最新记录到头部
    entries.insert(0, {
        "ip":        ip,
        "username":  username,
        "password":  password,
        "hostname":  hostname,
        "last_used": datetime.now().strftime("%Y-%m-%d %H:%M"),
    })

    # 滚动截断
    entries = entries[:MAX_ENTRIES]

    HISTORY_FILE.write_text(
        json.dumps(entries, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )


def remove(ip: str, username: str, password: str) -> None:
    """删除指定记录。"""
    entries = load()
    key = _key(ip, username, password)
    entries = [e for e in entries if _key(e["ip"], e["username"], e["password"]) != key]
    HISTORY_FILE.write_text(
        json.dumps(entries, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
