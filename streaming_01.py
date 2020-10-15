from imutils.video import VideoStream
from imutils.video import FPS
import os
import imutils
import time
import cv2

print("[INFO] capture and streaming to server")
print("[INFO] starting video stream...")
vs=VideoStream(src=0).start()
time.sleep(2.0)
fps=FPS().start()
frame=vs.read()
cv2.imshow('streaming',frame)
while True:

    key=cv2.waitKey(1) & 0xFF
    if key == ord("q"):
        break
    fps.update()
    frame=vs.read()
fps.stop()
cv2.destroyAllWindows()
vs.stop()
