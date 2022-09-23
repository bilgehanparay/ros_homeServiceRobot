#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <nav_msgs/Odometry.h>
// pickup-dropoff zones
float pickup_x = 1;
float pickup_y = 0;
float pickup_w = 1.0;

float dropoff_x = -10;
float dropoff_y = 0;
float dropoff_w = 1.2;

enum eRoboState{
  ROBOT_MOV_TO_PICKUP,
  ROBOT_AT_PICKUP,
  ROBOT_TO_DROPOFF,
  ROBOT_AT_DROPOFF
};
eRoboState robotState = ROBOT_MOV_TO_PICKUP;

// handle marker position
void marker_odom_callback(const nav_msgs::Odometry od){
  float curr_x = od.pose.pose.position.x;
  float curr_y = od.pose.pose.position.y;
  float curr_w = od.pose.pose.orientation.w;

  float dPickupX = fabs(curr_x - pickup_x);
  float dPickupY = fabs(curr_y - pickup_y);
  float dPickUp = sqrt(dPickupX*dPickupX + dPickupY*dPickupY);
  float dPickupW = fabs(curr_w - pickup_w);

  float dDropOfX = fabs(curr_x - dropoff_x);
  float dDropOfY = fabs(curr_y - dropoff_y);
  float dDropOf = sqrt(dDropOfX*dDropOfX + dDropOfY*dDropOfY);
  float dDropOfW = fabs(curr_w - dropoff_w);
  float dThreshold = 0.5;
  if(robotState == ROBOT_MOV_TO_PICKUP && (dPickUp >= dThreshold)){
    robotState = ROBOT_MOV_TO_PICKUP;
  }else if(robotState == ROBOT_MOV_TO_PICKUP && dPickUp < dThreshold){
    robotState = ROBOT_AT_PICKUP;
  }else if(robotState == ROBOT_AT_PICKUP && dPickUp < dThreshold){
    robotState = ROBOT_AT_PICKUP;
  }else if(robotState == ROBOT_AT_PICKUP && (dPickUp >= dThreshold)){
    robotState = ROBOT_TO_DROPOFF;
  }else if(robotState == ROBOT_TO_DROPOFF && (dDropOf >= dThreshold)){
    robotState = ROBOT_TO_DROPOFF;
    // ROS_INFO("odom_x %f, dropoff_x %f", curr_x, dropoff_x);
  }else if(robotState == ROBOT_TO_DROPOFF && dDropOf < dThreshold){
    robotState = ROBOT_AT_DROPOFF;
  }else{
    robotState = ROBOT_AT_DROPOFF; // should never be the case anyway
  }
}

int main( int argc, char** argv ){
  ros::init(argc, argv, "add_markers");

  ros::NodeHandle n;
  ros::Rate r(1);
  // marker publisher
  ros::Publisher marker_pub = n.advertise<visualization_msgs::Marker>("visualization_marker", 1);
  ros::Subscriber sub1 = n.subscribe("/odom", 10, marker_odom_callback);

  // Set our initial shape type to be a cube
  uint32_t shape = visualization_msgs::Marker::CUBE;

  visualization_msgs::Marker marker;
  // Set the frame ID and timestamp.  See the TF tutorials for information on these.
  marker.header.frame_id = "map";
  marker.header.stamp = ros::Time::now();

  // Set the namespace and id for this marker.  This serves to create a unique ID
  // Any marker sent with the same namespace and id will overwrite the old one
  marker.ns = "basic_shapes";
  marker.id = 0;

  // Set the marker type.  Initially this is CUBE, and cycles between that and SPHERE, ARROW, and CYLINDER
  marker.type = shape;

  // Set the marker action.  Options are ADD, DELETE, and new in ROS Indigo: 3 (DELETEALL)
  marker.action = visualization_msgs::Marker::ADD;

  // Set the pose of the marker.  This is a full 6DOF pose relative to the frame/time specified in the header
  marker.pose.position.x = pickup_x;
  marker.pose.position.y = pickup_y;
  marker.pose.position.z = 0;
  marker.pose.orientation.x = 0.0;
  marker.pose.orientation.y = 0.0;
  marker.pose.orientation.z = 0.0;
  marker.pose.orientation.w = pickup_w;

  // Set the scale of the marker -- 1x1x1 here means 1m on a side
  marker.scale.x = 0.25;
  marker.scale.y = 0.25;
  marker.scale.z = 0.25;

  // Set the color -- be sure to set alpha to something non-zero!
  marker.color.r = 0.0f;
  marker.color.g = 1.0f;
  marker.color.b = 0.0f;
  marker.color.a = 1.0;

  marker.lifetime = ros::Duration();
  while (marker_pub.getNumSubscribers() < 1){
    if (!ros::ok())
    {
      return 0;
    }
    ROS_WARN_ONCE("Please create a subscriber to the marker");
    sleep(1);
  }

  while(ros::ok){
    
    switch(robotState){
      case ROBOT_MOV_TO_PICKUP:
        marker_pub.publish(marker);
        ROS_INFO("to pickup");
      break;

      case ROBOT_AT_PICKUP:
        marker.action = visualization_msgs::Marker::DELETE;
        marker_pub.publish(marker);
        ROS_INFO("at pickup");
      break;

      case ROBOT_AT_DROPOFF:
        marker.header.frame_id = "map";
        marker.header.stamp = ros::Time::now();

        // Set the namespace and id for this marker.  This serves to create a unique ID
        // Any marker sent with the same namespace and id will overwrite the old one
        marker.ns = "basic_shapes";
        marker.id = 0;

        // Set the marker type.  Initially this is CUBE, and cycles between that and SPHERE, ARROW, and CYLINDER
        marker.type = shape;

        // Set the marker action.  Options are ADD, DELETE, and new in ROS Indigo: 3 (DELETEALL)
        marker.action = visualization_msgs::Marker::ADD;

        // Set the pose of the marker.  This is a full 6DOF pose relative to the frame/time specified in the header
        marker.pose.position.x = dropoff_x;
        marker.pose.position.y = dropoff_y;
        marker.pose.position.z = 0;
        marker.pose.orientation.x = 0.0;
        marker.pose.orientation.y = 0.0;
        marker.pose.orientation.z = 0.0;
        marker.pose.orientation.w = dropoff_w; 
        marker_pub.publish(marker); 
        ROS_INFO("at dropof");  
      break;

      case ROBOT_TO_DROPOFF:
        // marker_pub.publish(marker);
        ROS_INFO("to dropof");
      break;
    }
    ros::spinOnce();
    ros::Duration(1).sleep();
  }

  ros::spin();
}





