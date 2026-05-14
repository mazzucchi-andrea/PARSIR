# SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
# SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
#
# SPDX-License-Identifier: GPL-3.0-or-later

objdump -D $1 | awk -F 'm' '{print $1}' | awk '{$1=""; sub(/^ /, ""); print}' | sed '0,/text/d' | ./translate-to-binary 
