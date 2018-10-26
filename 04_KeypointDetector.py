import cv2 as cv

#SETTINGS
history = 20        #How many frames to compare with before calling it motion
threshold = 500     #How far pixels have to move before calling it motion
shadows = False     #Also applies to reflections (I think)
minkeypoints = 5    #How many moving pixels justify shifting the camera
frameupdate = 10    #After how many frames to check if camera shift is necessary

red = (0,0,255)
green = (0,255,0)

w = 100              #width of rectangle   
h = 100              #height of rectangle
thickness = 3

#VARIABLES
timer = 0;          #Counts every frame
centroid = (0,0)    #Point that Camera will be centered on

#Initialise Video
video = cv.VideoCapture(0)
#Subtract static portion of video
fgbg = cv.createBackgroundSubtractorMOG2(history, threshold, shadows)

#Report Camera Problem
if not video.isOpened():
    print('Camera Startup Error')
    exit

#Read every frame until Q is pressed
while(True):
    timer += 1
    ret, frame  = video.read()
    gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)
    fgmask = fgbg.apply(gray)
    
    fast = cv.FastFeatureDetector_create()
    
    brief = cv.xfeatures2d.BriefDescriptorExtractor_create()
    
    keypoints = fast.detect(fgmask, None)
    
    keypoints, descriptors = brief.compute(fgmask, keypoints)
    
    if len(keypoints)>minkeypoints and timer%frameupdate==0:
        x = [k.pt[0] for k in keypoints]
        y = [k.pt[1] for k in keypoints]
        centroid = (int(sum(x) / len(keypoints)), 
                    int(sum(y) / len(keypoints)))
    
    #cv.circle(frame, centroid, 5, (0,0,255), -1)  
    topcorner = centroid[0]-w, centroid[1]+h
    botcorner = centroid[0]+w, centroid[1]-h 
    
    cv.rectangle(frame, topcorner, botcorner, red, thickness)
    
    cv.drawKeypoints(frame, keypoints, frame, color=green)
    
    cv.imshow('frame', frame)
    if cv.waitKey(1)&0xFF == ord('q'):
        break
    
video.release()
cv.destroyAllWindows()