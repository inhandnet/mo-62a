#!/usr/bin/env python3
"""
s1-powerkey: S1 power button timing daemon for MO-62A.

Mirrors Ubuntu laptop power-button behaviour:
  press         → XFCE4 shutdown dialog (immediately, while still held)
  hold ≥ 3s     → systemctl poweroff (dialog bypassed)
  release < 3s  → dialog stays open for user interaction
  ~7s           → PMIC hardware forced shutdown (FSD, not software-controlled)
"""
import glob
import logging
import os
import struct
import subprocess
import threading
import time

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s s1-powerkey %(levelname)s: %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)
log = logging.getLogger("s1-powerkey")

DEVICE_NAME = "tps6594-pwrbutton"
EV_KEY      = 1
KEY_POWER   = 116
T_POWEROFF  = 3.0   # hold ≥ 3s → force poweroff

# struct input_event on aarch64: timeval(8+8) + type(2) + code(2) + value(4) = 24 bytes
EVENT_FMT  = "llHHi"
EVENT_SIZE = struct.calcsize(EVENT_FMT)


def find_device():
    for name_path in glob.glob("/sys/class/input/input*/name"):
        try:
            with open(name_path) as f:
                if f.read().strip() != DEVICE_NAME:
                    continue
            input_dir = os.path.dirname(name_path)
            for event in glob.glob(os.path.join(input_dir, "event*")):
                return "/dev/input/" + os.path.basename(event)
        except OSError:
            continue
    return None


def show_xfce_dialog():
    env = {}
    try:
        result = subprocess.run(
            ["pgrep", "-u", "debian", "-x", "xfce4-session"],
            capture_output=True, text=True,
        )
        pid = result.stdout.strip().split()[0]
        with open(f"/proc/{pid}/environ", "rb") as f:
            for var in f.read().split(b"\x00"):
                if var.startswith((b"DISPLAY=", b"DBUS_SESSION_BUS_ADDRESS=")):
                    k, v = var.decode(errors="replace").split("=", 1)
                    env[k] = v
    except Exception as e:
        log.warning("Cannot find XFCE session: %s — no dialog shown", e)
        return

    log.info("Press → XFCE shutdown dialog")
    display = env.get("DISPLAY", ":0")
    dbus    = env.get("DBUS_SESSION_BUS_ADDRESS", "")
    cmd     = f"DISPLAY={display} DBUS_SESSION_BUS_ADDRESS={dbus} xfce4-session-logout"
    subprocess.Popen(["su", "debian", "-c", cmd],
                     stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def main():
    dev = None
    while dev is None:
        dev = find_device()
        if dev is None:
            log.warning("Waiting for %s...", DEVICE_NAME)
            time.sleep(1)

    log.info("Monitoring %s", dev)

    poweroff_timer = None

    with open(dev, "rb") as f:
        while True:
            data = f.read(EVENT_SIZE)
            if len(data) < EVENT_SIZE:
                continue
            _, _, etype, code, value = struct.unpack(EVENT_FMT, data)
            if etype != EV_KEY or code != KEY_POWER:
                continue

            if value == 1:  # press — show dialog immediately and start poweroff countdown
                threading.Thread(target=show_xfce_dialog, daemon=True).start()
                poweroff_timer = threading.Timer(T_POWEROFF, lambda: (
                    log.info("3s hold → poweroff") or
                    subprocess.run(["systemctl", "poweroff"])
                ))
                poweroff_timer.start()

            elif value == 0:  # release — cancel poweroff, dialog stays open
                if poweroff_timer:
                    poweroff_timer.cancel()
                    poweroff_timer = None


if __name__ == "__main__":
    main()
