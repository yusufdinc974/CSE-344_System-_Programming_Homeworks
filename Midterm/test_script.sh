#!/bin/bash
set -e

# Server FIFO name
SERVER_FIFO="server_fifo"

# Timeout function
timeout_handler() {
    echo "Operation timed out!"
    cleanup
    exit 1
}

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    # Kill any running server processes
    pkill -f "BankServer" 2>/dev/null || true
    
    # Kill any running client processes
    pkill -f "BankClient" 2>/dev/null || true
    
    # Wait a bit to make sure processes exit
    sleep 1
    
    # Remove FIFOs
    rm -f client_*_fifo
    rm -f $SERVER_FIFO
}

# Set up traps
trap 'timeout_handler' ALRM
trap 'cleanup; exit' INT TERM EXIT

# Make sure previous instances are cleaned up
cleanup

# Create a fresh log file
rm -f AdaBank.bankLog

echo "===== Testing Basic Server ====="

# Start the bank server in the background
./BankServer AdaBank $SERVER_FIFO &
SERVER_PID=$!

# Wait for server to initialize
sleep 2

# Run the first client file with a timeout
echo "Running Client01.file..."
# Set a 30 second alarm
(sleep 30; kill -ALRM $) &
ALARM_PID=$!
./BankClient Client01.file $SERVER_FIFO
# Cancel the alarm
kill $ALARM_PID 2>/dev/null || true
echo "Client01 completed."

# Give some time for server to process
sleep 2

# Run the second client file with a timeout
echo "Running Client02.file..."
# Set a 30 second alarm
(sleep 30; kill -ALRM $) &
ALARM_PID=$!
./BankClient Client02.file $SERVER_FIFO
# Cancel the alarm
kill $ALARM_PID 2>/dev/null || true
echo "Client02 completed."

# Display log file
echo "Bank log after basic server test:"
cat AdaBank.bankLog
echo

# Stop the server
echo "Stopping the basic server..."
kill -TERM $SERVER_PID
wait $SERVER_PID 2>/dev/null || true
sleep 2
rm -f $SERVER_FIFO
rm -f client_*_fifo

echo "===== Testing Enhanced Server ====="

# Start the enhanced server with a clear log file
rm -f AdaBank.bankLog
./BankServer_Enhanced AdaBank $SERVER_FIFO &
SERVER_PID=$!

# Wait for server to initialize
sleep 2

# Run the third client file with a timeout
echo "Running Client03.file..."
# Set a 30 second alarm
(sleep 30; kill -ALRM $) &
ALARM_PID=$!
./BankClient Client03.file $SERVER_FIFO
# Cancel the alarm
kill $ALARM_PID 2>/dev/null || true
echo "Client03 completed."

# Display final log file
echo "Bank log after enhanced server test:"
cat AdaBank.bankLog
echo

echo "All tests completed successfully!"
exit 0