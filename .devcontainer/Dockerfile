FROM ros:jazzy

RUN apt-get update && apt-get install -y --no-install-recommends \
      ros-jazzy-desktop \
      gdb \
      clangd \
    && rm -rf /var/lib/apt/lists/*

RUN echo 'source /opt/ros/jazzy/setup.bash' >> /root/.bashrc

ENV CMAKE_EXPORT_COMPILE_COMMANDS=1
