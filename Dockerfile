# Dockerfile for local development
# This allows you to develop locally with the same environment as the server

FROM fedora:42

# Set environment variables
ENV PKG_CONFIG_PATH="/usr/local/lib64/pkgconfig:/usr/local/lib/pkgconfig"
ENV LD_LIBRARY_PATH="/usr/local/lib64:/usr/local/lib"

# Install development tools and dependencies
RUN dnf update -y && \
    dnf groupinstall -y "Development Tools" "Development Libraries" && \
    dnf install -y \
        cmake \
        gcc-c++ \
        make \
        git \
        curl \
        curl-devel \
        rapidjson-devel \
        gtest-devel \
        gmock-devel \
        meson \
        ninja-build \
        pkg-config \
        openssl-devel \
        zlib-devel \
        fmt-devel \
        spdlog-devel \
        tbb-devel \
        vim \
        nano \
        htop \
        procps-ng \
        && dnf clean all

# Create a non-root user for development
ARG USERNAME=developer
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME && \
    useradd --uid $USER_UID --gid $USER_GID -m $USERNAME && \
    echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME && \
    chmod 0440 /etc/sudoers.d/$USERNAME


# Set up library paths
RUN echo "/usr/local/lib64" > /etc/ld.so.conf.d/local.conf && \
    echo "/usr/local/lib" >> /etc/ld.so.conf.d/local.conf && \
    ldconfig

# Switch to non-root user
USER $USERNAME

# Set working directory
WORKDIR /workspace

# Create helpful aliases
RUN echo 'alias ll="ls -la"' >> ~/.bashrc && \
    echo 'alias build="mkdir -p build && cd build && cmake .. && make -j\$(nproc)"' >> ~/.bashrc && \
    echo 'alias test="cd build && make unit_tests integration_tests && ./tests/unit_tests && ./tests/integration_tests"' >> ~/.bashrc && \
    echo 'alias run="cd build && ./rest_server"' >> ~/.bashrc && \
    echo 'export PKG_CONFIG_PATH="/usr/local/lib64/pkgconfig:/usr/local/lib/pkgconfig"' >> ~/.bashrc

# Default command
CMD ["/bin/bash"]
