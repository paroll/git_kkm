from imutils.video import VideoStream
from imutils.video import FPS
import os
import imutils
import time
import cv2

print("[INFO] capture and data transfer to server")
detector=cv2.CascadeClassifier("haarcascade_frontalface_default.xml")
print("[INFO] starting video stream...")
vs=VideoStream(src=0).start()
time.sleep(2.0)
fps=FPS().start()
i=0
pwd='./images'
number=[]
for path,dirs,files in os.walk(pwd):
	for file in files:
		file=file[:-4]
		number.append(int(file))
	number.sort(reverse=True)
	i=number[0]+1
while True:
    frame=vs.read()
    frame=imutils.resize(frame,width=500)
    
    gray=cv2.cvtColor(frame,cv2.COLOR_BGR2GRAY)
    rects=detector.detectMultiScale(gray, scaleFactor=1.1,minNeighbors=5, minSize=(30, 30),flags=cv2.CASCADE_SCALE_IMAGE)
    for (x,y,w,h) in rects:
        cv2.imwrite("images/%d.png"%i,frame[max(0,y-20):y+h+20,max(0,x-20):x+w+20],params=[cv2.IMWRITE_PNG_COMPRESSION,0])
        i+=1
    cv2.imshow("sdfs",frame)
    key=cv2.waitKey(1) & 0xFF
    if key == ord("q"):
        break
    fps.update()
fps.stop()
cv2.destroyAllWindows()
vs.stop()
