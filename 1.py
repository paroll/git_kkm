import cv2
CAM_ID = 0

cap = cv2.VideoCapture(CAM_ID) 

if cap.isOpened() == False:
    print( 'Can not open the CAM(%d)' % (CAM_ID))
    exit()

cv2.namedWindow('Face')

face_cascade = cv2.CascadeClassifier()
face_cascade.load('/usr/local/share/opencv4/haarcascades/haarcascade_frontalface_default.xml')


while(True):

    ret, frame = cap.read()

    grayframe = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    grayframe = cv2.equalizeHist(grayframe)

   #faces = face_cascade.detectMultiScale(grayframe, 1.1, 3, 0, (30, 30))
    faces = face_cascade.detectMultiScale(grayframe, 1.5)

    for (x,y,w,h) in faces:
        cv2.rectangle(frame,(x,y),(x+w,y+h),(0,255,0),3, 4, 0)

    cv2.imshow('Face',frame)

    if cv2.waitKey(10) >= 0:
        break;

cap.release()
cv2.destroyWindow('Face')
