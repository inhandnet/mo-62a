from interface.expansion.gpio_loopback import get_tests as gpio_tests


def get_tests(board) -> list:
    return gpio_tests(board)


__all__ = ["get_tests"]
