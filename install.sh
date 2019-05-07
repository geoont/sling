#!/bin/bash

# Exit on errors.
set -e

echo "========================================================================="
echo "Installing SLING development environment"
echo "========================================================================="

# Install packages.
echo
echo "=== Install SLING dependencies"
PYVER=3.5
PYPKGS="python${PYVER} python${PYVER}-dev python3-pip"
PKGS="pkg-config zip g++ zlib1g-dev unzip ${PYPKGS}"
sudo apt-get install ${PKGS}

# Install bazel.
BAZELVER=0.13.0
BAZELSH=bazel-${BAZELVER}-installer-linux-x86_64.sh
BAZELREPO=https://github.com/bazelbuild/bazel
BAZELURL=${BAZELREPO}/releases/download/${BAZELVER}/${BAZELSH}
if ! which bazel > /dev/null; then
  echo
  echo "=== Install Bazel build system"
  wget -P ${BAZELURL}
  chmod +x /tmp/${BAZELSH}
  sudo /tmp/${BAZELSH}
  rm /tmp/${BAZELSH}
fi

# Build SLING.
echo
echo "=== Build SLING"
tools/buildall.sh

# Install SLING Python API.
echo
echo "=== Install SLING Python API"
SLINGPKG=/usr/lib/python3/dist-packages/sling

if [[ -L "/usr/lib/python2.7/dist-packages/sling" ]]; then
  echo "Removing deprecated SLING Python 2.7 package"
  sudo rm /usr/lib/python2.7/dist-packages/sling
fi
if [[ -L "/usr/local/lib/python2.7/dist-packages/sling" ]]; then
  echo "Removing deprecated SLING Python 2.7 local package"
  sudo rm /usr/local/lib/python2.7/dist-packages/sling
fi

if [[ $(sudo pip3 --disable-pip-version-check freeze | grep "sling==") ]]; then
  echo "Removing existing SLING pip package"
  sudo -H pip3 --disable-pip-version-check uninstall sling
fi

if [[ -x "${SLINGPKG}" ]]; then
  echo "SLING Python package already installed"
else
  echo "Adding link for SLING Python package"
  sudo ln -s $(realpath python) ${SLINGPKG}
fi

# Done.
echo
echo "=== SLING is now installed."

