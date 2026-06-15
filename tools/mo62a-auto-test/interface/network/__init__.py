from interface.network.ethernet  import get_tests as eth_tests
from interface.network.wifi      import get_tests as wifi_tests
from interface.network.bluetooth import get_tests as bt_tests


def get_tests(board) -> list:
    return eth_tests(board) + wifi_tests(board) + bt_tests(board)


__all__ = ["get_tests"]
