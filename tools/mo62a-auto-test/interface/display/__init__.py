from interface.display.hdmi   import get_tests as hdmi_tests
from interface.display.imx219 import get_tests as imx219_tests
from interface.power.led      import get_tests as led_tests


def get_tests(board) -> list:
    return hdmi_tests(board) + led_tests(board) + imx219_tests(board)


__all__ = ["get_tests"]
