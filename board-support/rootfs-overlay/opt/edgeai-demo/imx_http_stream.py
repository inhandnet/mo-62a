#!/usr/bin/env python3
"""IMX219 (direct CSI) -> VPAC VISS/MSC -> C7x TIDL YOLOX -> MJPEG HTTP stream.

Data path uses TI hardware accelerators end to end:
  IMX219 RAW10 -> CSI2RX -> tiovxisp(VPAC VISS, debayer+2A) -> tiovxmultiscaler(VPAC MSC)
  -> onnxruntime TIDL EP (C7x DSP) -> draw -> MJPEG :8080

Run as root (needs /dev/mem, dma_heap, rpmsg). media-ctl must be configured first.
"""
import os, sys, time, threading, subprocess
import numpy as np
import cv2
import onnxruntime as ort
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

MODEL_DIR = "/opt/ti/model_zoo/ONR-OD-8200-yolox-nano-lite-mmdet-coco-416x416"
MODEL     = MODEL_DIR + "/model/yolox_nano_lite_416x416_20220214_model.onnx"
ARTIFACTS = MODEL_DIR + "/artifacts.bak"
VIDEO     = "/dev/video2"
SUBDEV    = "/dev/v4l-subdev2"
CAP_W, CAP_H = 960, 540     # VISS/MSC output (what python receives)
NET_W, NET_H = 416, 416
CONF_TH   = 0.40
JPEG_Q    = 70
PORT      = 8080

COCO = ["person","bicycle","car","motorcycle","airplane","bus","train","truck","boat",
"traffic light","fire hydrant","stop sign","parking meter","bench","bird","cat","dog",
"horse","sheep","cow","elephant","bear","zebra","giraffe","backpack","umbrella","handbag",
"tie","suitcase","frisbee","skis","snowboard","sports ball","kite","baseball bat",
"baseball glove","skateboard","surfboard","tennis racket","bottle","wine glass","cup",
"fork","knife","spoon","bowl","banana","apple","sandwich","orange","broccoli","carrot",
"hot dog","pizza","donut","cake","chair","couch","potted plant","bed","dining table",
"toilet","tv","laptop","mouse","remote","keyboard","cell phone","microwave","oven",
"toaster","sink","refrigerator","book","clock","vase","scissors","teddy bear",
"hair drier","toothbrush"]

latest_jpeg = None
jpeg_lock = threading.Lock()
cur_frame = None
frame_lock = threading.Lock()
frame_evt = threading.Event()
running = True


def create_session():
    opts = {"tidl_tools_path": "null", "artifacts_folder": ARTIFACTS}
    try:
        s = ort.InferenceSession(MODEL,
            providers=["TIDLExecutionProvider", "CPUExecutionProvider"],
            provider_options=[opts, {}])
        print("Session OK:", s.get_providers(), flush=True)
        return s, "TIDL(C7x)"
    except Exception as e:
        print("TIDL FAILED, CPU fallback:", str(e)[:120], flush=True)
        return ort.InferenceSession(MODEL, providers=["CPUExecutionProvider"]), "CPU"


def capture_thread():
    global cur_frame, running
    gst = ["gst-launch-1.0","-q",
        "v4l2src","device=" + VIDEO,"io-mode=dmabuf-import",
        "!","video/x-bayer,format=rggb,width=1920,height=1080",
        "!","tiovxisp","sensor-name=SENSOR_SONY_IMX219_RPI",
            "dcc-isp-file=/opt/imaging/imx219/linear/dcc_viss.bin",
            "sink_0::dcc-2a-file=/opt/imaging/imx219/linear/dcc_2a.bin",
            "sink_0::device=" + SUBDEV,
        "!","video/x-raw,format=NV12,width=1920,height=1080",
        "!","tiovxmultiscaler",
        "!","video/x-raw,format=NV12,width=%d,height=%d" % (CAP_W, CAP_H),
        "!","videoconvert","!","video/x-raw,format=BGR",
        "!","fdsink","fd=1"]
    proc = subprocess.Popen(gst, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, bufsize=0)
    fsize = CAP_W*CAP_H*3
    def rd(n):
        b = bytearray()
        while len(b) < n:
            c = proc.stdout.read(n - len(b))
            if not c:
                return None
            b += c
        return bytes(b)
    while running:
        raw = rd(fsize)
        if raw is None:
            print("gst stream ended", flush=True); break
        fr = np.frombuffer(raw, np.uint8).reshape(CAP_H, CAP_W, 3)
        with frame_lock:
            cur_frame = fr
        frame_evt.set()
    proc.terminate()


def infer_thread():
    global latest_jpeg
    sess, mode = create_session()
    iname = sess.get_inputs()[0].name
    sx, sy = CAP_W/NET_W, CAP_H/NET_H
    t_last = time.monotonic(); fps = 0.0
    infer_acc = 0.0; infer_n = 0
    while running:
        frame_evt.wait(timeout=1.0); frame_evt.clear()
        with frame_lock:
            fr = cur_frame
        if fr is None:
            continue
        small = cv2.resize(fr, (NET_W, NET_H))
        tensor = small[:, :, ::-1].transpose(2,0,1)[None].copy()
        t0 = time.monotonic()
        try:
            dets, labels = sess.run(None, {iname: tensor})
        except Exception as e:
            print("infer error:", str(e)[:120], flush=True)
            dets, labels = np.zeros((0,5), np.float32), np.zeros((0,), np.int64)
        infer_ms = (time.monotonic()-t0)*1000
        infer_acc += infer_ms; infer_n += 1
        out = fr.copy()
        n = 0
        for i in range(len(dets)):
            x1,y1,x2,y2,conf = dets[i]
            if conf < CONF_TH: continue
            n += 1
            cls = int(labels[i]) if i < len(labels) else 0
            name = COCO[cls] if 0 <= cls < len(COCO) else str(cls)
            p1 = (int(x1*sx), int(y1*sy)); p2 = (int(x2*sx), int(y2*sy))
            cv2.rectangle(out, p1, p2, (0,255,0), 2)
            cv2.putText(out, "%s %.2f" % (name, conf), (p1[0], max(0,p1[1]-5)),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,255,0), 2)
        now = time.monotonic()
        fps = 0.9*fps + 0.1*(1.0/max(1e-3, now-t_last)); t_last = now
        duty = infer_ms * fps / 10.0  # C7x duty % = infer_ms/period; period=1000/fps
        cv2.putText(out, "IMX219+VPAC+%s  infer %.1fms  %.1fFPS  C7x~%.0f%%  det=%d"
                    % (mode, infer_ms, fps, min(100,duty), n),
                    (8,22), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0,0,255), 2)
        ok, jpg = cv2.imencode(".jpg", out, [cv2.IMWRITE_JPEG_QUALITY, JPEG_Q])
        if ok:
            with jpeg_lock:
                latest_jpeg = jpg.tobytes()
        if infer_n % 60 == 0:
            print("avg_infer=%.1fms fps=%.1f C7x_duty~%.0f%%"
                  % (infer_acc/infer_n, fps, min(100, infer_acc/infer_n*fps/10.0)), flush=True)


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def do_GET(self):
        if self.path in ("/", "/index.html"):
            html = b"<html><body style='background:#111;margin:0'><img src='/stream' style='width:100%'></body></html>"
            self.send_response(200); self.send_header("Content-Type","text/html")
            self.send_header("Content-Length",str(len(html))); self.end_headers()
            self.wfile.write(html); return
        if self.path == "/stream":
            self.send_response(200)
            self.send_header("Content-Type","multipart/x-mixed-replace; boundary=frame")
            self.end_headers()
            last = None
            try:
                while True:
                    with jpeg_lock:
                        buf = latest_jpeg
                    if buf is not None and buf is not last:
                        last = buf
                        self.wfile.write(b"--frame\r\nContent-Type: image/jpeg\r\n")
                        self.wfile.write(("Content-Length: %d\r\n\r\n" % len(buf)).encode())
                        self.wfile.write(buf); self.wfile.write(b"\r\n")
                    else:
                        time.sleep(0.005)
            except (BrokenPipeError, ConnectionResetError):
                return
        self.send_error(404)


if __name__ == "__main__":
    threading.Thread(target=capture_thread, daemon=True).start()
    threading.Thread(target=infer_thread, daemon=True).start()
    print("Serving on http://0.0.0.0:%d/" % PORT, flush=True)
    ThreadingHTTPServer(("0.0.0.0", PORT), Handler).serve_forever()
