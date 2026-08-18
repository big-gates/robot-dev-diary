import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import ExecuteProcess, TimerAction
from launch.substitutions import Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    pkg = get_package_share_directory("fake_robot")

    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="screen",
        parameters=[
            {
                "robot_description": ParameterValue(
                    Command(
                        ["xacro ", os.path.join(pkg, "urdf", "fake_robot.urdf.xacro")]
                    ),
                    value_type=str,
                ),
                "use_sim_time": True,
            }
        ],
    )

    ros_gz_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        output="screen",
        parameters=[
            {
                "config_file": os.path.join(pkg, "config", "bridge.yaml"),
                "use_sim_time": True,
            }
        ],
    )

    gazebo = ExecuteProcess(
        cmd=[
            "gz",
            "sim",
            "-r",
            "-s",
            "--headless-rendering",
            os.path.join(pkg, "worlds", "restaurant.sdf"),
        ],
        output="screen",
    )

    spawn = Node(
        package="ros_gz_sim",
        executable="create",
        output="screen",
        arguments=["-topic", "robot_description", "-name", "fake_robot", "-z", "0.15"],
        parameters=[{"use_sim_time": True}],
    )

    delayed_spawn = TimerAction(period=5.0, actions=[spawn])

    foxglove = Node(
        package="foxglove_bridge",
        executable="foxglove_bridge",
        output="screen",
        parameters=[{"use_sim_time": True}],
    )

    return LaunchDescription(
        [
            gazebo,
            robot_state_publisher,
            ros_gz_bridge,
            delayed_spawn,
            foxglove,
        ]
    )
