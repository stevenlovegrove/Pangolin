#!/bin/bash

# exit when any command fails
set -e

MANAGERS=(dnf apt port vcpkg brew)
MANAGER=""
LIST=0
VERBOSE=0
DRYRUN=0
UPDATE=0
REQUIRED_RECOMMENDED_ALL=1
SUDO=""

PKGS_UPDATE=""
PKGS_REQUIRED=()
PKGS_RECOMMENDED=()
PKGS_ALL=()
PKGS_OPTIONS=()

# Parse Command line
PARAMS=""
while (( "$#" )); do
  case "$1" in
    -v|--verbose)
      VERBOSE=1
      shift
      ;;
    -d|--dry-run)
      DRYRUN=1
      shift
      ;;
    -l|--list)
      LIST=1
      shift
      ;;
    -u|--update-package-list)
      UPDATE=1
      shift
      ;;
    -m|--package-manager)
      if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
        MANAGER=($2)
        shift 2
      else
        echo "Error: Argument for $1 is missing" >&2
        exit 1
      fi
      ;;
    -h|--help)
      echo "$0 [-vh] [-m package-manager-list] [required|recommended|all]"
      echo "  -m, --package-manager:     preferred package manager order (default: \"${MANAGERS[*]}\")"
      echo "  -v, --verbose:             verbose output"
      echo "  -d, --dry-run:             print actions, but do not execute"
      echo "  -l, --list:                just list the packages to install"
      echo "  -u, --update-package-list: update package manager package list"
      echo "  -h, --help:                this help message"
      echo " (required|recommended|all) the set of dependencies to select."
      exit 0
      ;;
    -*|--*=) # unsupported flags
      echo "Error: Unsupported flag $1" >&2
      exit 1
      ;;
    *) # preserve positional arguments
      PARAMS="$1"
      shift
      ;;
  esac
done

# Make lower case
PARAMS=$(echo "$PARAMS" | tr '[:upper:]' '[:lower:]' | tr -s "[:blank:]")

# Work out which set of dependencies we're installing
case "$PARAMS" in
  required)
    REQUIRED_RECOMMENDED_ALL=0
    if ((VERBOSE > 0)); then echo "Selecting required dependencies only."; fi
    ;;
  ""|recommended)
    REQUIRED_RECOMMENDED_ALL=1
    if ((VERBOSE > 0)); then echo "Selecting required+recommended dependencies."; fi
    ;;
  all)
    REQUIRED_RECOMMENDED_ALL=2
    if ((VERBOSE > 0)); then echo "Selecting all available dependencies."; fi
    ;;
  *)
    echo "Unrecognized positional argument \"$PARAMS\". Expecting one of (required,recommended [default],all)"
    exit 1
    ;;
esac

# Find an available package manager from the preferred list
# if one has not already been selected manually.
if [ -z "$MANAGER" ]
then
  for m in ${MANAGERS[@]}
  do
      if [ -x "$(command -v $m)" ]; then
          MANAGER="$m"
          break
      fi
  done
fi

# If no package manager is found, exit
if [ -z "$MANAGER" ]
then
      echo "Error: No preferred package managers from list [${MANAGERS[*]}] found. Use -m to select manually." >&2
      exit 1
fi
if ((VERBOSE > 0)); then echo "Using \"$MANAGER\" package manager (select another using -m)"; fi

# Setup prereq commands and packages.
if [[ "$MANAGER" == "apt" ]]; then
    SUDO="sudo"
    PKGS_UPDATE="apt update"
    PKGS_OPTIONS+=(install --no-install-suggests --no-install-recommends)
    if ((DRYRUN > 0));  then PKGS_OPTIONS+=(--dry-run); SUDO=""; fi
    PKGS_REQUIRED+=(libgl1-mesa-dev libwayland-dev libxkbcommon-dev wayland-protocols libegl1-mesa-dev)
    PKGS_REQUIRED+=(libc++-dev libglew-dev libeigen3-dev cmake g++ ninja-build)
    PKGS_RECOMMENDED+=(libjpeg-dev libpng-dev)
    PKGS_RECOMMENDED+=(libavcodec-dev libavutil-dev libavformat-dev libswscale-dev libavdevice-dev)
    PKGS_ALL+=(libdc1394-22-dev libraw1394-dev libopenni-dev python3.9-dev python3-distutils)
elif [[ "$MANAGER" == "dnf" ]]; then
    SUDO="sudo"
    PKGS_UPDATE="dnf check-update"
    PKGS_OPTIONS+=(install)
    PKGS_REQUIRED+=(wayland-devel libxkbcommon-devel g++ ninja-build)
    PKGS_REQUIRED+=(glew-devel eigen3 cmake)
    PKGS_RECOMMENDED+=(libjpeg-devel libpng-devel OpenEXR-devel)
    PKGS_ALL+=(libdc1394-22-devel libraw1394-devel librealsense-devel openni-devel)
    if ((DRYRUN > 0));  then
        MANAGER="echo $MANAGER"
        SUDO=""
    fi
elif [[ "$MANAGER" == "port" ]]; then
    SUDO="sudo"
    PKGS_UPDATE="port sync -q"
    if ((DRYRUN > 0));  then PKGS_OPTIONS+=(-y); SUDO=""; fi
    PKGS_OPTIONS+=(-N install -q)
    PKGS_REQUIRED+=(glew eigen3-devel cmake +gui ninja)
    PKGS_RECOMMENDED+=(jpeg libpng openexr tiff ffmpeg-devel lz4 zstd py37-pybind11 catch2)
    PKGS_ALL+=(libdc1394 openni)
elif [[ "$MANAGER" == "brew" ]]; then
    PKGS_OPTIONS+=(install)
    if ((VERBOSE > 0)); then PKGS_OPTIONS+=(--verbose); fi
    PKGS_REQUIRED+=(glew eigen cmake ninja)
    PKGS_RECOMMENDED+=(libjpeg libpng openexr libtiff ffmpeg lz4 zstd catch2)
    # Brew doesn't have a dryrun option
    if ((DRYRUN > 0));  then
        MANAGER="echo $MANAGER"
    fi
elif [[ "$MANAGER" == "vcpkg" ]]; then
    # TODO: this should be a config option somehow...
    PKGS_OPTIONS+=(install --triplet=x64-windows )
    if ((DRYRUN > 0));  then PKGS_OPTIONS+=(--dry-run); fi
    PKGS_REQUIRED+=(glew eigen3)
    PKGS_RECOMMENDED+=(libjpeg-turbo libpng openexr tiff ffmpeg lz4 zstd python3 Catch2)
    PKGS_ALL+=(openni2 realsense2)
else
    echo "Error: Don't know how to use \"$MANAGER\", please fix the script." >&2
    exit 1
fi

if ((REQUIRED_RECOMMENDED_ALL < 2)); then PKGS_ALL=(); fi
if ((REQUIRED_RECOMMENDED_ALL < 1)); then PKGS_RECOMMENDED=(); fi

PACKAGES=( "${PKGS_REQUIRED[*]}" "${PKGS_RECOMMENDED[*]}" "${PKGS_ALL[*]}" )

if ((LIST > 0)); then
    echo "${PACKAGES[*]}"
    exit 0
fi

if ((UPDATE > 0)); then
    if ((VERBOSE > 0)); then echo "Requesting \"$MANAGER\" package update."; fi
    $SUDO $PKGS_UPDATE
fi

if ((VERBOSE > 0)); then echo "Requesting install of: ${PACKAGES[*]}"; fi

# Install
$SUDO $MANAGER ${PKGS_OPTIONS[*]} ${PACKAGES[*]}
