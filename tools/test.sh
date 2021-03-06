#!/bin/sh

# This runs a couple of cheap tests on knxd.

set -ex
export PATH="$(pwd)/src/examples/.libs:$(pwd)/src/examples:$(pwd)/src/server/.libs:$(pwd)/src/server:$PATH"

EF=$(tempfile)

# first test argument handling
if knxd --stop-right-now >$EF 2>&1; then
	echo "Bad argument A" >&2
	cat $EF 2>&1
	exit 1
fi

if knxd --stop-right-now -b dummy: >$EF 2>&1; then
	echo "Bad argument B" >&2
	cat $EF 2>&1
	exit 1
fi

if ! knxd --stop-right-now -b dummy: -b dummy: >$EF 2>&1; then
	echo "Bad argument C" >&2
	cat $EF 2>&1
	exit 1
fi

if knxd --stop-right-now -b dummy: -b dummy: --tpuarts-disch-reset >$EF 2>&1; then
	echo "Bad argument D" >&2
	cat $EF 2>&1
	exit 1
fi

if knxd --stop-right-now -b dummy: --tpuarts-disch-reset -b dummy: >$EF 2>&1; then
	echo "Bad argument E" >&2
	cat $EF 2>&1
	exit 1
fi

if knxd --stop-right-now -T -b dummy: -b dummy: >$EF 2>&1; then
	echo "Bad argument F" >&2
	cat $EF 2>&1
	exit 1
fi

S1=$(tempfile); rm $S1
S2=$(tempfile); rm $S2
S3=$(tempfile); rm $S3
L1=$(tempfile)
L2=$(tempfile)
E1=$(tempfile)
E2=$(tempfile)

knxd -t 0xfffc -f 9 -e 3.2.1 -u$S1 -u$S2 -u$S3 -bdummy: &
KNXD=$!
trap 'rm -f $L1 $L2 $E1 $E2 $EF; kill $KNXD' 0 1 2

sleep 1
knxtool grouplisten local:$S2 1/2/3 >$L1 2>$E1 &
PL1=$!
knxtool vbusmonitor1 local:$S2 >$L2 2>$E2 &
PL2=$!
# will die by itself when the server terminates

sleep 1
knxtool groupswrite local:$S1 1/2/3 4
sleep 1
knxtool groupwrite local:$S2 1/2/3 4 5 6

sleep 1
kill $KNXD
sleep 1
kill $PL1 $PL2 || true
trap 'rm -f $L1 $L2 $E1 $E2 $EF' 0 1 2
sleep 1
#ls -l $L1 $L2 $E1 $E2
#cat $L1 $L2 $E1 $E2
diff -u "$(dirname "$0")"/logs/listen $L1
diff -u "$(dirname "$0")"/logs/monitor $L2

set +ex
#sed -e 's/^/O grouplisten: /' <$L1
#sed -e 's/^/O vbusmonitor1: /' <$L2
sed -e 's/^/E grouplisten: /' <$E1
sed -e 's/^/E vbusmonitor1: /' <$E2

