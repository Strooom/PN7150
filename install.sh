#!/bin/sh
#
# install.sh script for use with arduino_ci
# - this script will be run from the Arduino libraries directory.
# - download the latest version of the Strooom/logging library as myLoggingLibrary
git clone https://github.com/Strooom/logging.git
# mv logging myLoggingLibrary     # not sure if this is needed