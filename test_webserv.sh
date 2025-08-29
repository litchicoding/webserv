#!/usr/bin/env bash
set -euo pipefail

BASE_URL1="http://127.0.0.1:8080"
BASE_URL2="http://127.0.0.1:8081"

# Couleurs
CLR_RST='\033[0m'
CLR_GRN='\033[32m'
CLR_RED='\033[31m'

ok()   { echo -e "   ${CLR_GRN}✅  $*${CLR_RST}"; }
fail() { echo -e "   ${CLR_RED}❌  $*${CLR_RST}"; }

echo "Running Webserv full tests..."

test_number=1
do_test() {
    local method=$1
    local url=$2
    local expected=$3

    status=$(curl -s -o /dev/null -w "%{http_code}" -X "$method" "$url")
    echo "[Test $test_number] $method $url → expected $expected, got $status"
    if [[ "$status" == "$expected" ]]; then
        ok "Passed"
    else
        fail "Failed"
    fi
    test_number=$((test_number+1))
}

# 0) Sanity
do_test GET "$BASE_URL1/" 200
do_test GET "$BASE_URL2/" 200

# 1) GET static pages
do_test GET "$BASE_URL1/index.html" 200
do_test GET "$BASE_URL1/nonexistent.html" 404

# 2) Method not allowed
do_test POST "$BASE_URL1/" 405

# 3) Autoindex
do_test GET "$BASE_URL1/upload/" 200
do_test GET "$BASE_URL1/upload" 301

# 4) Upload (multipart)
# On teste juste que POST /upload retourne quelque chose
do_test POST "$BASE_URL1/upload" 201 || echo "  Note: multipart POST may fail if not implemented"

# 5) DELETE
do_test DELETE "$BASE_URL1/upload/nonexistent.txt" 404

# 6) Max body size
do_test POST "$BASE_URL1/upload" 413 || echo "  Note: 413 may fail if client_max_body_size not handled"

# 7) POST /data
do_test POST "$BASE_URL1/data" 200 || echo "  Note: application/x-www-form-urlencoded handling"

# 8) CGI
do_test GET "$BASE_URL1/cgi-bin/hello.py" 200
do_test POST "$BASE_URL1/cgi-bin/form.php" 200 || echo "  Note: CGI POST may fail if not implemented"

# 9) Multi-port
do_test GET "$BASE_URL2/" 200

# 10) Stress test (20 GET / in parallel)
echo "[Test $test_number] Stress test 20 GET /"
parallel_status=0
for i in $(seq 1 20); do
    status=$(curl -s -o /dev/null -w "%{http_code}" "$BASE_URL1/")
    if [[ "$status" != "200" ]]; then
        parallel_status=1
        echo "  GET / iteration $i → $status"
    fi
done
if [[ "$parallel_status" -eq 0 ]]; then
    ok "All 20 GET / requests returned 200"
else
    fail "Some GET / requests failed"
fi
