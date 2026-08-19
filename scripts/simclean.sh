#!/bin/bash
# 시뮬 관련 프로세스를 전부 정리한다.
#
# 왜 필요한가:
#   launch 를 Ctrl+C 로 껐어도 ExecuteProcess 로 띄운 Gazebo 는 살아남는 경우가 있다.
#   그 상태에서 다시 launch 하면 시뮬이 2벌 돌고, 발행 주기가 배수로 튀며,
#   /clock 이 두 곳에서 나와 시간이 뒤섞인다. 증상은 매번 다르지만 원인은 하나다.
#
# 왜 pkill -f 를 안 쓰는가:
#   -f 는 명령줄 전체를 매칭해서 이 스크립트를 호출한 셸까지 죽인다.
#   launch 래퍼도 "sim.launch.py" 로 찾으면 같은 문제가 생기므로
#   실제 args 형태인 "bin/ros2 launch" 로 좁히고 자기 PID 는 제외한다.
#
# 사용:
#   ./scripts/simclean.sh && ros2 launch fake_robot sim.launch.py

SELF=$$

kill_by_name() {
  for n in ruby robot_state_pub parameter_bridg foxglove_bridge teleop_node odom_node; do
    pkill "$1" -x "$n" 2>/dev/null
  done
}

kill_launch_wrapper() {
  ps -eo pid,args \
    | awk -v self="$SELF" '$1 != self && /bin\/ros2 launch/ && !/awk/ {print $1}' \
    | while read -r pid; do kill "$1" "$pid" 2>/dev/null; done
}

kill_by_name ""
kill_launch_wrapper ""
sleep 3
kill_by_name -9
kill_launch_wrapper -9
sleep 2

left=$(ps -eo comm | grep -cE "ruby|robot_state|parameter_b|foxglove|teleop_node|odom_node")
if [ "$left" -eq 0 ]; then
  echo "simclean: 0 개 — 깨끗함"
else
  echo "simclean: 남음"
  ps -eo comm | grep -E "ruby|robot_state|parameter_b|foxglove|teleop_node|odom_node" | sort | uniq -c
fi
