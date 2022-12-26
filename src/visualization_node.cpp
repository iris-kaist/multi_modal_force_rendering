// Visualize the virtual wall, guidance, force, HIP with RViZ

#include <ros/ros.h>

#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/transform_listener.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Transform.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/PoseStamped.h>

#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <phantom_premium_msgs/TeleoperationDeviceStateStamped.h>

/**
 * This tutorial demonstrates simple receipt of messages over the ROS system.
 */
ros::Subscriber subInterfaceState_;

ros::Publisher pubEEMaker_;
ros::Publisher pubVGMaker_;
ros::Publisher pubVWLeftMaker_;
ros::Publisher pubVWRightMaker_;

phantom_premium_msgs::TeleoperationDeviceStateStamped msg_;

visualization_msgs::Marker ee_marker_;
visualization_msgs::Marker vg_marker_;
visualization_msgs::Marker vw_left_marker_;
visualization_msgs::Marker vw_right_marker_;

geometry_msgs::TransformStamped tf_stamped_;
phantom_premium_msgs::TeleoperationDeviceStateStamped interface_;

geometry_msgs::PoseStamped ee_pose_stamed_;

double vw_width_= 0.0;

int publish_rate_;

struct Virtual_Wall{
	double px = 0.0;
	double py = 0.0;
	double pz = 0.0;
	double kp = 0.0;
	double kd = 0.0;
};
Virtual_Wall vw_left_;
Virtual_Wall vw_right_;


struct Virtual_Guidance{
	double px = 0.0;
	double py = 0.0;
	double pz = 0.0;
	double kp = 0.0;
	double kd = 0.0;
};
Virtual_Guidance vg_;

void VirtualMarkerInit(visualization_msgs::Marker *virtual_wall)
{
	virtual_wall->header.frame_id = "base_link";
	virtual_wall->header.stamp = ros::Time();
	virtual_wall->ns = "virtual_wall";
	virtual_wall->id = 0;
	virtual_wall->type = visualization_msgs::Marker::CUBE;
	virtual_wall->action = visualization_msgs::Marker::ADD;
	virtual_wall->pose.position.x = 0;
	virtual_wall->pose.position.y = 0;
	virtual_wall->pose.position.z = 0;
	virtual_wall->pose.orientation.x = 0.0;
	virtual_wall->pose.orientation.y = 0.0;
	virtual_wall->pose.orientation.z = 0.0;
	virtual_wall->pose.orientation.w = 1.0;
	virtual_wall->scale.x = 0.1;
	virtual_wall->scale.y = 0.1;
	virtual_wall->scale.z = 0.1;
	virtual_wall->color.a = 1.0; // Don't forget to set the alpha!
	virtual_wall->color.r = 1.0;
	virtual_wall->color.g = 0.0;
	virtual_wall->color.b = 0.0;
	//only if using a MESH_RESOURCE marker type:
	// marker_mesh_resource = "package://pr2_description/meshes/base_v0/base.dae";
}


void ForceMarkerInit(visualization_msgs::Marker *force)
{
	force->header.frame_id = "base_link";
	force->header.stamp = ros::Time();
	force->ns = "force_feedback";
	force->id = 0;
	force->type = visualization_msgs::Marker::CUBE;
	force->action = visualization_msgs::Marker::ADD;
	force->pose.position.x = 0;
	force->pose.position.y = 0;
	force->pose.position.z = 0;
	force->pose.orientation.x = 0.0;
	force->pose.orientation.y = 0.0;
	force->pose.orientation.z = 0.0;
	force->pose.orientation.w = 1.0;
	force->scale.x = 0.1;
	force->scale.y = 0.1;
	force->scale.z = 0.1;
	force->color.a = 1.0; // Don't forget to set the alpha!
	force->color.r = 1.0;
	force->color.g = 0.0;
	force->color.b = 0.0;
	//only if using a MESH_RESOURCE marker type:
	// marker_mesh_resource = "package://pr2_description/meshes/base_v0/base.dae";
}

void EEMarkerInit()
{
	ee_marker_.header.frame_id = "master";
	ee_marker_.header.stamp = ros::Time();
	ee_marker_.ns = "ee_marker";
	ee_marker_.id = 0;
	ee_marker_.type = visualization_msgs::Marker::SPHERE;
	ee_marker_.action = visualization_msgs::Marker::ADD;
	ee_marker_.pose.position.x = 0;
	ee_marker_.pose.position.y = 0;
	ee_marker_.pose.position.z = 0;
	ee_marker_.pose.orientation.x = 0.0;
	ee_marker_.pose.orientation.y = 0.0;
	ee_marker_.pose.orientation.z = 0.0;
	ee_marker_.pose.orientation.w = 1.0;
	ee_marker_.scale.x = 0.05;
	ee_marker_.scale.y = 0.05;
	ee_marker_.scale.z = 0.05;
	ee_marker_.color.a = 1.0; // Don't forget to set the alpha!
	ee_marker_.color.r = 0.0;
	ee_marker_.color.g = 1.0;
	ee_marker_.color.b = 0.0;
	//only if using a MESH_RESOURCE marker type:
	// marker_mesh_resource = "package://pr2_description/meshes/base_v0/base.dae";
}

void VGMarkerInit()
{
	vg_marker_.header.frame_id = "base_link";
	vg_marker_.header.stamp = ros::Time();
	vg_marker_.ns = "vg_marker";
	vg_marker_.id = 0;
	vg_marker_.type = visualization_msgs::Marker::LINE_STRIP;
	vg_marker_.action = visualization_msgs::Marker::ADD;
	vg_marker_.pose.position.x = 0;
	vg_marker_.pose.position.y = 0;
	vg_marker_.pose.position.z = 0;
	vg_marker_.pose.orientation.x = 0.0;
	vg_marker_.pose.orientation.y = 0.0;
	vg_marker_.pose.orientation.z = 0.0;
	vg_marker_.pose.orientation.w = 1.0;
	vg_marker_.scale.x = 0.02;
	vg_marker_.scale.y = 0.1;
	vg_marker_.scale.z = 0.1;
	vg_marker_.color.a = 1.0; // Don't forget to set the alpha!
	vg_marker_.color.r = 0.0;
	vg_marker_.color.g = 0.0;
	vg_marker_.color.b = 1.0;
	//only if using a MESH_RESOURCE marker type:
	// marker_mesh_resource = "package://pr2_description/meshes/base_v0/base.dae";

	geometry_msgs::Point p1, p2;
	p1.x = vg_.px;
	p1.y = vg_.py;
	p1.z = vg_.pz - 0.5;

	p2.x = vg_.px;
	p2.y = vg_.py;
	p2.z = vg_.pz + 0.5;

	vg_marker_.points.push_back(p1);
	vg_marker_.points.push_back(p2);
}



void VW_LEFT_MarkerInit()
{
	vw_left_marker_.header.frame_id = "base_link";
	vw_left_marker_.header.stamp = ros::Time();
	vw_left_marker_.ns = "vw_left_marker";
	vw_left_marker_.id = 0;
	vw_left_marker_.type = visualization_msgs::Marker::CUBE;
	vw_left_marker_.action = visualization_msgs::Marker::ADD;
	vw_left_marker_.pose.position.x = vw_left_.px;
	vw_left_marker_.pose.position.y = vw_left_.py - vw_width_*0.5;
	vw_left_marker_.pose.position.z = vw_left_.pz;
	vw_left_marker_.pose.orientation.x = 0.0;
	vw_left_marker_.pose.orientation.y = 0.0;
	vw_left_marker_.pose.orientation.z = 0.0;
	vw_left_marker_.pose.orientation.w = 1.0;
	vw_left_marker_.scale.x = 1.0;
	vw_left_marker_.scale.y = vw_width_;
	vw_left_marker_.scale.z = 1.0;
	vw_left_marker_.color.a = 0.5; // Don't forget to set the alpha!
	vw_left_marker_.color.r = 0.8;
	vw_left_marker_.color.g = 0.8;
	vw_left_marker_.color.b = 0.8;
	//only if using a MESH_RESOURCE marker type:
	// marker_mesh_resource = "package://pr2_description/meshes/base_v0/base.dae";
}

void VW_RIGHT_MarkerInit()
{
	vw_right_marker_.header.frame_id = "base_link";
	vw_right_marker_.header.stamp = ros::Time();
	vw_right_marker_.ns = "vw_right_marker";
	vw_right_marker_.id = 0;
	vw_right_marker_.type = visualization_msgs::Marker::CUBE;
	vw_right_marker_.action = visualization_msgs::Marker::ADD;
	vw_right_marker_.pose.position.x = vw_right_.px;
	vw_right_marker_.pose.position.y = vw_right_.py + vw_width_*0.5;
	vw_right_marker_.pose.position.z = vw_right_.pz;
	vw_right_marker_.pose.orientation.x = 0.0;
	vw_right_marker_.pose.orientation.y = 0.0;
	vw_right_marker_.pose.orientation.z = 0.0;
	vw_right_marker_.pose.orientation.w = 1.0;
	vw_right_marker_.scale.x = 1.0;
	vw_right_marker_.scale.y = vw_width_;
	vw_right_marker_.scale.z = 1.0;
	vw_right_marker_.color.a = 0.5; // Don't forget to set the alpha!
	vw_right_marker_.color.r = 0.8;
	vw_right_marker_.color.g = 0.8;
	vw_right_marker_.color.b = 0.8;
	//only if using a MESH_RESOURCE marker type:
	// marker_mesh_resource = "package://pr2_description/meshes/base_v0/base.dae";
}



void UpdateTF(tf2_ros::TransformBroadcaster &br)
{
	static geometry_msgs::Transform transform;

    transform.translation.x = interface_.state.pose.position.x;
    transform.translation.y = interface_.state.pose.position.y;
    transform.translation.z = interface_.state.pose.position.z;
    transform.rotation.w = 1;

    tf_stamped_.header.stamp = ros::Time::now();
    tf_stamped_.header.frame_id = "master";
    tf_stamped_.child_frame_id = "ee";
    tf_stamped_.transform = transform;

    br.sendTransform(tf_stamped_);
}

void CallbackInterfaceState(const phantom_premium_msgs::TeleoperationDeviceStateStamped::ConstPtr& msg)
{
	interface_ = *msg;
	tf2_ros::TransformBroadcaster br;
	UpdateTF(br);

	ee_marker_.pose.position.x = msg->state.pose.position.x;
	ee_marker_.pose.position.y = msg->state.pose.position.y;
	ee_marker_.pose.position.z = msg->state.pose.position.z;

	// ee_marker_.header.stamp = msg->header.stamp;
	ee_marker_.header.stamp = ros::Time::now();

	pubEEMaker_.publish(ee_marker_);



	// tf2_ros::Buffer tfBuffer;
	// tf2_ros::TransformListener tfListener(tfBuffer);

	// geometry_msgs::TransformStamped transformStamped;
    // try{
    //   transformStamped = tfBuffer.lookupTransform("base_link", "ee",
    //                            ros::Time(0));
    // }
    // catch (tf2::TransformException &ex) {
    //   ROS_WARN("%s",ex.what());
    //   ros::Duration(1.0).sleep();
    // }
	// geometry_msgs::Point p;
	// p.x = transformStamped.transform.translation.x;
	// p.y = transformStamped.transform.translation.y;
	// p.z = transformStamped.transform.translation.z;

}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "force_visualize_node");

	ros::NodeHandle nh;
	ros::NodeHandle pnh("~");

	pnh.param("publish_rate", publish_rate_, 100);
	
	tf2_ros::TransformBroadcaster br;

    subInterfaceState_ = nh.subscribe("/master_control/input_master_state", 1, CallbackInterfaceState); //topic que function

	pubEEMaker_ = nh.advertise<visualization_msgs::Marker>("ee_marker", 1); //topic que
	pubVWLeftMaker_ = nh.advertise<visualization_msgs::Marker>("vw_left_marker", 10); //topic que
	pubVWRightMaker_ = nh.advertise<visualization_msgs::Marker>("vw_right_marker", 10); //topic que

	pubVGMaker_ = nh.advertise<visualization_msgs::Marker>("vg_marker", 10); //topic que

    pnh.getParam("/virtual_guidance/position/x",vg_.px);
	pnh.getParam("/virtual_guidance/position/y",vg_.py);
	pnh.getParam("/virtual_guidance/position/z",vg_.pz);
	pnh.getParam("/virtual_guidance/stiffness",vg_.kp);
	pnh.getParam("/virtual_guidance/damping",vg_.kd);

	pnh.getParam("/virtual_wall/left/position/x",vw_left_.px);
	pnh.getParam("/virtual_wall/left/position/y",vw_left_.py);
	pnh.getParam("/virtual_wall/left/position/z",vw_left_.pz);
	pnh.getParam("/virtual_wall/left/stiffness",vw_left_.kp);
	pnh.getParam("/virtual_wall/left/damping",vw_left_.kd);

	pnh.getParam("/virtual_wall/right/position/x",vw_right_.px);
	pnh.getParam("/virtual_wall/right/position/y",vw_right_.py);
	pnh.getParam("/virtual_wall/right/position/z",vw_right_.pz);
	pnh.getParam("/virtual_wall/right/stiffness",vw_right_.kp);
	pnh.getParam("/virtual_wall/right/damping",vw_right_.kd);

	pnh.param("virtual_wall_width",vw_width_,0.02);

	EEMarkerInit();
	VGMarkerInit();
	VW_LEFT_MarkerInit();
	VW_RIGHT_MarkerInit();
	

	ros::Rate loop_rate(publish_rate_);
	int count = 0;
	while (ros::ok())
	{
		

		// Transform master to base_link
    // geometry_msgs::TransformStamped transformStamped;
    // try{
    //   transformStamped = tfBuffer.lookupTransform("base_link", "master",
    //                            ros::Time(0));
    // }
    // catch (tf2::TransformException &ex) {
    //   ROS_WARN("%s",ex.what());
    //   ros::Duration(1.0).sleep();
    //   continue;
    // }

	// tf2::Matrix3x3 mx;
	// mx.





	// mx = transformStamped.transform.getBasis();
	// std::cout << "Transform" <<std::endl;
	// std::cout << transformStamped.transform <<""



// 		geometry_msgs::Pose
// phantomTransformMatrixToPoseMsg(const array<double, 16> &transform_data) {
//   auto position =
//       createPoint(transform_data[3] * PHANTOM_POSITION_SCALE_FORWARD,
//                   transform_data[7] * PHANTOM_POSITION_SCALE_FORWARD,
//                   transform_data[11] * PHANTOM_POSITION_SCALE_FORWARD);

//   tf2::Matrix3x3 mx(transform_data[0], transform_data[1], transform_data[2],
//                     transform_data[4], transform_data[5], transform_data[6],
//                     transform_data[8], transform_data[9], transform_data[10]);
//   tf2::Quaternion q;
//   mx.getRotation(q);

//   geometry_msgs::Pose pose;
//   pose.orientation = tf2::toMsg(q);
//   pose.position = position;
//   return pose;
// }

		//


		// PublishMarkerArray();
		// ee_marker_.header.stamp = ros::Time::now();
		vg_marker_.header.stamp = ros::Time::now();
		vw_left_marker_.header.stamp = ros::Time::now();
		vw_right_marker_.header.stamp = ros::Time::now();

		pubVGMaker_.publish(vg_marker_);
		pubVWLeftMaker_.publish(vw_left_marker_);
		pubVWRightMaker_.publish(vw_right_marker_);

		ros::spinOnce();

		loop_rate.sleep();
		++count;
	}

	return 0;
}
