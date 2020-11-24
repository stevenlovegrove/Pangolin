#!/bin/bash

# exit when any command fails
set -e

MANAGERS=(dnf apt-get port vcpkg brew)
MANAGER=""
VERBOSE=0
DRYRUN=0
UPDATE=0
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
    -u|--udpate-package-list)
      UDPATE=1
      shift
      ;;
    -m|--package-manager)
      if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
        MANAGERS=($2)
        shift 2
      else
        echo "Error: Argument for $1 is missing" >&2
        exit 1
      fi
      ;;
    -h|--help)
      echo "$0 [-vh] [-m package-manager-list]"
      echo "  -m, --package-manager:     preferred package manager order (default: \"${MANAGERS[*]}\")"
      echo "  -v, --verbose:             verbose output"
      echo "  -d, --dry-run:             print actions, but do not execute"
      echo "  -u, --udpate-package-list: update package manager package list"
      echo "  -h, --help:                this help message"
      exit 0
      ;;
    -*|--*=) # unsupported flags
      echo "Error: Unsupported flag $1" >&2
      exit 1
      ;;
    *) # preserve positional arguments
      PARAMS="$PARAMS $1"
      shift
      ;;
  esac
done

# Find an available package manager from the preferred list
for m in ${MANAGERS[@]}
do
    if [ -x "$(command -v $m)" ]; then
        MANAGER="$m"
        break
    fi
done

# If no package manager is found, exit
if [ -z "$MANAGER" ]
then
      echo "Error: No preferred package managers from list [${MANAGERS[*]}] found. Use -m to select manually." >&2
      exit 1
fi

if ((VERBOSE > 0)); then echo "Using \"$MANAGER\" package manager (select another using -m)"; fi

# Setup prereq commands and packages.
if [[ "$MANAGER" == "apt-get" ]]; then
    SUDO="sudo"
    PKGS_UPDATE="apt-get -qq update"
    PKGS_OPTIONS+=(install --no-install-suggests --no-install-recommends)
    if ((DRYRUN > 0));  then PKGS_OPTIONS+=(--dry-run); fi
    if ((VERBOSE = 0)); then PKGS_OPTIONS+=(--qq); fi
    PKGS_REQUIRED+=(libgl1-mesa-dev libwayland-dev libxkbcommon-dev wayland-protocols libegl1-mesa-dev)
    PKGS_REQUIRED+=(libc++-dev libglew-dev libeigen3-dev libjpeg-dev libpng-dev cmake)
elif [[ "$MANAGER" == "port" ]]; then
    SUDO="sudo"
    PKGS_UPDATE="port selfupdate -q"
    PKGS_OPTIONS+=(-N install -q)
    if ((DRYRUN > 0));  then PKGS_OPTIONS+=(-y); fi
    if ((VERBOSE = 0)); then PKGS_OPTIONS+=(--q); fi
    PKGS_REQUIRED+=(glew eigen3-devel jpeg libpng cmake)
elif [[ "$MANAGER" == "brew" ]]; then
    PKGS_UPDATE="brew update"
    PKGS_OPTIONS+=(install)
    if ((VERBOSE > 0)); then PKGS_OPTIONS+=(--verbose); fi
    PKGS_REQUIRED+=(glew eigen libjpeg libpng cmake)
else
    echo "Error: Don't know how to use \"$MANAGER\", please fix the script." >&2
    exit 1
fi

if ((UPDATE > 0)); then
    $PKGS_UPDATE
fi

# Install
$SUDO $MANAGER ${PKGS_OPTIONS[*]} ${PKGS_REQUIRED[*]}
