#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit_msgs/msg/collision_object.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <chrono>

using namespace std::chrono_literals;

// 辅助函数：创建碰撞物体
moveit_msgs::msg::CollisionObject create_collision_object(
    const std::string& id, 
    double size_x, double size_y, double size_z,
    double pos_x, double pos_y, double pos_z) 
{
    moveit_msgs::msg::CollisionObject obj;
    obj.header.frame_id = "base_link"; 
    obj.id = id;

    shape_msgs::msg::SolidPrimitive primitive;
    primitive.type = primitive.BOX;
    primitive.dimensions = {size_x, size_y, size_z};

    geometry_msgs::msg::Pose pose;
    // 默认姿态
    pose.orientation.w = 1.0;
    pose.position.x = pos_x; 
    pose.position.y = pos_y; 
    pose.position.z = pos_z;

    obj.primitives.push_back(primitive);
    obj.primitive_poses.push_back(pose);
    obj.operation = obj.ADD;

    return obj;
}

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = rclcpp::Node::make_shared("pick_and_place_node");
    
    moveit::planning_interface::MoveGroupInterface move_group(node, "ur_manipulator");
    moveit::planning_interface::PlanningSceneInterface planning_scene;

    move_group.setPlanningTime(10.0);

    // ==========================================
    // 阶段 1：构建物理世界 (造一张超大的桌子拦住下方)
    // ==========================================
    // 桌子放大到 2米x2米，厚度0.1，z=-0.05，完全封死机械臂往下的路径
    auto table = create_collision_object("table", 2.0, 2.0, 0.1, 0.0, 0.0, -0.05);
    // 方块：z=0.025 (平放在 z=0 的桌面上)
    auto box = create_collision_object("target_box", 0.05, 0.05, 0.05, 0.4, 0.2, 0.025);

    planning_scene.applyCollisionObjects({table, box});
    RCLCPP_INFO(node->get_logger(), "阶段 1：超大桌子与方块已就位！");
    rclcpp::sleep_for(2s); 

    // ==========================================
    // 阶段 2：机械臂安全起立 (标准肘部朝上姿态)
    // ==========================================
    RCLCPP_INFO(node->get_logger(), "阶段 2：准备起立...");
    move_group.setJointValueTarget({0.0, -1.57, 1.57, -1.57, -1.57, 0.0});
    move_group.move();

    // ==========================================
    // 定义标准抓取姿态 (强制末端垂直朝下)
    // UR5 末端垂直朝下的四元数通常为绕 X 轴旋转 180 度
    // ==========================================
    geometry_msgs::msg::Pose target_pose;
    target_pose.orientation.x = 1.0; 
    target_pose.orientation.y = 0.0;
    target_pose.orientation.z = 0.0;
    target_pose.orientation.w = 0.0;
    
    // ==========================================
    // 阶段 3：抓取 (Pick) - 悬停、下压、吸附、提起
    // ==========================================
    RCLCPP_INFO(node->get_logger(), "阶段 3：前往抓取点上方 (Pre-pick)...");
    target_pose.position.x = 0.4;
    target_pose.position.y = 0.2;
    target_pose.position.z = 0.25; // 悬停在方块上方 25cm 处
    move_group.setPoseTarget(target_pose);
    move_group.move();

    RCLCPP_INFO(node->get_logger(), "下压进行抓取...");
    target_pose.position.z = 0.15; // 刚好接触方块上方
    move_group.setPoseTarget(target_pose);
    move_group.move();

    // 吸附
    std::string end_effector_link = "tool0"; 
    move_group.attachObject("target_box", end_effector_link);
    RCLCPP_INFO(node->get_logger(), "已吸附方块！");
    rclcpp::sleep_for(1s);

    RCLCPP_INFO(node->get_logger(), "将方块提起 (Lift)...");
    target_pose.position.z = 0.35; // 提得足够高，绝对避开桌子
    move_group.setPoseTarget(target_pose);
    move_group.move();

    // ==========================================
    // 阶段 4：搬运与放置 (Place) - 平移、下压、释放、提起
    // ==========================================
    RCLCPP_INFO(node->get_logger(), "阶段 4：高空平移至放置点上方...");
    target_pose.position.y = -0.2; // 移动到桌子另一侧
    move_group.setPoseTarget(target_pose);
    move_group.move();

    RCLCPP_INFO(node->get_logger(), "下压进行放置...");
    target_pose.position.z = 0.15; // 降回桌面高度
    move_group.setPoseTarget(target_pose);
    move_group.move();

    // 释放
    move_group.detachObject("target_box");
    RCLCPP_INFO(node->get_logger(), "已释放方块！");
    rclcpp::sleep_for(1s);

    RCLCPP_INFO(node->get_logger(), "安全撤退...");
    target_pose.position.z = 0.25; // 夹爪上升离开物体
    move_group.setPoseTarget(target_pose);
    move_group.move();

    RCLCPP_INFO(node->get_logger(), "任务圆满完成！");
    rclcpp::shutdown();
    return 0;
}
