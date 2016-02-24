#include <cwru_pcl_utils/cwru_pcl_utils.h>

using namespace std;

int main(int argc, char** argv) {
    ros::init(argc, argv, "cwru_pcl_utils_davinci_main"); //node name
    ros::NodeHandle nh;
    CwruPclUtils cwru_pcl_utils(&nh);

    // whole point cloud received including coordinate information
    while (!cwru_pcl_utils.got_kinect_cloud()) {
        ROS_INFO("did not receive pointcloud");
        ros::spinOnce();
        ros::Duration(1.0).sleep();
    }
    ROS_INFO("got a pointcloud");
    ROS_INFO("saving pointcloud");
    cwru_pcl_utils.save_kinect_snapshot();
    cwru_pcl_utils.save_kinect_clr_snapshot();  // save color version of pointcloud as well


    //set up a publisher to display clouds in rviz:
    ros::Publisher pubCloud = nh.advertise<sensor_msgs::PointCloud2> ("/pcl_cloud_display", 1);
    //pcl::PointCloud<pcl::PointXYZ> & outputCloud
    pcl::PointCloud<pcl::PointXYZ> display_cloud; // instantiate a pointcloud object, which will be used for display in rviz
    sensor_msgs::PointCloud2 pcl2_display_cloud; //(new sensor_msgs::PointCloud2); //corresponding data type for ROS message

    Eigen::Vector3f centroid, plane_normal, major_axis;

    pcl::PointCloud<pcl::PointXYZ>::Ptr all_points = cwru_pcl_utils.get_all_points();
    pcl::PointCloud<pcl::PointXYZ> cloud;
    //pcl::PointCloud<pcl::PointXYZ> cloud;
    cloud.push_back (pcl::PointXYZ (-1, -1, 0)); 
    cloud.push_back (pcl::PointXYZ (-1, 1, 0)); 
    cloud.push_back (pcl::PointXYZ (1, -1, 0)); 
    cloud.push_back (pcl::PointXYZ (1, 1, 0)); 
    pcl::PointCloud<pcl::PointXYZ>::Ptr test_cloud(&cloud);

    // compute the three vectors and plane_dist
    plane_normal = cwru_pcl_utils.get_plane_normal(test_cloud);
    major_axis = plane_normal;
    centroid = cwru_pcl_utils.get_centroid(test_cloud);
    double plane_dist = plane_normal.dot(centroid);
    ROS_INFO_STREAM("ALL POINTS ---------- normal: " << plane_normal.transpose() << "; dist = " << plane_dist);

    // select point(s) and compute the centroid
    while (ros::ok()) {
        if (cwru_pcl_utils.got_selected_points()) {
            ROS_INFO("process selected-points!");
            cwru_pcl_utils.reset_got_selected_points();   // reset the selected-points trigger

            pcl::PointCloud<pcl::PointXYZ>::Ptr selected_points = cwru_pcl_utils.get_selected_points();
            cwru_pcl_utils.print_points(selected_points);

            ROS_INFO("Press ENTER to check plane_normal of the selected points.");
            cin.get();
            // compute the three vectors and plane_dist
            plane_normal = cwru_pcl_utils.get_plane_normal(selected_points);
            major_axis = plane_normal;
            centroid = cwru_pcl_utils.get_centroid(selected_points);
            plane_dist = plane_normal.dot(centroid);
            //ROS_INFO_STREAM(" normal: " << plane_normal.transpose() << "; dist = " << plane_dist);

            ROS_INFO("Press ENTER to check coplanar points.");
            cin.get();
            // extract coplanar points and create a point cloud -- genpurpose_cloud 
            cwru_pcl_utils.extract_coplanar_pcl_operation(centroid); // offset the transformed, selected points and put result in gen-purpose object
            pcl::PointCloud<pcl::PointXYZ>::Ptr genpurpose_cloud = cwru_pcl_utils.get_genpurpose_points();
            // compute the three vectors and plane_dist
            plane_normal = cwru_pcl_utils.get_plane_normal(genpurpose_cloud);
            major_axis = plane_normal;
            centroid = cwru_pcl_utils.get_centroid(genpurpose_cloud);
            plane_dist = plane_normal.dot(centroid);
            ROS_INFO_STREAM(" normal: " << plane_normal.transpose() << "; dist = " << plane_dist);

            cwru_pcl_utils.get_gen_purpose_cloud(display_cloud);
        }

        //cwru_pcl_utils.get_gen_purpose_cloud(display_cloud);
        pcl::toROSMsg(display_cloud, pcl2_display_cloud); //convert datatype to compatible ROS message type for publication
        pcl2_display_cloud.header.stamp = ros::Time::now(); //update the time stamp, so rviz does not complain        
        pubCloud.publish(pcl2_display_cloud); //publish a point cloud that can be viewed in rviz (under topic pcl_cloud_display)

        ros::Duration(0.5).sleep(); // sleep for half a second
        ros::spinOnce();
    }
    ROS_INFO("my work here is done!");

}
