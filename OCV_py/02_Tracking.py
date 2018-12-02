# =============================================================================
# Object Tracking 
# =============================================================================

import cv2 as cv

cap = cv.VideoCapture(0)

fgbg = cv.createBackgroundSubtractorMOG2()

while(True):
    ret, frame = cap.read()
    
    fgmask = fgbg.apply(frame)
    
    cv.imshow('frame', fgmask)
    if cv.waitKey(1)&0xFF == ord('q'):
        break
    
cap.release()
cv.destroyAllWindows()