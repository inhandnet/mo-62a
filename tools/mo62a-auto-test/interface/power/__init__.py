from interface.power.fan    import get_tests as fan_tests
from interface.power.button import get_tests as button_tests
from interface.power.battery import get_tests as battery_tests


def get_tests(board) -> list:
    return fan_tests(board) + button_tests(board) + battery_tests(board)


__all__ = ["get_tests"]
