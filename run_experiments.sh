#!/bin/bash

TRACES=(
    "meta_reag.oracleGeneral.zst"
    "meta_rnha.oracleGeneral.zst"
    "meta_rprn.oracleGeneral.zst"
)

PERCENTAGES=(0.0005 0.005 0.01 0.05 0.1)
PERCENTAGE_LABELS=("0.05%" "0.5%" "1%" "5%" "10%")

#Check for executables
if [ ! -f "./cache_simulator.exe" ] || [ ! -f "./get_wss.exe" ]; then
    echo "Error:cache_simulator.exe or get_wss.exe not found."
    exit 1
fi

echo "************************************************************************"
echo " DISTINCT ENTRY COUNT FROM INPUT STREAM"
echo "************************************************************************"

for trace in "${TRACES[@]}"; do
    
    if [ ! -f "$trace" ]; then
        echo "Warning: File '$trace' not  found! Check for next..."
        continue
    fi
    
    echo "------------------------------------------------------------------------"
    echo " Analysis: $trace"
    echo " Distinct Entry Calculation... "
    
    # Run helper program and store count in WSS
    WSS=$(./get_wss.exe "$trace")
    echo " $WSS Distinct Entries Found!"
    echo "------------------------------------------------------------------------"
    # Run Simulator for every case
    for i in "${!PERCENTAGES[@]}"; do
        p="${PERCENTAGES[$i]}"
        label="${PERCENTAGE_LABELS[$i]}"
        
        # Calculate capacity with awk (integer rounding)
        CAPACITY=$(awk "BEGIN {print int($WSS * $p)}")
        
        # In case of zero capacity turn it to zero
        if [ "$CAPACITY" -eq 0 ]; then
            CAPACITY=1
        fi
        echo "************************************************************************"
        echo ">>> RUN FOR CACHE CAPACITY: $CAPACITY (PERCENTAGE: $label) <<<"
        
        ./cache_simulator.exe "$trace" "$CAPACITY"
    done
done

echo "************************************************************************"
echo "Auto Grading completed succesfully!"
echo "************************************************************************"
