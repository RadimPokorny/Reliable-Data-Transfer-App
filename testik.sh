#!/bin/bash

EXEC="./ipk-rdt"
PORT1=9000
PORT2=9999
HOST="127.0.0.1"
HOST6="::1"
TEST_DIR="test_data"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'
GRAY='\033[1;30m'
BLUE='\033[1;36m'

trap 'sudo tc qdisc del dev lo root 2>/dev/null' EXIT INT TERM

make -s
mkdir -p $TEST_DIR

echo "========================================"
echo " Spouštím automatizované testy ipk-rdt  "
echo "========================================"

check_result() {
    local test_name=$1
    local in_file=$2
    local out_file=$3
    local time=$4

    if cmp -s "$in_file" "$out_file"; then
        echo -e "[ ${GREEN}PASS${NC} ] $test_name ${GRAY}-${time}ms-${NC}"
    else
        echo -e "[ ${RED}FAIL${NC} ] $test_name (Soubory se liší!)"
        md5sum "$in_file" "$out_file"
    fi
}

check_result_s() {
    local test_name=$1
    local in_file=$2
    local out_file=$3
    local time=$4

    if cmp -s "$in_file" "$out_file"; then
        echo -e "[ ${GREEN}PASS${NC} ] $test_name ${BLUE}-${time}ms-${NC}"
    else
        echo -e "[ ${RED}FAIL${NC} ] $test_name (Soubory se liší!)"
        md5sum "$in_file" "$out_file"
    fi
}


echo "----------------------------------------"
echo "            Bez úprav sítě              "
echo "----------------------------------------"
# ==========================================
# TEST 1: Přenos malého souboru
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test1_in.bin bs=1K count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test1_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test1_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result "Běžný soubor (10 KB)" "$TEST_DIR/test1_in.bin" "$TEST_DIR/test1_out.bin" "$DURATION_MS"

# ==========================================
# TEST 2: Přenos prázdného souboru
# ==========================================
touch $TEST_DIR/test2_in.bin

$EXEC -s -p $PORT1 -o $TEST_DIR/test2_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test2_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result "Prázdný soubor (0 B)" "$TEST_DIR/test2_in.bin" "$TEST_DIR/test2_out.bin" "$DURATION_MS"

# ==========================================
# TEST 3: Přenos skrz rouru
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test3_in.bin bs=1K count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test3_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
cat $TEST_DIR/test3_in.bin | $EXEC -c -a $HOST -p $PORT1 > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result "Soubor skrz rouru (10 KB)" "$TEST_DIR/test3_in.bin" "$TEST_DIR/test3_out.bin" "$DURATION_MS"

# ==========================================
# TEST 4: Velký soubor
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test4_in.bin bs=1M count=20 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test4_out.bin -w 5 > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test4_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result "Velký soubor (20 MB)" "$TEST_DIR/test4_in.bin" "$TEST_DIR/test4_out.bin" "$DURATION_MS"

# ==========================================
# TEST 5: IPv6 test
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test5_in.bin bs=1K count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test5_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST6 -p $PORT1 -i $TEST_DIR/test5_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result "IPv6 test (10 KB)" "$TEST_DIR/test5_in.bin" "$TEST_DIR/test5_out.bin" "$DURATION_MS"

# ==========================================
# TEST 6: Rozdílné porty
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test6_in.bin bs=1K count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test6_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

$EXEC -c -a $HOST -p $PORT2 -i $TEST_DIR/test6_in.bin > /dev/null 2>&1

wait $SERVER_PID

if [ $? -ne 0 ]; then
    echo -e "[ ${GREEN}PASS${NC} ] Test rozdílných portů (10 KB)"
else
    echo -e "[ ${RED}FAIL${NC} ] Test rozdílných portů (10 KB)"
fi

echo "----------------------------------------"
echo "          Hodně zatížená síť            "
echo "----------------------------------------"
sudo tc qdisc add dev lo root netem delay 100ms 50ms distribution normal loss 20% corrupt 5% duplicate 5% reorder 10%
# ==========================================
# TEST 1: Přenos malého souboru
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test1_in.bin bs=1K count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test1_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test1_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result "Běžný soubor (10 KB)" "$TEST_DIR/test1_in.bin" "$TEST_DIR/test1_out.bin" "$DURATION_MS"

# ==========================================
# TEST 2: Přenos prázdného souboru
# ==========================================
touch $TEST_DIR/test2_in.bin

$EXEC -s -p $PORT1 -o $TEST_DIR/test2_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test2_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result "Prázdný soubor (0 B)" "$TEST_DIR/test2_in.bin" "$TEST_DIR/test2_out.bin" "$DURATION_MS"

# ==========================================
# TEST 3: Přenos skrz rouru
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test3_in.bin bs=1K count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test3_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
cat $TEST_DIR/test3_in.bin | $EXEC -c -a $HOST -p $PORT1 > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result "Soubor skrz rouru (10 KB)" "$TEST_DIR/test3_in.bin" "$TEST_DIR/test3_out.bin" "$DURATION_MS"

# ==========================================
# TEST 4: Velký soubor
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test4_in.bin bs=1M count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test4_out.bin -w 5 > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test4_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result "Velký soubor (20 MB)" "$TEST_DIR/test4_in.bin" "$TEST_DIR/test4_out.bin" "$DURATION_MS"

# ==========================================
# TEST 5: IPv6 test
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test5_in.bin bs=1K count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test5_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST6 -p $PORT1 -i $TEST_DIR/test5_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result "IPv6 test (10 KB)" "$TEST_DIR/test5_in.bin" "$TEST_DIR/test5_out.bin" "$DURATION_MS"

# ==========================================
# TEST 6: Rozdílné porty
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test6_in.bin bs=1K count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test6_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

$EXEC -c -a $HOST -p $PORT2 -i $TEST_DIR/test6_in.bin > /dev/null 2>&1

wait $SERVER_PID

if [ $? -ne 0 ]; then
    echo -e "[ ${GREEN}PASS${NC} ] Test rozdílných portů (10 KB)"
else
    echo -e "[ ${RED}FAIL${NC} ] Test rozdílných portů (10 KB)"
fi

sudo tc qdisc del dev lo root


echo "----------------------------------------"
echo "              Speed testy               "
echo "----------------------------------------"
# ==========================================
# TEST 1: Přenos malého souboru
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test1_in.bin bs=1K count=1 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test1_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test1_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result_s "Malý soubor (1 KB)" "$TEST_DIR/test1_in.bin" "$TEST_DIR/test1_out.bin" "$DURATION_MS"

# ==========================================
# TEST 2: Přenos malého souboru
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test2_in.bin bs=1K count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test2_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test2_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result_s "Středně malý soubor (10 KB)" "$TEST_DIR/test2_in.bin" "$TEST_DIR/test2_out.bin" "$DURATION_MS"

# ==========================================
# TEST 3: Přenos středního souboru
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test3_in.bin bs=10K count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test3_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test3_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result_s "Střední soubor (100 KB)" "$TEST_DIR/test3_in.bin" "$TEST_DIR/test3_out.bin" "$DURATION_MS"

# ==========================================
# TEST 4: Přenos většího souboru
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test4_in.bin bs=1M count=1 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test4_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test4_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result_s "Větší soubor (1 MB)" "$TEST_DIR/test4_in.bin" "$TEST_DIR/test4_out.bin" "$DURATION_MS"

# ==========================================
# TEST 5: Přenos velkého souboru
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test5_in.bin bs=1M count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test5_out.bin > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test5_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result_s "Velký soubor (10 MB)" "$TEST_DIR/test5_in.bin" "$TEST_DIR/test5_out.bin" "$DURATION_MS"

# ==========================================
# TEST 6: Přenos obřího souboru
# ==========================================
dd if=/dev/urandom of=$TEST_DIR/test6_in.bin bs=10M count=10 2>/dev/null

$EXEC -s -p $PORT1 -o $TEST_DIR/test6_out.bin  -w 20 > /dev/null 2>&1 &
SERVER_PID=$!

START_TIME=$(date +%s%N)
$EXEC -c -a $HOST -p $PORT1 -i $TEST_DIR/test6_in.bin > /dev/null 2>&1 &
CLIENT_PID=$!

wait $CLIENT_PID
DURATION_MS=$((($(date +%s%N) - START_TIME) / 1000000 ))

wait $SERVER_PID

check_result_s "Obří soubor (100 MB)" "$TEST_DIR/test6_in.bin" "$TEST_DIR/test6_out.bin" "$DURATION_MS"

# ==========================================
# ÚKLID
# ==========================================
echo "========================================"
rm -rf $TEST_DIR