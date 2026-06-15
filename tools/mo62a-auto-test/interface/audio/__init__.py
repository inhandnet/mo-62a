from interface.audio.output import get_tests as _output_tests
from interface.audio.loopback import HeadphoneLoopbackTest


def get_tests(board) -> list:
    return _output_tests(board) + [HeadphoneLoopbackTest(board)]


__all__ = ["get_tests"]
