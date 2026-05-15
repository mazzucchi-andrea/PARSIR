#!/bin/bash

# SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
# SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
#
# SPDX-License-Identifier: GPL-3.0-or-later

echo $1
echo $2

if [ -d "$1" ]; then
  echo "directory $1 exists"
else
echo "creating dir $1"
mkdir $1
fi

touch $2; rm $2; touch $2

