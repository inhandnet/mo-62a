from interface.power.fan import get_tests as fan_tests


def get_tests(board) -> list:
    return fan_tests(board)


__all__ = ["get_tests"]
