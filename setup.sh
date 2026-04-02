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

platform=$1

entry_header() {
cat << EOF
-------------------------------------------------------------------------------
TISDK setup script
This script will set up your development host for SDK development.
Parts of this script require administrator priviliges (sudo access).
-------------------------------------------------------------------------------
EOF
}

exit_footer() {
cat << EOF
-------------------------------------------------------------------------------
TISDK setup completed!
Please continue reading the Software Developer's Guide for more information on
how to develop software on the EVM
-------------------------------------------------------------------------------
EOF
}



run_package_install() {
	if [ -f $cwd/bin/setup-package-install.sh ]; then
     		$cwd/bin/setup-package-install.sh
     		check_status
	else
    		echo "setup-package-install.sh does not exist in the bin directory"
    		exit 1
	fi
}

run_target_nfs() {
	if [ -f $cwd/bin/setup-targetfs-nfs.sh ]; then
    		$cwd/bin/setup-targetfs-nfs.sh $platform
    		check_status
	else
    		echo "setup-targetfs-nfs.sh does not exist in the bin directory"
    		exit 1
	fi
}

run_tftp() {
	if [ -f $cwd/bin/setup-tftp.sh ]; then
    		$cwd/bin/setup-tftp.sh $platform
    		check_status
	else
    		echo "setup-tftp.sh does not exist in the bin directory"
    		exit 1
	fi
}

run_minicom() {
	if [ -f $cwd/bin/setup-minicom.sh ]; then
    		$cwd/bin/setup-minicom.sh
    		check_status
	else
    		echo "setup-minicom.sh does not exist in the bin directory"
    		exit 1
	fi
}

run_uboot() {
	if [ -f $cwd/bin/setup-uboot-env.sh ]; then
    		$cwd/bin/setup-uboot-env.sh $platform
    		check_status
	else
    		echo "setup-uboot-env.sh does not exist in the bin directory"
    		exit 1
	fi
}

cwd=`dirname $0`
# Minimum major Linux version for running add-to-group script
min_ver_upper=12

# Publish the TISDK setup header
entry_header

# Make sure that the common.sh file exists
if [ -f $cwd/bin/common.sh ]; then
    . $cwd/bin/common.sh
    get_host_type host
    get_major_host_version host_upper
else
    echo "common.sh does not exist in the bin directory"
    exit 1
fi

if [ -f $cwd/bin/setup-host-check.sh ]; then
    $cwd/bin/setup-host-check.sh
    check_status
else
    echo "setup-host-check.sh does not exist in the bin directory"
    exit 1
fi

# Only execute if the Linux version is above 12.xx
if [ "$host_upper" -gt "$min_ver_upper" -o "$host_upper" -eq "$min_ver_upper" ]; then
    if [ -f $cwd/bin/add-to-group.sh ]; then
        $cwd/bin/add-to-group.sh
        check_status
    else
        echo "add-to-group.sh does not exist in the bin directory"
        exit 1
    fi
fi


while true; do
    read -p "Do you wish to install required host packages (Press (Y) to run, (n) to skip)? " -r response
    case $response in
        [Yy]* ) run_package_install; break;;
        [Nn]* ) echo "host packages installation skipped"; break;;
	   "" ) run_package_install; break;; 
	    * )  echo "Enter Y/n";;
    esac
done

while true; do
    read -p "Do you wish to run nfs setup (Press (Y) to run, (n) to skip) ? " -r response
    case $response in
        [Yy]* ) run_target_nfs; break;;
        [Nn]* ) echo "nfs setup skipped"; break;;
	   "" ) run_target_nfs; break;;
	    * )  echo "Enter Y/n";;
    esac
done

while true; do
    read -p "Do you wish to run tftp setup (Press (Y) to run, (n) to skip) ? " -r response
    case $response in
        [Yy]* ) run_tftp; break;;
        [Nn]* ) echo "tftp setup skipped"; break;;
	   "" ) run_tftp; break;;
	    * )  echo "Enter Y/n";;
    esac
done

while true; do
    read -p "Do you wish to run minicom setup (Press (Y) to run, (n) to skip) ? " -r response
    case $response in
        [Yy]* ) run_minicom; break;;
        [Nn]* ) echo "minicom setup skipped"; break;;
	   "" ) run_minicom; break;;
	    * )  echo "Enter Y/n";;
    esac
done

while true; do
    read -p "Do you wish to run uboot setup (Press (Y) to run, (n) to skip) ? " -r response
    case $response in
        [Yy]* ) run_uboot; break;;
        [Nn]* ) echo "uboot setup skipped"; break;;
	   "" ) run_uboot; break;;
	    * )  echo "Enter Y/n";;
    esac
done

# Publish the TISDK exit header
exit_footer
