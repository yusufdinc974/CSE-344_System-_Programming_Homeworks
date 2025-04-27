# test_script.sh
#!/bin/bash
set -e

# Create a server FIFO
SERVER_FIFO="server_fifo"

# Clean up function
cleanup() {
    echo "Cleaning up..."
    if [ ! -z "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null || true
        wait $SERVER_PID 2>/dev/null || true
    fi
    rm -f client_*_fifo
    rm -f $SERVER_FIFO
    rm -f AdaBank.bankLog
    exit 0
}

# Set up trap
trap cleanup SIGINT SIGTERM EXIT

# Make the scripts executable
chmod +x BankServer BankClient BankServer_Enhanced

echo "======== Testing Basic Server ========"
# Start the server in the background
./BankServer AdaBank $SERVER_FIFO &
SERVER_PID=$!

# Wait for server to initialize
sleep 1

# Run the first client
echo "Running Client01..."
./BankClient Client01.file $SERVER_FIFO
echo

# Run the second client
echo "Running Client02..."
./BankClient Client02.file $SERVER_FIFO
echo

# Display the log file
echo "Bank Log after basic server test:"
cat AdaBank.bankLog
echo

# Stop the basic server
echo "Stopping basic server..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null || true
rm -f $SERVER_FIFO
sleep 1

echo "======== Testing Enhanced Server ========"
# Start the enhanced server
./BankServer_Enhanced AdaBank $SERVER_FIFO &
SERVER_PID=$!

# Wait for server to initialize
sleep 1

# Run clients
echo "Running Client03 with enhanced server..."
./BankClient Client03.file $SERVER_FIFO
echo

# Display the log file
echo "Bank Log after enhanced server test:"
cat AdaBank.bankLog
echo

# Create a high-load test with 20 clients
echo "Creating stress test with 20 clients..."
echo "N deposit 100" > stress_test.file
for i in {1..19}; do
    echo "BankID_1 deposit 10" >> stress_test.file
done

# Run stress test
echo "Running stress test with 20 clients..."
./BankClient stress_test.file $SERVER_FIFO
echo

# Display the final log file
echo "Final Bank Log:"
cat AdaBank.bankLog
echo

echo "All tests completed successfully!"