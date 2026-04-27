#!/usr/bin/env python3
"""
s1-powerkey: S1 power button timing daemon for MO-62A.

Three states based on how long S1 is held:
  < 3s   → systemctl reboot            (triggered on release)
  ≥ 3s   → XFCE4 shutdown dialog       (triggered while still held)
  ≥ 5s   → systemctl poweroff           (triggered while still held)
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
T_DIALOG    = 3.0   # seconds before showing XFCE dialog
T_POWEROFF  = 5.0   # seconds before forcing poweroff

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
    log.info("3s → XFCE shutdown dialog")
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

    display = env.get("DISPLAY", ":0")
    dbus    = env.get("DBUS_SESSION_BUS_ADDRESS", "")
    cmd     = f"DISPLAY={display} DBUS_SESSION_BUS_ADDRESS={dbus} xfce4-session-logout"
    proc    = subprocess.Popen(["su", "debian", "-c", cmd],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        _, err = proc.communicate(timeout=5)
        if proc.returncode != 0:
            log.warning("xfce4-session-logout rc=%d stderr=%r", proc.returncode, err)
    except subprocess.TimeoutExpired:
        pass  # dialog is showing — expected


def main():
    dev = None
    while dev is None:
        dev = find_device()
        if dev is None:
            log.warning("Waiting for %s...", DEVICE_NAME)
            time.sleep(1)

    log.info("Monitoring %s", dev)

    time_press     = None
    dialog_timer   = None
    poweroff_timer = None

    with open(dev, "rb") as f:
        while True:
            data = f.read(EVENT_SIZE)
            if len(data) < EVENT_SIZE:
                continue
            _, _, etype, code, value = struct.unpack(EVENT_FMT, data)
            if etype != EV_KEY or code != KEY_POWER:
                continue

            if value == 1:  # press
                time_press     = time.monotonic()
                dialog_timer   = threading.Timer(T_DIALOG,   show_xfce_dialog)
                poweroff_timer = threading.Timer(T_POWEROFF, lambda: (
                    log.info("5s → poweroff") or subprocess.run(["systemctl", "poweroff"])
                ))
                dialog_timer.start()
                poweroff_timer.start()

            elif value == 0 and time_press is not None:  # release
                held       = time.monotonic() - time_press
                time_press = None
                dialog_timer.cancel()
                poweroff_timer.cancel()
                dialog_timer = poweroff_timer = None

                if held < T_DIALOG:
                    log.info("%.2fs → reboot", held)
                    subprocess.run(["systemctl", "reboot"])
                # ≥ T_DIALOG: dialog already fired; poweroff timer cancelled


if __name__ == "__main__":
    main()
