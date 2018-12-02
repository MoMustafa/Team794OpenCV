import cv2 as cv
from datetime import datetime

font = cv.FONT_HERSHEY_SIMPLEX
scale = 0.5
color = (0,0,0)
bold = 1

scaling_factor=1

facefrontal = cv.CascadeClassifier('.\Cascades\haarcascades\haarcascade_frontalface_alt.xml')
faceprofile = cv.CascadeClassifier('.\Cascades\haarcascades\haarcascade_profileface.xml')
body = cv.CascadeClassifier('.\Cascades\haarcascades\haarcascade_upperbody.xml')

cap = cv.VideoCapture(0)

if not cap.isOpened():
    print('Camera Startup Error')
    exit

fwid = int(cap.get(3)*scaling_factor)
fhgt = int(cap.get(4)*scaling_factor)

recording = cv.VideoWriter('recording.avi', cv.VideoWriter_fourcc('M','J','P','G'), 10, (fwid,fhgt))

while(True):
    ret, frame = cap.read()
    
    frame = cv.resize(frame, None, fx=scaling_factor, fy=scaling_factor, interpolation=cv.INTER_AREA)
    
    frame_gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)
    frame_gray = cv.equalizeHist(frame_gray)
    
    face_fronts = facefrontal.detectMultiScale(frame_gray, scaleFactor=1.3, minNeighbors=3)
    face_sides = faceprofile.detectMultiScale(frame_gray, scaleFactor=1.3, minNeighbors=3)
    body_dect = body.detectMultiScale(frame_gray, scaleFactor=1.3, minNeighbors=3)
    for(x,y,w,h) in face_fronts:
        cv.rectangle(frame, (x,y), (x+w,y+h), (0,255,0), 3)
    for(x,y,w,h) in face_sides:
        cv.rectangle(frame, (x,y), (x+w,y+h), (255,0,0), 3)
    for(x,y,w,h) in body_dect:
        cv.rectangle(frame, (x,y), (x+w,y+h), (0,0,255), 3)
    
    cv.putText(frame,str(datetime.now()),(int(fwid/20),int(fhgt/20)),font,scale,color,bold,cv.LINE_AA)
    cv.imshow('frame', frame)
    #recording.write(frame)
    
    if cv.waitKey(1)&0xFF == ord('q'):
        break
cap.release()
cv.destroyAllWindows()
    
    
    

