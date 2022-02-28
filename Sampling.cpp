//#include "tests_alberto/Sampling.h"
#include "geometry_msgs/Point.h"
#include "geometry_msgs/PoseArray.h"
#include "nav_msgs/OccupancyGrid.h"
#include "nav_msgs/MapMetaData.h"
#include "ros/ros.h"
#include <vector>
#include <math.h>
#include <time.h>
#include "tf2/LinearMath/Quaternion.h"
#include <costmap_2d/costmap_2d_ros.h>
#include <random>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <tf/transform_broadcaster.h>
#include "gazebo_msgs/ModelStates.h"

//sensors
#include <sensor_msgs/LaserScan.h>
namespace tests_alberto
{

typedef struct {
    geometry_msgs::Pose pose;
    float prob;
} Particle;

class Sampling
{

public:
Sampling(): n_(), vec_part_(NUM_PART)
{
  sub_map_ = n_.subscribe("map",100,&Sampling::mapCallback,this); //queue size enough?
  sub_gaz_ = n_.subscribe("gazebo/model_states",100,&Sampling::gazeboCallback,this);
  pub_ = n_.advertise<geometry_msgs::PoseArray>("particlecloud",10);
  //pub_.publish(vec_part_);

}

 void 
 mapCallback(const nav_msgs::OccupancyGrid::ConstPtr& msg)
 {
  //std_msgs::Header header = msg->header;
  map_height_ = msg->info.height;
  map_width_ = msg->info.width;
  map_received_ = true;
 }
 
 void
 gazeboCallback( const gazebo_msgs::ModelStatesConstPtr& gazemsg)
 {
   gazebo_pos_ = gazemsg->pose.back().position;
   ROS_INFO_STREAM("Gazebo position: x,y,z " << gazebo_pos_.x << gazebo_pos_.y << gazebo_pos_.z);
 }
 
 void predict(bool particles_init)
 {
     
    for (int i= 0;i< NUM_PART; i++)
    {
      float a_or = -M_PI, b_or = M_PI, b_pos_x,b_pos_y, a_pos; 
      if (!particles_init)
      {     
       b_pos_x = map_width_, b_pos_y = map_height_, a_pos = 0;
      }
      else
      {
        return;//Initialize particles using laser measures
      }
      //Generating a random orientation az
      unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator (seed);
      std::uniform_real_distribution <float> dist_or (a_or,b_or);
      float ax,ay = 0.0, az = dist_or (generator);
      
      //Converting to quaternion
      tf2::Quaternion q;    
      q.setRPY(ax,ay,az);
      
      //asigning to a particle
      Particle part;
      part.pose.orientation = tf2::toMsg(q);
      
      //Generating a random distance x position
      seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator2 (seed); //¿Por qué no puedo utilizar el mismo generator?
      std::uniform_real_distribution <float> dist_pos_x (a_pos, b_pos_x);
      part.pose.position.x = dist_pos_x (generator2);

      //Generating a random distance y position
      std::default_random_engine generator3 (seed);
      std::uniform_real_distribution <float> dist_pos_y (a_pos, b_pos_y);
      part.pose.position.y = dist_pos_y (generator3);
      part.pose.position.z = 0;

      //Calculating probability
      float probx = float(1)/ (b_pos_x - a_pos );
      float proby = float(1)/ (b_pos_y - a_pos);
      //float proby = float(1)/ (b_pos_y - a_pos + float(1));
      part.prob = probx * proby;

      //Checking values
      ROS_INFO("Particle probx: %f Particle proby: %f",probx,proby);
      ROS_INFO("Particle orientation z: %f, particle position x: %f, particle position y: %f",az,part.pose.position.x,part.pose.position.y);
      ROS_INFO_STREAM("Particle: " <<i<< " of "<< NUM_PART);
      //vec_part_->push_back(part);

      //Updating particles vector
      //ROS_INFO_STREAM_FILTER("Vector Partciles in step "<<i<<" ");
      vec_part_.push_back(part);
    }
   return;
  
 }

 void
 correct()
 {
   sub_lsr_ = n_.subscribe("scan_filtered", 100, &Sampling::lsr_callback, this);
   get_near_particles();
 }
 void
 get_near_particles()
 {

 }
 
 void
 lsr_callback(const sensor_msgs::LaserScanConstPtr &lsr_msg)
 {
   lsr_data_ =lsr_msg->ranges;
   ROS_INFO_STREAM("Ranges size: " << lsr_data_.size());
 }
  void step()
  {
    bool particles_inited = false;
    if (map_received_) {
      ROS_INFO("MAP RECEIVED, Height: %f, Width: %f", map_height_, map_width_);
      predict(particles_inited);
      particles_inited = true;
      
    }
    
    if (!particles_inited) {
      ROS_ERROR("Particles not initiated");
      return;
    }
    else {
      ROS_INFO("End initializing particles");
      correct();
      predict(particles_inited);
      return; 
    }



  geometry_msgs::PoseArray my_poses;
 for ( auto i = 0; i < vec_part_.size(); i++ )
 {
  my_poses.poses.push_back(vec_part_.at(i).pose);
   //pub_.publish(vec_part_->at(i).pose);
 }
 pub_.publish(my_poses);
  }

 private:
  static const int NUM_PART = 200;

  std::vector<Particle> vec_part_;
  std::vector<float> lsr_data_;
  costmap_2d::Costmap2D cmap_;
  ros::NodeHandle n_;
  ros::Subscriber sub_map_;
  ros::Subscriber sub_gaz_;
  ros::Subscriber sub_lsr_;
  ros::Publisher pub_;
  bool map_received_ = false;
  float map_width_, map_height_;
  geometry_msgs::Point gazebo_pos_;
};
} //namespace tests_alberto
int main (int argc, char **argv)
{
  ros::init(argc, argv, "sampling");
  ros::NodeHandle n;
  static const int RATE = 20;
  ros::Rate loop_rate(RATE);
  
  tests_alberto::Sampling samp;
  
  while (ros::ok())
  {
    samp.step();

    ros::spinOnce();
    loop_rate.sleep();
  }
  return 0;
}

