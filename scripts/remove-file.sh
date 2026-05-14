#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
# SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
#
# SPDX-License-Identifier: GPL-3.0-or-later

if [ "$(numactl --hardware | grep available | awk '{print $2}')" = "1" ]; then rm ../setup-data/hw.txt; echo "single NUMA node configuration - the HW config file is being removed\nPARSIR will run with the basic (default) configuration"; fi
