//
// Created by alperen on 27.09.2023.
//

#include "Client.h"

Client::Client(string const& _id, string const& _ip, string const& _mac) {
    client_id = _id;
    client_ip = _ip;
    client_mac = _mac;
}

ostream &operator<<(ostream &os, const Client &client) {
    os << "client_id: " << client.client_id << " client_ip: " << client.client_ip << " client_mac: "
       << client.client_mac << endl;
    return os;
}

Client::~Client() {
    // TODO: Free any dynamically allocated memory if necessary.
    while(!incoming_queue.empty())
    {
        stack<Packet*> frame = incoming_queue.front();
        incoming_queue.pop();

        while(!frame.empty())
        {
            Packet* packet = frame.top();
            frame.pop();
            delete packet;
        }
    }
    while (!outgoing_queue.empty())
    {
        std::stack<Packet*> frame = outgoing_queue.front();
        outgoing_queue.pop();

        // Stack'teki belleği temizle
        while (!frame.empty()) {
            Packet* packet = frame.top();
            frame.pop();
            delete packet;
        }
    }

}