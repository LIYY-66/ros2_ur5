import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.substitutions import Command
from launch_ros.parameter_descriptions import ParameterValue

def generate_launch_description():
    # 1. 动态调用官方 ur_description 包中的 xacro 产生标准的 UR5 URDF
    urdf_file = os.path.join(get_package_share_directory('ur_description'), 'urdf', 'ur.urdf.xacro')
    robot_description_content = Command([
        'xacro ', urdf_file, ' ur_type:=ur5 name:=ur'
    ])
    robot_description = {'robot_description': ParameterValue(robot_description_content, value_type=str)}

    # 2. 动态调用官方 ur_moveit_config 包中的 xacro 产生标准的 UR5 SRDF
    srdf_file = os.path.join(get_package_share_directory('ur_moveit_config'), 'srdf', 'ur.srdf.xacro')
    robot_description_semantic_content = Command([
        'xacro ', srdf_file, ' ur_type:=ur5 name:=ur'
    ])
    robot_description_semantic = {'robot_description_semantic': ParameterValue(robot_description_semantic_content, value_type=str)}

    # 3. 直接定义官方默认的 ur_manipulator 运动学求解参数，彻底免去读外部 yaml 文件的风险
    kinematics_config = {
        'robot_description_kinematics': {
            'ur_manipulator': {
                'kinematics_solver': 'kdl_kinematics_plugin/KDLKinematicsPlugin',
                'kinematics_solver_search_resolution': 0.005,
                'kinematics_solver_timeout': 0.005,
            }
        }
    }

   # 4. 配置你的 C++ 核心控制节点
    pick_and_place_node = Node(
        package="move_test_cpp",
        executable="pick_and_place_cpp_node",
        output="screen",
        parameters=[
            robot_description,
            robot_description_semantic,
            kinematics_config,
            {'use_sim_time': False} # <-- 强行加入这一行，确保时钟纯净
        ],
    )

    return LaunchDescription([pick_and_place_node])