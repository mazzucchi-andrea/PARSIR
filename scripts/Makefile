# SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
# SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
#
# SPDX-License-Identifier: GPL-3.0-or-later

get-HW-info:
	@if [ ! -d ../setup-data ]; then mkdir -p ../setup-data; fi
	lscpu | grep -i numa > ../setup-data/hw.txt 
	./remove-file.sh

get-CPUs:
	@ nproc --all
