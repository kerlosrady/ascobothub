#! /usr/bin/env python
# -*- coding: utf-8 -*-

import rospy

from std_msgs.msg import Float32,Float32MultiArray

from nav_msgs.msg import Path

import tf

import geometry_msgs.msg

#from look_hand.srv import shelf_detection



#rospy.init_node('mission_planner')


#rospy.init_node('mission_planning')
#rospy.Subscriber("detection", object_pos, vision_callback)
#rospy.Subscriber("grip", bool, grip_callback)



class mission_planner():

	def __init__(self):


		self.state=1
		self.sub1=rospy.Subscriber("control_arm", Float32, self.control_arm_callback)
		self.sub2=rospy.Subscriber("grip",Float32, self.grip_callback)
		self.sub3=rospy.Subscriber("can_detected",Path, self.can_detection_callback)
		#self.msgcamera=Path()
		#self.tf = TransformListener()
		#rospy.Subscriber("control_base", base_states, control_base_callback)
		self.pubr = rospy.Publisher('arm_action_r',geometry_msgs.msg.Pose, queue_size=10)
		self.publ = rospy.Publisher('arm_action_l',geometry_msgs.msg.Pose, queue_size=10)
		self.pub2= rospy.Publisher('gripper', Float32, queue_size=10)
		self.pub3= rospy.Publisher('chatter_1',Float32, queue_size=10)


		self.pub4= rospy.Publisher('camera_pos', Float32, queue_size=10)
		self.pub5= rospy.Publisher('can_detection', Float32, queue_size=10)

		self.cans_detected = False
		rate = rospy.Rate(1) # 10hz

	
		

		while not rospy.is_shutdown():

			# if state==0 and table_depth >= threshold:
			#         #publish certain action to get back
			#         state=1
			#         msgb= base_data()
			#         msgb.loc= heading_pos
			#         pub1.publish(msgb)g

			#if state==1 and table_depth - threshold >=0.1:
			if self.state==1:
				#msgb = base_data()
				#msgb.stop =True
				#pub1.publish(msgb)
				self.state =2
				#print("pub3")
				rospy.sleep(1)
				self.forward= 5.0
				self.pub3.publish(self.forward)
				
			
			if self.state==2:
				self.pub3.publish(6)
				self.state=3
				print(self.state)


			if self.state==3 and self.cans_detected is True:
				self.state = 4
				self.execute_state=1
				cycle= cycle+1

				# if execute_state==1:
				while (done is not True):

							
					if self.execute_state ==1 :
						

						
						self.point1= PointStamped()
						self.point2= PointStamped()

						self.pose1=PoseStamped()
						self.pose2=PoseStamped()

						self.point1.header = self.msgcamera_id
						self.point2.header = self.msgcamera_id
						self.pose1.header = self.msgcamera_id
						self.pose2.header = self.msgcamera_id

						self.poset1Tr =transformPose("/base_link",pose1)
						self.poset2Tr= transformPose("/base_link",pose2)
						
						self.point1Tr =transformPoint("/base_link",point1)
						self.point2Tr= transformPoint("/base_link",point2)

						#arm 1
		
						pose_goal1 = geometry_msgs.msg.Pose()
					    	pose_goal1.orientation.w =1
					    	pose_goal1.position.x = point1Tr.point.x
					    	pose_goal1.position.y = point1Tr.point.y
					    	pose_goal1.position.z = point1Tr.point.z 
					    	pose_goal1.orientation.x =0
					    	pose_goal1.orientation.y =0
					    	pose_goal1.orientation.z =1

						self.pubr.publish(pose_goal1)
						

						#arm 2
						
						pose_goal2 = geometry_msgs.msg.Pose()
					    	pose_goal2.orientation.w =1
					    	pose_goal2.position.x = point2Tr.point.x
					    	pose_goal2.position.y = point2Tr.point.y
					    	pose_goal2.position.z = point2Tr.point.z 
					    	pose_goal2.orientation.x =0
					    	pose_goal2.orientation.y =0
					    	pose_goal2.orientation.z =1

						self.publ.publish(pose_goal2)
		
		
					if self.execute_state == 2 and self.reach_target == True:
						self.execute_state = 3
						self.reach_target= False
						self.pub2.publish(True)

						# rospy.wait_for_service('can_detection')

						# try:
						# 	can_pos = rospy.ServiceProxy('grip', grip)
						# 	resp1 = grip(1)
							
						# 	grip_target= resp1


						# except rospy.ServiceException as e:
						# 	print("Service call failed: %s" % e)
					

					if self.execute_state == 2 and grip_target == True:
							
						z1= z1+0.1
						z2= z2=0.1
						msg1 = Float32MultiArray
						msg1 = [x1, y1, z1,x2,y2,z2]
						execute_state = 3
						grip_target= False
						done= True
			#rospy.spin()




	def control_arm_callback(self,data):

		if data.data is True:
			self.reach_target= True

	def can_detection_callback(self,data):
		#can1_posx= data.data[0]
		#can1_posy= data.data[1]
		#can1_posz= data.data[2]

		#can2_posx= data.data[3]
		#can2_posy= data.data[4]
		#can2_posz= data.data[5]
		if self.state==3:
			self.cans_detected= True
			print(self.cans_detected)
			self.msgcamera_id= data.header.frame_id
			self.msgcamera_poses =data.poses
			print(data.poses)
		
		
		
		
		
		
            	#print position, quaternion
		
		


	def grip_callback(self,data):
		if data.data is True:
			grip_target= True


if __name__=='__main__':

	rospy.init_node("mission_planner")

	
	try:
     		mission_planner()
     	except rospy.ROSInterruptException:
     		pass


	






	

