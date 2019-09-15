# https://picamera.readthedocs.io/en/release-1.10/api_camera.html


import time
import picamera
import argparse

#imports for moving files
import shutil
import os

#variables for copying images
source = '/var/www/html/img/'
dest = '/mnt/share' #replace with mount location

# parse arguments
parser = argparse.ArgumentParser(description='Take pic at waypoint')
parser.add_argument('waypoint', metavar='W', type=int, nargs='+', \
                    help='waypoint number')
args = parser.parse_args()
waypoint = int(args.waypoint[0])
if waypoint <= 0:
    waypoint = 0
else:
    waypoint = waypoint - 1
'''
waypoint = str(args.waypoint)
waypoint = waypoint.replace('[','')
waypoint = waypoint.replace(']','')'''
print(waypoint)
print('got here')

with picamera.PiCamera() as camera:

    camera.resolution = (3280, 2464)
    
    # dark image settings
    camera.brightness = 22
    #camera.iso = 200
    camera.exposure_mode = 'sports'
    camera.contrast = 100
    camera.shutter_speed = 3200
    time.sleep(2)

    
    #camera.start_preview()
    try:
        for i, filename in enumerate(camera.capture_continuous('/var/www/html/img/bot{counter:d}.jpg')):
            # print(filename)
            time.sleep(0.6) # time between images
            if i == waypoint:
                break
    finally:
	#move all images to the mnt path
	files = os.listdir(source)
	for f in files:
		shutil.move(source+f,dest)
        print('done')
