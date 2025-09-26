#include "hello_server.h"
#include <iostream>
#include <signal.h>

using namespace std;
using namespace Pistache;

HelloServer* server_ptr = nullptr;

void signal_handler(int signal) {
    if (server_ptr) {
        cout << "\nShutting down server gracefully..." << endl;
        exit(0);
    }
}

int main() {
    // Set up signal handler for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    Port port(9080);
    int thr = 2;

    Address addr(Ipv4::any(), port);

    cout << "C++ REST Server starting..." << endl;
    cout << "Listening on port 9080 (accepting connections from any IP)" << endl;
    cout << "Available endpoints:" << endl;
    cout << "  GET /hello - Returns Hello World message" << endl;
    cout << "  GET /      - Returns Hello World message" << endl;
    cout << "Local access:  http://localhost:9080" << endl;
    cout << "Remote access: http://10.0.0.32:9080 (or your-server-ip:9080)" << endl;
    cout << "Press Ctrl+C to stop the server" << endl;

    HelloServer server(addr);
    server_ptr = &server;

    server.init(thr);
    server.start();

    return 0;
}