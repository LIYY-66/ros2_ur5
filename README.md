# ROS2 UR5机器人抓取仿真

## 项目介绍

本项目是一个基于ROS2 Jazzy的UR5E机器人抓取与放置仿真环境。它集成了Gazebo Harmonic物理仿真器与MoveIt2运动规划框架，提供了完整的机器人仿真、运动规划与抓取控制功能。项目支持单机器人及多机器人协同仿真场景，适用于机器人抓取算法开发、运动规划测试与多机协作研究。

## 功能特点

- **完整的UR5E机器人仿真模型**：包含精确的URDF描述与Gazebo物理属性
- **MoveIt2集成**：支持基于Rviz的运动规划、轨迹可视化与实时控制
- **抓取与放置功能**：提供完整的Pick and Place工作流程实现
- **多机器人仿真**：支持多台UR5机器人协同工作场景
- **夹爪控制**：通过ROS2话题或Action接口实现夹爪开合控制
- **模块化设计**：清晰的包结构便于功能扩展与定制

## 使用方法

### 环境要求

- ROS2 Jazzy
- Gazebo Harmonic
- MoveIt2

### 安装与配置


### 启动仿真

#### 单机器人仿真

启动包含单台UR5机器人的Gazebo仿真环境与MoveIt2控制界面：

```bash
ros2 launch ur_simulation_gz ur_sim_moveit.launch.py
```

启动运动规划演示节点：

```bash
ros2 run move_test_cpp demo_controller
```

#### 多机器人仿真

启动包含多台UR5机器人的协同仿真环境：

```bash
ros2 launch ur_simulation_gz dual_sim_moveit.launch.py
```

#### 夹爪控制

通过ROS话题直接控制夹爪开合：

```bash
ros2 topic pub /forward_position_gripper_controller/commands std_msgs/msg/Float64MultiArray "{data: [0.1]}"
```

### 项目结构

```
ur_simulation_gz/
├── worlds/                    # Gazebo仿真世界文件
└── ur_simulation_gz/          # Gazebo启动文件与URDF模型

ur_moveit_config/
├── launch/                   # MoveIt2与Rviz启动配置
├── config/                   # MoveIt2参数配置文件
└── srdf/                     # 语义机器人描述文件

move_test_cpp/                # MoveIt2运动规划与机器人控制节点
```

## 当前实现状态

### 已完成功能

- 单机器人抓取与放置完整流程
- 基于Rviz的多机器人运动规划与执行
- 使用Action消息机制的夹爪控制接口

### 待实现功能

- 基于MoveIt2的多机器人协同控制框架
- 高级抓取姿态优化算法
- 仿真环境中的物体识别与定位模块

## 开发说明

本项目采用标准的ROS2包结构，便于集成到现有ROS2工作流中。所有启动文件均经过测试，确保在指定环境版本下稳定运行。用户可根据需要修改URDF模型、调整运动规划参数或扩展控制逻辑。

仿真环境提供了真实的物理交互效果，适合进行算法验证与系统测试，无需实际机器人硬件即可开展机器人抓取相关研究。