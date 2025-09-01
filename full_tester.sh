#!/usr/bin/env bash
# =============================================================================
#  FULL TESTER: POST ➜ oversized ➜ DELETE                             🧪🕵️
# -----------------------------------------------------------------------------
# 1. Creates N small upload               📤  (expect 201)
# 2. Sends one oversized payload           🚫  (expect 413)
# 3. Deletes the created upload           🗑️  (expect 200 and removal)
# 4. Displays the remaining files count    📂
# -----------------------------------------------------------------------------
#  Keeps colours & emojis from original scripts.
# =============================================================================

set -euo pipefail

# ────────────────────────────────────────────────────────────────────────────
# Colour palette (ANSI)
# ────────────────────────────────────────────────────────────────────────────
CLR_RST='\033[0m'; CLR_BLD='\033[1m'
CLR_RED='\033[31m'; CLR_GRN='\033[32m'; CLR_YLW='\033[33m'
CLR_BLU='\033[34m'; CLR_MAG='\033[35m'; CLR_CYN='\033[36m'

# Emojis
ICON_OK="✅";   ICON_FAIL="❌"; ICON_INFO="ℹ️";   ICON_UP="📤"; 
ICON_DEL="🗑️";  ICON_BIG="🚫";  ICON_FILE="📄"; ICON_STAR="✨"

# ────────────────────────────────────────────────────────────────────────────
# Pretty helpers
# ────────────────────────────────────────────────────────────────────────────
ok()   { echo -e "   ${CLR_GRN}${ICON_OK}  $*${CLR_RST}"; }
fail() { echo -e "   ${CLR_RED}${ICON_FAIL}  $*${CLR_RST}"; }

# Title banner for the script itself
echo -e "${CLR_BLU}${ICON_STAR}${ICON_STAR}${ICON_STAR}  Webserv functional tester  ${ICON_STAR}${ICON_STAR}${ICON_STAR}${CLR_RST}"

# ────────────────────────────────────────────────────────────────────────────
# CONFIGURATION
# ────────────────────────────────────────────────────────────────────────────
POST_URL="http://localhost:8080/upload"   # Endpoint for POST
DEL_BASE="http://localhost:8080"          # Prefix for DELETE (concatenate Location)
UPLOAD_DIR="www/upload"                  # Physical upload directory

N=5                # Number of objects to create & delete
SMALL_SIZE=512     # Bytes for normal payloads
BIG_SIZE_MB=0.9     # > client_max_body_size (10 MiB) ⇒ expect 413

# helper for pretty section titles
step() { echo -e "\n${CLR_CYN}$1${CLR_RST}"; }

# ────────────────────────────────────────────────────────────────────────────
# 0) Sanity check
# ────────────────────────────────────────────────────────────────────────────
step "${ICON_INFO}  Sanity check server…"
# Accept a broad range because GET on /upload may not be allowed.
if curl -s -o /dev/null -w "%{http_code}\n" "$POST_URL" | grep -qE '200|301|404|405|500'; then
  ok "Server is up"
else
  fail "Server not responding on $POST_URL"
  exit 1
fi

# Create temporary workspace for payloads & headers
TMP=$(mktemp -d)

# ────────────────────────────────────────────────────────────────────────────
# 1) POST N SMALL PAYLOADS
# ────────────────────────────────────────────────────────────────────────────
step "${ICON_UP}  Creating $N files (${SMALL_SIZE} o)…"
seq 1 "$N" | while read -r i; do
  head -c "$SMALL_SIZE" /dev/urandom > "$TMP/payload_$i.bin"
done

# send them (parallel) and record headers
printf "%s\n" "$TMP"/payload_*.bin | \
  xargs -P4 -I{} sh -c '
    curl -s -D - -o /dev/null -X POST "$0" --data-binary "@{}" > "{}.hdr"
  ' "$POST_URL"

# collect successful locations
LOCATIONS=()
for h in "$TMP"/*.hdr; do
  loc=$(grep -i "^Location:" "$h" | cut -d" " -f2- | tr -d "\r")
  status=$(grep -m1 -oE "HTTP/1\.[01] 201" "$h" || true)
  if [[ -n $loc && -n $status ]]; then
    LOCATIONS+=("$loc")
  else
    echo -e "   ${CLR_RED}${ICON_FAIL}${CLR_RST} $(basename "$h") (status/location missing)"
  fi
done

# Print list of created files (compact)
if [[ ${#LOCATIONS[@]} -eq $N ]]; then
  ok "${#LOCATIONS[@]} / $N upload created successfully"
  for loc in "${LOCATIONS[@]}"; do echo -e "      ${ICON_FILE} $(basename "$loc")"; done
else
  fail "Upload step failed (${#LOCATIONS[@]} / $N). Aborting."
  exit 1
fi

# ────────────────────────────────────────────────────────────────────────────
# 2) BIG PAYLOAD TEST (> limit)
... (45lignes restantes)