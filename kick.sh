#!/bin/sh

# kick.sh - Run specific or all kick scripts based on parameter

case "$1" in
phold)
	echo "Running kick_phold.sh..."
	./kick_phold.sh
	;;
pcs)
	echo "Running kick_pcs.sh..."
	./kick_pcs.sh
	;;
highway)
	echo "Running kick_highway.sh..."
	./kick_highway.sh
	;;
"")
	echo "No parameter provided. Running all scripts..."
	./kick_phold.sh
	./kick_pcs.sh
	./kick_highway.sh
	;;
*)
	echo "Error: Unknown parameter '$1'"
	echo "Usage: $0 [phold|pcs|highway]"
	echo "  phold   - Run kick_phold.sh"
	echo "  pcs     - Run kick_pcs.sh"
	echo "  highway - Run kick_highway.sh"
	echo "  (none)  - Run all scripts"
	exit 1
	;;
esac
