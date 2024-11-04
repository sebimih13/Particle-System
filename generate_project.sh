#!/bin/bash

# install prerequisites
packages=(
    build-essential
    libx11-dev
    libxcursor-dev
    libxrandr-dev
    libxinerama-dev
    libxi-dev
    libgl-dev
)

for package in "${packages[@]}"; do
    if dpkg -l | grep -q "^ii\s\+$package"; then
        echo "$package is already installed"
    else
        echo "$package is not installed"
        sudo apt install -y "$package"
    fi
done

# Vulkan SDK
force_exit=false

echo "Checking for Vulkan SDK"
if [ -n "$VULKAN_SDK" ]; then
    echo "Vulkan SDK is already installed - $VULKAN_SDK"
else
    VULKAN_SDK_INSTALL_URL="https://sdk.lunarg.com/sdk/download/latest/linux/vulkan_sdk.tar.xz"
    VULKAN_SDK_OUTPUT_FILE="VulkanSDK-Installer.tar.xz"
    
    echo "Vulkan SDK is not installed"

    VULKAN_SDK_VERSION=$(wget -qO- https://vulkan.lunarg.com/sdk/latest/linux.txt)
    echo "Getting the latest Vulkan SDK version: $VULKAN_SDK_VERSION"

    echo "Download installer for Linux"
    wget "$VULKAN_SDK_INSTALL_URL" -O "$VULKAN_SDK_OUTPUT_FILE"

    echo "Run Vulkan SDK Installer"
    tar xf "$VULKAN_SDK_OUTPUT_FILE"

    echo "Create VulkanSDK folder in Home"
    mkdir -p ~/VulkanSDK
    mv -uf "$VULKAN_SDK_VERSION"/* ~/VulkanSDK/

    echo "Run setup env"
    source ~/VulkanSDK/setup-env.sh

    echo "Add $VULKAN_SDK to bashrc"
    if ! grep -Fxq "export VULKAN_SDK=$VULKAN_SDK" ~/.bashrc; then
        echo "export VULKAN_SDK=$VULKAN_SDK" >> ~/.bashrc
    fi

    echo "Remove Vulkan SDK Installer"
    rm -rf "$VULKAN_SDK_VERSION"
    rm -rf "$VULKAN_SDK_OUTPUT_FILE"

    force_exit=true
fi

echo "Checking for VulkanSDK vendor folder"
if [ -d vendor/VulkanSDK ]; then
    echo "Found VulkanSDK vendor folder"
else
    echo "Create VulkanSDK vendor folder"
    mkdir -p vendor/VulkanSDK

    echo "Copy 'include' folder"
    cp -r $VULKAN_SDK/include vendor/VulkanSDK

    echo "Copy 'lib' folder"
    cp -r $VULKAN_SDK/lib vendor/VulkanSDK

    echo "Copy 'glslc.exe'"
    cp $VULKAN_SDK/bin/glslc vendor/VulkanSDK
fi

# Install GIT Hooks
echo "Install GIT Hooks"

echo "Install post-checkout hook"
source=".githooks/post-checkout"
destination=".git/hooks/post-checkout"
cp -f "$source" "$destination"
chmod +x "$destination"

echo "Install post-merge hook"
source=".githooks/post-merge"
destination=".git/hooks/post-merge"
cp -f "$source" "$destination"
chmod +x "$destination"

# generate Makefile
vendor/premake5/premake5 gmake2

# force exit
if [ "$force_exit" = true ]; then
	while true; do
	    read -p "Close/Restart the terminal"
	done
else
    exit
fi
