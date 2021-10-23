/*
 * Software License Agreement (Modified BSD License)
 *
 *  Copyright (c) 2013, PAL Robotics, S.L.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of PAL Robotics, S.L. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/** \author Jordi Pages. */

/**
 * @file
 *
 * @brief example on how to subscribe to an image topic and how to make the robot look towards a given direction
 *
 * How to test this application:
 *
 * 1) Launch the application:
 *
 *   $ rosrun tiago_tutorials look_to_point
 *
 * 2) Click on image pixels to make TIAGo look towards that direction
 *
 */

// C++ standard headers
#include <exception>
#include <string>
#include <array>
#include <iostream>
#include <math.h>

// Boost headers
#include <boost/shared_ptr.hpp>

// ROS headers
#include <ros/ros.h>
#include <message_filters/time_synchronizer.h>
#include <message_filters/subscriber.h>
#include <image_transport/image_transport.h>
#include <actionlib/client/simple_action_client.h>
#include <sensor_msgs/CameraInfo.h>
#include <geometry_msgs/PointStamped.h>
#include <control_msgs/PointHeadAction.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/Image.h>
#include <ros/topic.h>

// OpenCV headers

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <cv_bridge/cv_bridge.h>

using namespace cv;
using namespace std;
using namespace sensor_msgs;
using namespace message_filters;

typedef union U_FloatParse {
    float float_data;
    unsigned char byte_data[4];
} U_FloatConvert;



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static const std::string graywindowName  = "Gray Image";
static const std::string cameraFrame     = "/xtion_rgb_optical_frame";   
static const std::string imageTopic      = "/xtion/rgb/image_raw";
static const std::string depthImageTopic = "/xtion/depth_registered/image_raw";
static const std::string cameraInfoTopic = "/xtion/rgb/camera_info";

// Camera images
cv_bridge::CvImagePtr cvImgPtr1;
cv_bridge::CvImagePtr cvImgPtr2;

// Intrinsic parameters of the camera
cv::Mat cameraIntrinsics;
cv::Mat tempImg = cv::imread('/home/user/ws/src/ascobothub/look_to_point/src/AH_can_label.png');
ros::Time latestImageStamp;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// My Function of detecting the top of cans

int match(cv::Mat sourceImg)
{
    cvtColor(sourceImg, sourceImg, CV_BGR2GRAY);
    cvtColor(tempImg, tempImg, CV_BGR2GRAY);

    cv::Mat sourceImgCanny;
    cv::Mat tempImgCanny;

    const int low_canny = 110;
    Canny(sourceImg, sourceImgCanny, low_canny, low_canny*3);
    Canny(tempImg, tempImgCanny, low_canny, low_canny*3);

    imshow("source", sourceImg);
    imshow("template", sourceImg);

    Mat res_32f(sourceImg.rows - tempImg.rows + 1, sourceImg.cols - tempImg.cols + 1, CV_32FC1);

    matchTemplate(sourceImg, tempImg, res_32f, CV_TM_CCOEFF_NORMED);

    Mat res;
    res_32f.convertTo(res, CV_8U, 255.0);
    imshow("result", res);

    int size = ((tempImg.cols + tempImg.rows) / 4) * 2 + 1; //force size to be odd
    adaptiveThreshold(res, res, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, size, -64);
    imshow("result_thresh", res);

    while(1) 
    {
        double minval, maxval;
        Point minloc, maxloc;
        minMaxLoc(res, &minval, &maxval, &minloc, &maxloc);

        if(maxval > 0)
        {
            rectangle(sourceImg, maxloc, Point(maxloc.x + tempImg.cols, maxloc.y + tempImg.rows), Scalar(0,255,0), 2);
            floodFill(res, maxloc, 0); //mark drawn blob
        }
        else
            break;
    }

    imshow("final", sourceImg);
    waitKey(0);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ROS call back for every new image received
void callback(const sensor_msgs::ImageConstPtr& imgMsg, const sensor_msgs::ImageConstPtr& depthImgMsg) 
{
  latestImageStamp = imgMsg->header.stamp;
  cvImgPtr1 = cv_bridge::toCvCopy(imgMsg, sensor_msgs::image_encodings::BGR8);
  cvImgPtr2 = cv_bridge::toCvCopy(depthImgMsg, sensor_msgs::image_encodings::TYPE_32FC1);
  cv::imshow("RGB",cvImgPtr1->image);
  cv::imshow("Depth",cvImgPtr2->image);
  int c = match(cvImgPtr1->image);
  cv::waitKey(15);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Entry point
int main(int argc, char** argv)
{
  // Initialize the ROS node
  ros::init(argc, argv, "Vision");

  ROS_INFO("Starting Vision application ...");
 
 //1st NodeHandle does the initialization,last one will cleanup any resources the node was using.   
 ros::NodeHandle nh;

  if (!ros::Time::waitForValid(ros::WallDuration(10.0))) // NOTE: Important when using simulated clock
  {
    ROS_FATAL("Timed-out waiting for valid time.");
    return EXIT_FAILURE;
  }

  // Get the camera intrinsic parameters from the appropriate ROS topic
  ROS_INFO("Waiting for camera intrinsics ... ");
  sensor_msgs::CameraInfoConstPtr msg = ros::topic::waitForMessage
      <sensor_msgs::CameraInfo>(cameraInfoTopic, ros::Duration(10.0));

  if(msg.use_count() > 0)
  {
    cameraIntrinsics = cv::Mat::zeros(3,3,CV_64F);
    cameraIntrinsics.at<double>(0, 0) = msg->K[0]; //fx
    cameraIntrinsics.at<double>(1, 1) = msg->K[4]; //fy
    cameraIntrinsics.at<double>(0, 2) = msg->K[2]; //cx
    cameraIntrinsics.at<double>(1, 2) = msg->K[5]; //cy
    cameraIntrinsics.at<double>(2, 2) = 1;
  }
  
  // Define ROS topic from where TIAGo publishes images
  // use compressed image transport to use less network bandwidth
  ROS_INFO_STREAM("Subscribing ");

  message_filters::Subscriber<Image> image_sub(nh,imageTopic, 1);
  message_filters::Subscriber<Image> depth_sub(nh,depthImageTopic, 1);
  TimeSynchronizer<sensor_msgs::Image,sensor_msgs::Image> sync(image_sub, depth_sub, 10);
  sync.registerCallback(boost::bind(&callback, _1, _2));

  ROS_INFO_STREAM("Done Subscribing");

  ros::spin();

  return EXIT_SUCCESS;
}
