#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser:: DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;
    if (!client.call(srv))
    {
        ROS_ERROR("Failed to call service DriveToTarget");
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    int l_cnt = 0;
    int r_cnt = 0;
    int m_cnt = 0;
    for(int i = 0;i < img.height * img.step; i += 3)
    {
        if ((img.data[i] == white_pixel) && (img.data[i+1] == white_pixel) && (img.data[i+2] == white_pixel))
        {
            int position = (i % img.step) / 3;
            if (position <  img.width / 3)
                l_cnt++;
            else if (position >  img.width * 2 / 3)
                r_cnt++;
            else
                m_cnt++;
        }
    }

    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    if ((l_cnt > r_cnt) && (l_cnt > m_cnt))
    {
        drive_robot(0.0, 0.5);  //turn left
    }
    else if ((r_cnt > l_cnt) && (r_cnt > m_cnt))
    {
        drive_robot(0.0, -0.5);   //turn right
    }
    else if ((m_cnt > l_cnt) && (r_cnt < m_cnt))
    {
        drive_robot(0.5, 0.0);  //move forward
    }
    else
    {
        drive_robot(0.0, 0.0);
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
