#!/bin/bash
if [[ -e test.db ]]; then
  rm test.db;
fi
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
sqlite3 test.db < "${SCRIPT_DIR}"/test_db.sql