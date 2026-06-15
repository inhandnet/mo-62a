from interface.storage.ddr  import get_tests as ddr_tests
from interface.storage.sdcard import get_tests as sd_tests


def get_tests(board) -> list:
    return ddr_tests(board) + sd_tests(board)


__all__ = ["get_tests"]
