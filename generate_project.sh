#!/bin/bash

# install prerequisites
# TODO: silent installation
# sudo apt install build-essential
# sudo apt install libx11-dev
# sudo apt install libxcursor-dev
# sudo apt install libxrandr-dev
# sudo apt install libxinerama-dev
# sudo apt install libxi-dev

# Vulkan SDK
# TODO
VULKAN_SDK_INSTALL_URL="https://sdk.lunarg.com/sdk/download/latest/linux/vulkan_sdk.tar.xz"
VULKAN_SDK_OUTPUT_FILE="VulkanSDK-Installer.tar.xz"

# TODO
echo "Vulkan SDK is already installed"
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

echo $VULKAN_SDK
echo "Create VulkanSDK vendor folder"

echo "Copy 'include' folder"
# TODO

echo "Copy 'lib' folder"
# TODO

echo "Copy 'glslc.exe'"
# TODO

echo "Remove Vulkan SDK Installer"
rm -rf "$VULKAN_SDK_VERSION"
rm -rf "$VULKAN_SDK_OUTPUT_FILE"

# TODO: uncomment
# vendor/premake5/premake5 gmake2