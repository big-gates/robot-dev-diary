FROM ros:jazzy

RUN apt-get update && apt-get install -y --no-install-recommends \
      ros-jazzy-desktop \
      gdb \
      clangd \
      clang-format \
      ros-jazzy-xacro \
      ros-jazzy-joint-state-publisher \
      ros-jazzy-foxglove-bridge \
    && rm -rf /var/lib/apt/lists/*

RUN echo 'source /opt/ros/jazzy/setup.bash' >> /root/.bashrc

ENV CMAKE_EXPORT_COMPILE_COMMANDS=1
