#!/bin/bash
# Runs the unit tests and etatest for every mode (eta and eta' parents, every parent forced and one per event),
# after build.sh. Prints one line per run and exits non-zero if any run fails.
# Usage: run_checks.sh [nEvents=500]   (a few minutes in total with the default)
cd "$(dirname "$0")/build" 2>/dev/null || { echo "build/ not found: run build.sh first"; exit 1; }
n=${1:-500}
[ -x etatest ] || { echo "etatest not found: run build.sh first"; exit 1; }
# Pythia finds its data files through PYTHIA8DATA (set by LCG views); otherwise take them from pythia8-config
if [ -z "$PYTHIA8DATA" ] && command -v pythia8-config >/dev/null; then
  export PYTHIA8DATA=$(pythia8-config --xmldoc)
fi
status=0
if [ -x etadecayer_tests ]; then
  if ./etadecayer_tests >unit_tests.log 2>&1; then
    echo "unit tests: $(grep -E 'All tests passed' unit_tests.log)"
  else
    echo "unit tests: FAILED, see unit_tests.log"
    status=1
  fi
else
  echo "unit tests: not built (Catch2 v3 not found by build.sh)"
fi
for run in "2mu" "2egamma" "2mugamma" "2mu2e" "4e" "4mu" "2e2pi" "2mu2pi" "2muA" "2piA" \
           "2mu:331" "2egamma:331" "2mugamma:331" "2mu2e:331" "4e:331" "4mu:331" "2e2pi:331" "2mu2pi:331" \
           "2muA:331" "2piA:331" "eta2pi" "eta2pi0"; do
  for one in 0 1; do
    out=$(./etatest "$run" "$n" 1 $one 2>&1)
    rc=$?
    line=$(echo "$out" | grep -E '^mode=')
    if [ $rc -eq 0 ]; then
      echo "PASS $line"
    else
      echo "FAIL ($run onePerEvent=$one, exit $rc) ${line:-$(echo "$out" | tail -3)}"
      status=1
    fi
  done
done
[ $status -eq 0 ] && echo "all checks passed" || echo "some checks FAILED"
exit $status
