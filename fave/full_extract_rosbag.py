#!/usr/bin/env python

import rospy
from rds_network_ros.msg import ToGui
from geometry_msgs.msg import PoseWithCovarianceStamped
import tf
import numpy as np
import time
import scipy.io as sio
import signal
import sys

n_max = 30*300 # 30 Hz for 5 min

commands_data = np.empty([n_max, 5])
# [[time, nominal_v, nominal_w, corrected_v, corrected_w]]
lrf_objects_data = np.empty([n_max, 4000]) # store xy for 2000 objects
lrf_objects_data[:] = np.nan
tracker_objects_data = np.empty([n_max, 100]) # store xy and xy_pred for 25 objects
tracker_objects_data[:] = np.nan

pose_data = np.empty([n_max, 4])
# [[time, x, y, phi]]

counter = 0
start_time = None

def save():
	global commands_data
	global lrf_objects_data
	global tracker_objects_data
	global pose_data
	commands_data = commands_data[0:counter, :]
	lrf_objects_data = lrf_objects_data[0:counter, :]
	tracker_objects_data = tracker_objects_data[0:counter, :]
	pose_data = pose_data[0:counter, :]
	sio.savemat('command.mat', {'data' : commands_data})
	sio.savemat('tracker_object.mat', {'data' : tracker_objects_data})
	sio.savemat('lrf_object.mat', {'data' : lrf_objects_data})
	sio.savemat('pose.mat', {'data' : pose_data})

def signal_handler(sig, frame):
	save()
	sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

tf_listener = None

def get_pose():
   global tf_listener
   (trans, rot) = tf_listener.lookupTransform('/tf_qolo_world', '/tf_rds', rospy.Time(0))
   rpy = tf.transformations.euler_from_quaternion(rot)
   print ("phi=", rpy[2])
   return (trans[0], trans[1], rpy[2])

def callbackToGui(msg):
	global commands_data
	global lrf_objects_data
	global tracker_objects_data
	global counter
	global start_time
	global pose_data

	try:
		(x, y, phi) = get_pose()
	except (tf.LookupException, tf.ConnectivityException, tf.ExtrapolationException):
		(x, y, phi) = (np.nan, np.nan, np.nan)

	nominal_v = msg.nominal_command.linear
	nominal_w = msg.nominal_command.angular
	corrected_v = msg.corrected_command.linear
	corrected_w = msg.corrected_command.angular

	if start_time == None:
		start_time = rospy.Time.now() #time.time()
		t = 0.0
	else:
		t = (rospy.Time.now() - start_time).to_sec() #time.time() - start_time

	if counter >= n_max:
		print ('Longer than expected')
		return
	commands_data[counter, :] = np.array([t, nominal_v, nominal_w, corrected_v, corrected_w])

	i_tracks = 0
	for i in range(len(msg.moving_objects)):
		if msg.moving_objects[i].radius < 0.1:
			if i*2 + 1 >= lrf_objects_data.shape[1]:
				print('More lrf-objects than expected')
			else:
				lrf_objects_data[counter, i*2] = msg.moving_objects[i].center.x
				lrf_objects_data[counter, i*2 + 1] = msg.moving_objects[i].center.y
		else:
			i_tracks = i
			break
	for i in range(i_tracks, len(msg.moving_objects)):
		x_track = msg.moving_objects[i].center.x
		y_track = msg.moving_objects[i].center.y
		i_0ed = i - i_tracks
		if i_0ed*4 + 3 >= tracker_objects_data.shape[1]:
			print('More tracker-objects than expected')
			break
		tracker_objects_data[counter, i_0ed*4] = x_track
		tracker_objects_data[counter, i_0ed*4 + 1] = y_track

	pose_data[counter, :] = np.array([t, x, y, phi])

	counter += 1

def main():
	global tf_listener
	rospy.init_node('extract_rosbag_command_object_node')
	tf_listener = tf.TransformListener()
	rospy.Subscriber('rds_to_gui', ToGui, callbackToGui)
	print ('Ready ...')
	rospy.spin()

if __name__ == '__main__':
	main()