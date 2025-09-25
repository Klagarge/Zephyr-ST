# Multi-stage Zephyr development image for STM32 targets

#############################################
# Stage 1: Build environment with all tools #
#############################################
FROM debian:stable-20241016-slim AS builder

# Build arguments
ARG ZEPHYR_RTOS_COMMIT=26603cefaf41298c417f2eee834ed40d9ab35d3a
ARG ZEPHYR_SDK_VERSION=0.16.8
ARG WGET_ARGS="-q --show-progress --progress=bar:force:noscroll"
ARG VIRTUAL_ENV=/opt/venv
ARG TARGETARCH

# Set non-interactive frontend and shell
ENV DEBIAN_FRONTEND=noninteractive
SHELL ["/bin/bash", "-c"]

# Install build dependencies and tools in one layer
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    wget \
    curl \
    git \
    python3 \
    python3-pip \
    python3-venv \
    build-essential \
    cmake \
    ninja-build \
    xz-utils \
    file \
    device-tree-compiler \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean

# Set up Python virtual environment
ENV VIRTUAL_ENV=${VIRTUAL_ENV}
RUN python3 -m venv ${VIRTUAL_ENV}
ENV PATH="${VIRTUAL_ENV}/bin:$PATH"

# Install Python dependencies
RUN pip3 install --no-cache-dir --upgrade pip && \
    pip3 install --no-cache-dir \
    west==1.2.0 \
    pyelftools \
    pyyaml \
    packaging \
    intelhex

# Create directories
RUN mkdir -p /opt/toolchains

# Install Zephyr RTOS
RUN cd /opt/toolchains && \
    git clone --depth 1 --branch main https://github.com/zephyrproject-rtos/zephyr.git && \
    cd zephyr && \
    git fetch --depth 1 origin ${ZEPHYR_RTOS_COMMIT} && \
    git checkout ${ZEPHYR_RTOS_COMMIT} && \
    pip3 install --no-cache-dir -r scripts/requirements-base.txt && \
    rm -rf .git

# Initialize west workspace and update modules
RUN cd /opt/toolchains && \
    west init -l zephyr && \
    west update --narrow -o=--depth=1 && \
    west blobs fetch hal_st && \
    west blobs fetch cmsis && \
    west blobs fetch mbedtls && \
    west blobs fetch mcuboot && \
    find . -name ".git" -type d -exec rm -rf {} + 2>/dev/null || true

# Download, extract and setup Zephyr SDK
RUN cd /opt/toolchains && \
    if [ "$TARGETARCH" = "arm64" ]; then \
        SDK_ARCH="aarch64"; \
    else \
        SDK_ARCH="x86_64"; \
    fi && \
    wget ${WGET_ARGS} https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-${SDK_ARCH}.tar.xz && \
    tar xf zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-${SDK_ARCH}.tar.xz && \
    rm zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-${SDK_ARCH}.tar.xz && \
    mv zephyr-sdk-${ZEPHYR_SDK_VERSION} zephyr-sdk

# Install Zephyr SDK ARM toolchain
RUN cd /opt/toolchains/zephyr-sdk && \
    ./setup.sh -t arm-zephyr-eabi -h -c

# Install host tools
RUN cd /opt/toolchains/zephyr-sdk && \
    if [ "$TARGETARCH" = "arm64" ]; then \
        SDK_ARCH="aarch64"; \
    else \
        SDK_ARCH="x86_64"; \
    fi && \
    wget ${WGET_ARGS} https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/hosttools_linux-${SDK_ARCH}.tar.xz && \
    tar xf hosttools_linux-${SDK_ARCH}.tar.xz && \
    rm hosttools_linux-${SDK_ARCH}.tar.xz && \
    bash zephyr-sdk-*-hosttools-standalone-*.sh -y -d .

################################
# Stage 2: Final runtime image #
################################
FROM debian:stable-20241016-slim AS runtime

# Runtime arguments
ARG PASSWORD="zephyr"
ARG ZEPHYR_SDK_VERSION=0.16.8
ARG VIRTUAL_ENV=/opt/venv

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV VIRTUAL_ENV=${VIRTUAL_ENV}
ENV PATH="${VIRTUAL_ENV}/bin:$PATH"
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8
ENV ZEPHYR_BASE=/opt/toolchains/zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=/opt/toolchains/zephyr-sdk

# Install runtime dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    git \
    file \
    locales \
    build-essential \
    cmake \
    ninja-build \
    device-tree-compiler \
    gdb \
    gdb-multiarch \
    openocd \
    openssh-server \
    python3 \
    python3-pip \
    rsync \
    vim \
    nano \
    ccache \
    libusb-1.0-0-dev \
    udev \
    wget \
    curl \
    sudo \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean

# Set up locale
RUN sed -i '/^#.*en_US.UTF-8/s/^#//' /etc/locale.gen && \
    locale-gen en_US.UTF-8 && \
    update-locale LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8

# Copy Zephyr installation from builder stage
COPY --from=builder /opt/toolchains/zephyr /opt/toolchains/zephyr
COPY --from=builder /opt/toolchains/tools /opt/toolchains/tools
COPY --from=builder /opt/toolchains/bootloader /opt/toolchains/bootloader
COPY --from=builder /opt/toolchains/.west /opt/toolchains/.west
COPY --from=builder /opt/toolchains/modules/lib /opt/toolchains/modules/lib
COPY --from=builder /opt/toolchains/modules/tee /opt/toolchains/modules/tee
COPY --from=builder /opt/toolchains/modules/crypto /opt/toolchains/modules/crypto
COPY --from=builder /opt/toolchains/modules/debug /opt/toolchains/modules/debug
COPY --from=builder /opt/toolchains/modules/fs /opt/toolchains/modules/fs
COPY --from=builder /opt/toolchains/modules/bsim_hw_models /opt/toolchains/modules/bsim_hw_models
COPY --from=builder /opt/toolchains/modules/hal/stm32 /opt/toolchains/modules/hal/stm32
COPY --from=builder /opt/toolchains/modules/hal/st /opt/toolchains/modules/hal/st
COPY --from=builder /opt/toolchains/modules/hal/cmsis /opt/toolchains/modules/hal/cmsis
COPY --from=builder /opt/toolchains/zephyr-sdk/arm-zephyr-eabi /opt/toolchains/zephyr-sdk/arm-zephyr-eabi
COPY --from=builder /opt/toolchains/zephyr-sdk/cmake /opt/toolchains/zephyr-sdk/cmake
COPY --from=builder /opt/toolchains/zephyr-sdk/sdk_toolchains /opt/toolchains/zephyr-sdk/sdk_toolchains
COPY --from=builder /opt/toolchains/zephyr-sdk/sdk_version /opt/toolchains/zephyr-sdk/sdk_version

COPY --from=builder ${VIRTUAL_ENV} ${VIRTUAL_ENV}

# Set up users and SSH
RUN echo "root:${PASSWORD}" | chpasswd && \
    groupadd -g 1000 developer && \
    useradd -u 1000 -g 1000 -m -s /bin/bash developer && \
    echo "developer:${PASSWORD}" | chpasswd && \
    usermod -aG sudo developer && \
    mkdir -p /var/run/sshd && \
    chmod 0755 /
