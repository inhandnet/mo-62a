#!/bin/sh

# MO-62A SDK setup script
# Initializes the development host for building the MO-62A SDK.

SDK_ROOT=$(cd "$(dirname "$0")"; pwd)

echo "-------------------------------------------------------------------------------"
echo "MO-62A SDK setup script"
echo "SDK root: $SDK_ROOT"
echo "-------------------------------------------------------------------------------"
echo

# ----------------------------------------------------------------------------
# Step 1: Verify host OS
# ----------------------------------------------------------------------------
echo "--------------------------------------------------------------------------------"
echo "Verifying Linux host distribution"

if [ -f /etc/lsb-release ]; then
    . /etc/lsb-release
    if [ "$DISTRIB_ID" = "Ubuntu" ] && [ "$DISTRIB_RELEASE" = "22.04" ]; then
        echo "Ubuntu 22.04 LTS is being used, continuing.."
    else
        echo "WARNING: This SDK has been verified on Ubuntu 22.04 LTS."
        echo "Detected: $DISTRIB_ID $DISTRIB_RELEASE. Proceeding anyway."
    fi
else
    echo "WARNING: Could not detect Linux distribution. Proceeding anyway."
fi
echo "--------------------------------------------------------------------------------"
echo

# ----------------------------------------------------------------------------
# Step 2: Add user to dialout group (for serial port access)
# ----------------------------------------------------------------------------
# Determine the real username even when run via sudo
if [ -n "$SUDO_USER" ]; then
    REAL_USER="$SUDO_USER"
else
    REAL_USER="$USER"
fi

if groups "$REAL_USER" 2>/dev/null | grep -q '\bdialout\b'; then
    echo "User '$REAL_USER' is already in the 'dialout' group."
else
    echo "Adding user '$REAL_USER' to the 'dialout' group (required for serial port access)..."
    sudo usermod -a -G dialout "$REAL_USER"
    if [ $? -eq 0 ]; then
        echo "Done. Please log out and log back in for the change to take effect."
    else
        echo "WARNING: Failed to add '$REAL_USER' to 'dialout'. You may need to do this manually:"
        echo "  sudo usermod -a -G dialout $REAL_USER"
    fi
fi
echo

# ----------------------------------------------------------------------------
# Step 3: Install required host packages (optional)
# ----------------------------------------------------------------------------
while true; do
    read -p "Do you wish to install required host packages? (Y/n) " response
    case "$response" in
        [Nn]*) echo "Host package installation skipped."; break ;;
        *)
            echo "Checking and installing required packages..."
            $SDK_ROOT/bin/setup-package-install.sh
            break ;;
    esac
done
echo

# ----------------------------------------------------------------------------
# Step 4: Set TI_SDK_PATH permanently in ~/.bashrc
# ----------------------------------------------------------------------------
BASHRC="$HOME/.bashrc"
EXPORT_LINE="export TI_SDK_PATH=\"$SDK_ROOT\""
MARKER="# MO-62A SDK"

if grep -q "^export TI_SDK_PATH=" "$BASHRC" 2>/dev/null; then
    OLD=$(grep "^export TI_SDK_PATH=" "$BASHRC")
    if [ "$OLD" = "$EXPORT_LINE" ]; then
        echo "TI_SDK_PATH is already set correctly in $BASHRC:"
        echo "  $EXPORT_LINE"
    else
        echo "Updating TI_SDK_PATH in $BASHRC..."
        sed -i "s|^export TI_SDK_PATH=.*|$EXPORT_LINE|" "$BASHRC"
        echo "  Updated to: $EXPORT_LINE"
    fi
else
    echo "Writing TI_SDK_PATH to $BASHRC..."
    echo "" >> "$BASHRC"
    echo "$MARKER" >> "$BASHRC"
    echo "$EXPORT_LINE" >> "$BASHRC"
    echo "  $EXPORT_LINE"
fi

# Apply to current shell session
export TI_SDK_PATH="$SDK_ROOT"
echo "TI_SDK_PATH is now set to: $TI_SDK_PATH"
echo

# ----------------------------------------------------------------------------
# Step 5: Create /opt symlink for toolchain interpreter compatibility
# ----------------------------------------------------------------------------
# The cross-compilation toolchain binaries have a hardcoded ELF interpreter
# path pointing to /opt/ti-processor-sdk-linux-rt-edgeai-am62a-evm-11.01.07.05/.
# A symlink at that path ensures the toolchain works regardless of where this
# repository is cloned.
SYMLINK_TARGET="/opt/ti-processor-sdk-linux-rt-edgeai-am62a-evm-11.01.07.05"

if [ -L "$SYMLINK_TARGET" ]; then
    CURRENT=$(readlink "$SYMLINK_TARGET")
    if [ "$CURRENT" = "$SDK_ROOT" ]; then
        echo "Toolchain symlink already points to the correct location:"
        echo "  $SYMLINK_TARGET -> $SDK_ROOT"
    else
        echo "Updating toolchain symlink (was pointing to: $CURRENT)..."
        sudo ln -sfn "$SDK_ROOT" "$SYMLINK_TARGET"
        echo "  $SYMLINK_TARGET -> $SDK_ROOT"
    fi
elif [ -e "$SYMLINK_TARGET" ]; then
    echo "WARNING: $SYMLINK_TARGET exists and is not a symlink. Skipping."
    echo "  If you have a real TI SDK installed there, the toolchain will use it."
else
    echo "Creating toolchain symlink..."
    sudo ln -s "$SDK_ROOT" "$SYMLINK_TARGET"
    echo "  $SYMLINK_TARGET -> $SDK_ROOT"
fi
echo

# ----------------------------------------------------------------------------
echo "-------------------------------------------------------------------------------"
echo "MO-62A SDK setup completed!"
echo "You can now build the SDK from: $SDK_ROOT"
echo "-------------------------------------------------------------------------------"
