#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit_msgs/msg/collision_object.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <chrono>

using namespace std::chrono_literals;

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("pick_and_place_cpp");

  // 1. 初始化 MoveGroupInterface
  moveit::planning_interface::MoveGroupInterface move_group(node, "ur_manipulator");
  move_group.setPlanningTime(10.0);
  move_group.setMaxVelocityScalingFactor(0.5);
  move_group.setMaxAccelerationScalingFactor(0.5);

  // 2. 初始化 PlanningSceneInterface (为了添加“物体”)
  moveit::planning_interface::PlanningSceneInterface planning_scene_interface;

  // 3. 定义并创建一个“物体”（Collision Object）
  moveit_msgs::msg::CollisionObject collision_object;
  collision_object.id = "target_box";
  collision_object.header.frame_id = "world"; // 必须和你的 robot_base 一致

  shape_msgs::msg::SolidPrimitive primitive;
  primitive.type = primitive.BOX;
  primitive.dimensions.resize(3);
  primitive.dimensions[0] = 0.05; // 长
  primitive.dimensions[1] = 0.05; // 宽
  primitive.dimensions[2] = 0.05; // 高

  geometry_msgs::msg::Pose box_pose;
  box_pose.orientation.w = 1.0;
  box_pose.position.x = 0.5; // 设置为你的抓取目标位置
  box_pose.position.y = 0.0;
  box_pose.position.z = 0.025; // 放在桌面上（z=0.025 是方块高度的一半）

  collision_object.primitives.push_back(primitive);
  collision_object.primitive_poses.push_back(box_pose);
  collision_object.operation = collision_object.ADD;

  // 将物体发布到场景中
  planning_scene_interface.applyCollisionObject(collision_object);
  RCLCPP_INFO(node->get_logger(), "Collision object added to the scene");

  // 初始化夹爪发布者
  auto gripper_pub = node->create_publisher<std_msgs::msg::Float64MultiArray>(
      "/ur1/forward_position_gripper_controller/commands", 10);

  // --- PICK 流程 ---
  geometry_msgs::msg::PoseStamped pick_pose;
  pick_pose.header.frame_id = "world";
  pick_pose.pose.position.x = 0.5;
  pick_pose.pose.position.y = 0.0;
  pick_pose.pose.position.z = 0.2;
  
  tf2::Quaternion q;
  q.setRPY(M_PI, 0.0, 0.0);
  pick_pose.pose.orientation.x = q.x();
  pick_pose.pose.orientation.y = q.y();
  pick_pose.pose.orientation.z = q.z();
  pick_pose.pose.orientation.w = q.w();
  
  move_group.setPoseTarget(pick_pose);
  if (move_group.move() == moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_INFO(node->get_logger(), "Reached pick position");
  }

  // 闭合夹爪并绑定物体
  std_msgs::msg::Float64MultiArray close_cmd;
  close_cmd.data = {0.7};
  gripper_pub->publish(close_cmd);
  RCLCPP_INFO(node->get_logger(), "Gripper closed");
  
  // 【关键点】将物体绑定到机械手上
  move_group.attachObject("target_box");
  RCLCPP_INFO(node->get_logger(), "Object attached");
  rclcpp::sleep_for(1s);

  // --- PLACE 流程 ---
  geometry_msgs::msg::PoseStamped place_pose;
  place_pose.header.frame_id = "world";
  place_pose.pose.position.x = 0.3;
  place_pose.pose.position.y = -0.3;
  place_pose.pose.position.z = 0.2;
  place_pose.pose.orientation = pick_pose.pose.orientation;

  move_group.setPoseTarget(place_pose);
  if (move_group.move() == moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_INFO(node->get_logger(), "Reached place position");
  }

  // 松开夹爪并解绑物体
  std_msgs::msg::Float64MultiArray open_cmd;
  open_cmd.data = {0.1};
  gripper_pub->publish(open_cmd);
  
  // 【关键点】解绑物体
  move_group.detachObject("target_box");
  RCLCPP_INFO(node->get_logger(), "Object detached and gripper opened");

  rclcpp::shutdown();
  return 0;
}