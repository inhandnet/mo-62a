#!/bin/sh

# This distribution contains contributions or derivatives under copyright
# as follows:
#
# Copyright (c) 2023, Texas Instruments Incorporated
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# - Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# - Neither the name of Texas Instruments nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cwd=`dirname $0`
. $cwd/common.sh

entry_header() {
cat << EOF
-------------------------------------------------------------------------------
setup package script
This script will make sure you have the proper host support packages installed
This script requires administrator priviliges (sudo access) if packages are to be installed.
-------------------------------------------------------------------------------
EOF
}

exit_footer() {
cat << EOF
--------------------------------------------------------------------------------
Package verification and installation successfully completed
--------------------------------------------------------------------------------
EOF
}

cwd=`dirname $0`
. $cwd/common.sh

entry_header

packages_to_install="xinetd tftpd nfs-kernel-server minicom build-essential libncurses5-dev autoconf automake dos2unix screen lrzsz lzop flex libssl-dev"

get_major_host_version host_major_version

# Starting with Ubuntu 14.04 the package name for uboot-mkimage has changed
# to u-boot-tools

if [ $host_major_version -ge 14 ]; then
    packages_to_install="$packages_to_install u-boot-tools"
else
    packages_to_install="$packages_to_install uboot-mkimage"

fi

cmd="sudo apt-get install "

# Check and only install the missing packages
for i in $packages_to_install; do
	is_it_installed=`dpkg-query -s $i 2>/dev/null`
	if [ "$?" -ne "0" ]; then
		needs_installation=`echo $needs_installation`" "$i
		new_cmd=`echo $cmd`" "$i
		cmd=$new_cmd
	fi
done

if [ "$needs_installation" = "" ]; then
    echo "System has required packages!"
else
    echo "System requires packages $needs_installation to be installed"

    echo "Installation requires you to have administrator priviliges (sudo access) "
    echo "on your host. Do you have administrator privilieges?"

    # Force the user to answer.  Maybe the user does not want to continue
    while true;
    do
        read -p "Type 'y' to continue or 'n' to exit the installation: " REPLY
        if [ "$REPLY" = 'y' -o "$REPLY" = 'n' ]; then
            break;
        fi
    done

    if [ "$REPLY" = 'n' ]; then
        echo "Installation is aborted by user"
        exit 1
    fi

    echo "Performing $cmd"
    $cmd
    check_status
fi
# Print the exit statement to the console
exit_footer
