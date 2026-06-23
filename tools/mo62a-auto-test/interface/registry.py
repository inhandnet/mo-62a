"""测试项注册表 — 根据 name_key 列表返回对应测试实例。"""

from __future__ import annotations
from interface.base import TestCase


def get_selected_tests(board, keys: list[str]) -> list[TestCase]:
    """
    根据选中的 name_key 列表，返回对应的测试实例（按 keys 顺序）。

    Args:
        board: SSHBoard 实例
        keys:  SelectPage 传入的 name_key 列表
    """
    all_tests = _build_registry(board)
    return [all_tests[k] for k in keys if k in all_tests]


def _build_registry(board) -> dict[str, TestCase]:
    """构建 name_key → TestCase 实例的映射。"""
    registry: dict[str, TestCase] = {}

    from interface.system    import get_tests as system_tests
    from interface.rtc       import get_tests as rtc_tests
    from interface.storage   import get_tests as storage_tests
    from interface.network   import get_tests as network_tests
    from interface.display   import get_tests as display_tests
    from interface.power     import get_tests as power_tests
    from interface.usb       import get_tests as usb_tests
    from interface.audio     import get_tests as audio_tests
    from interface.expansion import get_tests as expansion_tests
    for t in (system_tests(board) + rtc_tests(board) + storage_tests(board) +
              network_tests(board) + usb_tests(board) + audio_tests(board) +
              display_tests(board) + power_tests(board) + expansion_tests(board)):
        registry[t.name_key] = t

    return registry
