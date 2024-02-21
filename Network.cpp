#include <sstream>
#include "Network.h"
#include "fstream"
#include <algorithm>

std::vector<std::string> splitMessagesByLength(const std::string& message, int maxLength)
{
    std::vector<std::string> parcalar;

    int size = static_cast<int>(message.size());

    for (int i = 0; i < size; i += maxLength) {
        parcalar.push_back(message.substr(i, maxLength));
    }

    return parcalar;
}

std::vector<std::string> findMessage(const std::string& line, char delimiter) {
    std::istringstream iss(line);
    std::vector<std::string> sentence;
    std::string words;

    while (std::getline(iss, words, delimiter)) {
        if (!sentence.empty() && sentence.back().find('#') != std::string::npos) {
            sentence.back() += " " + words;
        } else {
            sentence.push_back(words);
        }
    }

    return sentence;
}

std::string takeCommand(const std::string& line)
{
    std::istringstream iss(line);
    std::vector<std::string> sentence;
    std::string words;
    while (std::getline(iss, words, ' ')) {
        sentence.push_back(words);
    }

    return sentence[0];

}
std::string takeClientId(const std::string& line)
{
    std::istringstream iss(line);
    std::vector<std::string> sentence;
    std::string words;
    while (std::getline(iss, words, ' ')) {
        sentence.push_back(words);
    }

    return sentence[1];
}

std::vector<std::string> readLinesFromFile(const std::string& fileName) {
    std::vector<std::string> lines;
    std::ifstream file(fileName);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file: " << fileName << std::endl;
        std::exit(-1);
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    file.close();
    return lines;
}





void printingFrame(std::vector<Client> &clients,int senderIndex,int frameIndex,string inOrOUT)
{
    queue<stack<Packet*>> queue;
    if(inOrOUT == "in")
    {
        queue = clients[senderIndex].incoming_queue;
        cout<<"Current Frame #"<<frameIndex<<" on the incoming queue of client "<< clients[senderIndex].client_id<<endl;
    }
    else
    {
        queue = clients[senderIndex].outgoing_queue;
        cout<<"Current Frame #"<<frameIndex<<" on the outgoing queue of client " << clients[senderIndex].client_id<<endl;
    }

    for(int i =1 ; i< queue.size(); i++) {
        queue.pop();
        if (i == frameIndex - 1) {
            stack<Packet *> stack = queue.front();

            Packet *packet = stack.top();
            auto *physical = dynamic_cast<PhysicalLayerPacket *> (packet);
            stack.pop();

            packet = stack.top();
            auto *network = dynamic_cast<NetworkLayerPacket *>(packet);
            stack.pop();


            packet = stack.top();
            auto *transport = dynamic_cast<TransportLayerPacket *>(packet);
            stack.pop();

            packet = stack.top();
            auto *app = dynamic_cast<ApplicationLayerPacket *>(packet);

            cout << "Carried Message: " << "\"" << app->message_data << "\"" << endl;

            cout << "Layer " << app->layer_ID << " info: Sender ID: " << app->sender_ID << ", Receiver ID: "
                 << app->receiver_ID << endl;

            cout << "Layer " << transport->layer_ID << " info: Sender port number: " << transport->sender_port_number <<
                 ", Receiver port number: " << transport->receiver_port_number << endl;

            cout << "Layer " << network->layer_ID << " info: Sender IP address: " << network->sender_IP_address <<
                 ", Receiver IP address: " << network->receiver_IP_address << endl;

            cout << "Layer " << physical->layer_ID << " info: Sender MAC address: " << physical->sender_MAC_address <<
                 ", Receiver MAC address: " << physical->receiver_MAC_address << endl;

            cout << "Number of hops so far: " << physical->numberOfHoops << endl;

        }
    }
}




void creatingFrame(std::vector<Client> &clients,std::vector<string> discreteMassage , const std::string& sender,
                   const std::string& reciver,int senderIndex, int reciverIndex, std::string sender_port,std::string receiver_port)
{
    for(int i =0 ; i < discreteMassage.size(); i++)
    {
        std::stack<Packet*> frame;
        auto it = clients[senderIndex].routing_table.find(reciver);
        string nextEleman = it->second;
        string macAdress;

        for(auto & client : clients)
        {
            if (client.client_id == nextEleman)
            {
                macAdress = client.client_mac;
            }
        }

        ApplicationLayerPacket* app = new ApplicationLayerPacket(0,sender,reciver,discreteMassage[i]);
        TransportLayerPacket* transport = new TransportLayerPacket(1,sender_port,receiver_port);
        NetworkLayerPacket* network = new NetworkLayerPacket(2,clients[senderIndex].client_ip,clients[reciverIndex].client_ip);
        PhysicalLayerPacket* physical = new PhysicalLayerPacket(3,clients[senderIndex].client_mac,macAdress);

        frame.push(app);
        frame.push(transport);
        frame.push(network);
        frame.push(physical);


        clients[senderIndex].outgoing_queue.push(frame);

        cout<<"Frame #"<<i+1<<endl;

        physical->print();

        network->print();

        transport->print();

        app->print();


        cout<<"Number of hops so far: "<<physical->numberOfHoops<<endl;
        cout<<"--------"<<endl;

    }
}


void messagesProces(std::string line, std::vector<Client> &clients, int message_limit,const string &sender_port,
                    const string &receiver_port)
{
    char delimeter = ' ';
    std::vector<std::string> input = findMessage(line,delimeter);

    std::string sender = input[1];
    std::string reciver = input[2];
    std::string str = input[3];
    int senderIndex = -1;
    int reciverIndex = -1;




    for(int i =0 ; i < clients.size(); i++)
    {
        if(sender == clients[i].client_id)
        {
            senderIndex = i;
        }
        else if(reciver == clients[i].client_id)
        {
            reciverIndex = i;
        }
    }


    size_t found = str.find_first_of("#");
    while (found != std::string::npos) {
        str.erase(found, 1);
        found = str.find_first_of("#");
    }


    cout<<"Message to be sent: "<<"\"" <<str<<"\""<<endl;
    cout<<endl;



    std::vector<std::string> discreteMassage = splitMessagesByLength(str,message_limit);
    //clients[senderIndex].frameCounterVector.push_back(discreteMassage.size());
    Log log("2023-11-22 20:30:03",str,discreteMassage.size(),0,clients[senderIndex].client_id,clients[reciverIndex].client_id,
            true,ActivityType::MESSAGE_SENT);
    clients[senderIndex].log_entries.push_back(log);
    creatingFrame(clients,discreteMassage,sender,reciver,senderIndex,reciverIndex,sender_port,receiver_port);

}

void showQInfo(string line, vector<Client> &clients)
{
    std::istringstream iss(line);
    std::vector<std::string> words;

    std::string word;
    while (std::getline(iss, word, ' ')) {
        words.push_back(word);
    }

    string clientId = words[1];
    string type = words[2];
    int clientIndex;
    for(int i = 0; i < clients.size(); i++)
    {
        if (clients[i].client_id == clientId)
        {
            clientIndex = i;
        }
    }
    if(type == "out")
    {
        cout<<"Client "<<clientId<< " Outgoing Queue Status"<<endl;
        cout<<"Current total number of frames: "<<clients[clientIndex].outgoing_queue.size()<<endl;
    }
    else if (type == "in")
    {
        cout<<"Client "<<clientId<< " Incoming Queue Status"<<endl;
        cout<<"Current total number of frames: "<<clients[clientIndex].incoming_queue.size()<<endl;
    }
}

void showFInfo(string line, vector<Client> &clients)
{
    std::istringstream iss(line);
    std::vector<std::string> words;

    std::string word;
    while (std::getline(iss, word, ' ')) {
        words.push_back(word);
    }

    string clientId = words[1];
    string type = words[2];
    int frameNumber = stoi(words[3]);
    int clientIndex;
    for(int i = 0; i < clients.size(); i++)
    {
        if (clients[i].client_id == clientId)
        {
            clientIndex = i;
        }
    }

    if(type == "in")
    {
        if(frameNumber < clients[clientIndex].incoming_queue.size())
        {
            //printingFrame(clients, clientIndex,frameNumber,"in");
        }
        else
        {
            cout<<"No such frame."<<endl;
        }
    }
    else
    {
        if(frameNumber < clients[clientIndex].outgoing_queue.size())
        {
            //printingFrame(clients, clientIndex,frameNumber,"out");
        }
        else
        {
            cout<<"No such frame."<<endl;
        }
    }

}

void sender(vector<Client> &clients)
{
    for(auto &client: clients)
    {
        int counter = 1;
        while(!client.outgoing_queue.empty())
        {
            stack<Packet*> frame = client.outgoing_queue.front();

            Packet* packet = frame.top();
            PhysicalLayerPacket* physical = dynamic_cast<PhysicalLayerPacket*>(packet);
            string incomingQowner = "";
            incomingQowner += physical->receiver_MAC_address[0];
            int incomingIndex = 0;
            for(int i = 0; i< clients.size();i++)
            {
                if(clients[i].client_id == incomingQowner)
                {
                    incomingIndex = i;
                }
            }

            frame.pop();
            packet = frame.top();
            NetworkLayerPacket* network = dynamic_cast<NetworkLayerPacket*>(packet);

            frame.pop();
            packet = frame.top();
            TransportLayerPacket* transport = dynamic_cast<TransportLayerPacket*>(packet);

            frame.pop();
            packet = frame.top();
            ApplicationLayerPacket* app = dynamic_cast<ApplicationLayerPacket*>(packet);

            frame.push(transport);
            frame.push(network);
            physical->numberOfHoops++;
            frame.push(physical);

            clients[incomingIndex].incoming_queue.push(frame);
            client.outgoing_queue.pop();

            char charsToFind[] = {'?', '!','.'};

            int dotCounter = 0;
            for (char charToFind : charsToFind) {
                size_t found = app->message_data.find(charToFind);
                if (found != std::string::npos) {
                    dotCounter++;
                }
            }
            cout<<"Client "<<client.client_id <<" sending frame #"<<counter<<" to client "<<incomingQowner<<endl;
            if(dotCounter == 1)
            {
                counter = 0;
            }


            int senderIndex = 0;

            for(int i = 0;i < clients.size(); i++)
            {
                if(clients[i].client_id == app->sender_ID)
                {
                    senderIndex = i;
                }
            }



            cout<<"Sender MAC address: "<<client.client_mac<<", Receiver MAC address: "<<physical->receiver_MAC_address<<endl;
            cout<<"Sender IP address: "<<network->sender_IP_address<<", Receiver IP address: "<<network->receiver_IP_address<<endl;
            cout<<"Sender port number: "<<transport->sender_port_number<<", Receiver port number: "<<transport->receiver_port_number <<endl;
            cout<<"Sender ID: "<<app->sender_ID <<", Receiver ID: "<<app->receiver_ID<<endl;
            cout<<"Message chunk carried: "<<"\""<<app->message_data <<"\""<<endl;
            cout<<"Number of hops so far: "<< physical->numberOfHoops <<endl;
            cout<<"--------"<<endl;

            counter++;
        }
    }
}

void receive(vector<Client> &clients)
{
    for (auto &client : clients)
    {

        int framesCounter = 0;
        string str = "";
        string messageData = "";
        int normalFrameCounter = 0;
        int type = 1;
        int dropFrameCounter = 0;
        while(!client.incoming_queue.empty())
        {
            stack<Packet*> frame = client.incoming_queue.front();

            Packet* packet = frame.top();
            PhysicalLayerPacket* physical = dynamic_cast<PhysicalLayerPacket*>(packet);
            frame.pop();

            packet = frame.top();
            NetworkLayerPacket* network = dynamic_cast<NetworkLayerPacket*>(packet);
            frame.pop();

            packet = frame.top();
            TransportLayerPacket* transport = dynamic_cast<TransportLayerPacket*>(packet);
            frame.pop();

            packet = frame.top();
            ApplicationLayerPacket* app = dynamic_cast<ApplicationLayerPacket*>(packet);

            frame.push(transport);
            frame.push(network);
            frame.push(physical);

            string reciver = app->receiver_ID;
            int dotCounter = 0;
            if(client.client_id == reciver)
            {
                char charsToFind[] = {'?', '!','.'};

                for (char charToFind : charsToFind) {
                    size_t found = app->message_data.find(charToFind);
                    if (found != std::string::npos) {
                        dotCounter++;
                    }
                }
                string lastMassage ;
                if(dotCounter == 1)
                {
                    str+= app->message_data;
                    lastMassage = str;
                    framesCounter++;
                    Log log("2023-11-22 20:30:03",str,framesCounter,physical->numberOfHoops,app->sender_ID,client.client_id, true,
                            ActivityType::MESSAGE_RECEIVED);
                    client.log_entries.push_back(log);
                    str = "";
                    cout<<"Client E receiving frame #"<<framesCounter <<" from client "<< physical->sender_MAC_address[0]<<
                        ", originating from client "<< app->sender_ID<<endl;
                    framesCounter = 0;
                }
                else
                {
                    framesCounter++;
                    str+= app->message_data;
                    cout<<"Client E receiving frame #"<<framesCounter <<" from client "<< physical->sender_MAC_address[0]<<
                        ", originating from client "<< app->sender_ID<<endl;
                }




                cout<<"Sender MAC address: "<<physical->sender_MAC_address<<", Receiver MAC address: "<<physical->receiver_MAC_address<<endl;
                cout<<"Sender IP address: "<<network->sender_IP_address<<", Receiver IP address: "<<network->receiver_IP_address<<endl;
                cout<<"Sender port number: "<<transport->sender_port_number<<", Receiver port number: "<<transport->receiver_port_number<<endl;
                cout<<"Sender ID: "<<app->sender_ID<<", Receiver ID: "<<app->receiver_ID<<endl;
                cout<<"Message chunk carried: \""<<app->message_data<<"\""<<endl;
                cout<<"Number of hops so far: "<<physical->numberOfHoops<<endl;
                cout<<"--------"<<endl;

                if(dotCounter == 1)
                {
                    cout<<"Client "<<app->receiver_ID<<" received the message \""<< lastMassage<<"\""<<" from client "<<
                        app->sender_ID<<"."<<endl;
                    cout<<"--------"<<endl;
                }


                while (!frame.empty()) {
                    Packet* packet = frame.top();
                    frame.pop();
                    delete packet;
                }
            }
            else
            {
                bool a = false;
                string x = client.routing_table.find(app->receiver_ID)->second;
                for(int i = 0;i < clients.size();i++)
                {
                    if(x == clients[i].client_id)
                    {
                        a = true;
                    }
                }
                if(!a)
                {
                    int senderIndex = 0;
                    for(int i = 0; i < clients.size();i++)
                    {
                        if(clients[i].client_id == app->sender_ID)
                        {
                            senderIndex = i;
                        }
                    }

                    int numberOfFrames = 0;
                    for(auto log : clients[senderIndex].log_entries)
                    {
                        size_t found = log.message_content.find(app->message_data);

                        if (found != std::string::npos && app->receiver_ID == log.receiver_id
                            && log.sender_id == app->sender_ID) {
                            numberOfFrames = log.number_of_frames;
                        }
                    }

                    char charsToFind[] = {'?', '!','.'};

                    for (char charToFind : charsToFind) {
                        size_t found = app->message_data.find(charToFind);
                        if (found != std::string::npos) {
                            dotCounter++;
                        }
                    }
                    if(dotCounter == 1)
                    {
                        dropFrameCounter++;
                        cout<<"Client "<<client.client_id<<" receiving frame #"<< dropFrameCounter<<" from client "<<app->sender_ID<<", but intended "<<
                            "for client "<<app->receiver_ID<<". Forwarding... "<<endl;
                        cout<<"Error: Unreachable destination. Packets are dropped after "<<physical->numberOfHoops <<" hops!"<<endl;
                        dotCounter = 0;
                        cout<<"--------"<<endl;
                        Log log("2023-11-22 20:30:03","",dropFrameCounter,physical->numberOfHoops,app->sender_ID,app->receiver_ID
                                ,false,ActivityType::MESSAGE_DROPPED);
                        dropFrameCounter = 0;
                        client.log_entries.push_back(log);
                    }
                    else
                    {
                        dropFrameCounter++;
                        cout<<"Client "<<client.client_id<<" receiving frame #"<<dropFrameCounter<<" from client "<<app->sender_ID<<", but intended "<<
                            "for client "<<app->receiver_ID<<". Forwarding... "<<endl;
                        cout<<"Error: Unreachable destination. Packets are dropped after "<<physical->numberOfHoops <<" hops!"<<endl;
                    }


                    while (!frame.empty()) {
                        Packet* packet = frame.top();
                        frame.pop();
                        delete packet;
                    }
                }
                else
                {
                    int newMacIndex = 0;
                    for(int i = 0 ;i < clients.size();i++)
                    {
                        if(clients[i].client_id == x)
                        {
                            newMacIndex = i;
                        }
                    }


                    int senderIndex = 0;
                    for(int i = 0; i < clients.size();i++)
                    {
                        if(clients[i].client_id == app->sender_ID)
                        {
                            senderIndex = i;
                        }
                    }

                    int numberOfFrames = 0;
                    for(auto log : clients[senderIndex].log_entries)
                    {
                        size_t found = log.message_content.find(app->message_data);

                        if (found != std::string::npos && app->receiver_ID == log.receiver_id
                            && log.sender_id == app->sender_ID) {
                            numberOfFrames = log.number_of_frames;
                        }
                    }
                    if(type == 1)
                    {
                        cout<<"Client "<<physical->receiver_MAC_address[0]<<" receiving a message from client "<<
                            physical->sender_MAC_address[0]<<", but intended for client "<<app->receiver_ID<<"."<<" Forwarding... "
                            <<endl;
                        type = 0;
                    }

                    physical->sender_MAC_address = physical->receiver_MAC_address;
                    physical->receiver_MAC_address = clients[newMacIndex].client_mac;

                    char charsToFind[] = {'?', '!','.'};

                    for (char charToFind : charsToFind) {
                        size_t found = app->message_data.find(charToFind);
                        if (found != std::string::npos) {
                            dotCounter++;
                        }
                    }
                    string lastMassage ;
                    if(dotCounter == 1)
                    {
                        cout<<"Frame #"<<++normalFrameCounter<<" MAC address change: New sender MAC "<<physical->sender_MAC_address
                            <<", new receiver MAC "<<physical->receiver_MAC_address<<endl;
                        cout<<"--------"<<endl;
                        Log log("2023-11-22 20:30:03","",normalFrameCounter,physical->numberOfHoops,app->sender_ID,
                                app->receiver_ID, true,ActivityType::MESSAGE_FORWARDED);
                        client.log_entries.push_back(log);
                        normalFrameCounter = 0;
                        dotCounter = 0;
                        type = 1;

                    }
                    else
                    {
                        cout<<"Frame #"<<++normalFrameCounter<<" MAC address change: New sender MAC "<<physical->sender_MAC_address
                            <<", new receiver MAC "<<physical->receiver_MAC_address<<endl;
                    }
                    client.outgoing_queue.push(frame);
                }
            }
            client.incoming_queue.pop();
        }





    }
}





void printLog(vector<Client> &clients,string clientId)
{
    for(auto &client: clients)
    {
        if(client.client_id == clientId && !client.log_entries.empty())
        {
            cout<<"Client "<<client.client_id<<" Logs:"<<endl;
            cout<<"--------------"<<endl;
            for(int i = 0;i < client.log_entries.size(); i++)
            {
                cout<<"Log Entry #"<< i+1<<":"<<endl;
                if(client.log_entries[i].activity_type == ActivityType::MESSAGE_SENT)
                {
                    cout<<"Activity: " <<"Message Sent" <<endl;
                }
                else if (client.log_entries[i].activity_type == ActivityType::MESSAGE_DROPPED)
                {
                    cout<<"Activity: "<<"Message Dropped"<<endl;
                }
                else if (client.log_entries[i].activity_type == ActivityType::MESSAGE_FORWARDED)
                {
                    cout<<"Activity: "<<"Message Forwarded"<<endl;
                }
                else if (client.log_entries[i].activity_type == ActivityType::MESSAGE_RECEIVED)
                {
                    cout<<"Activity: "<<"Message Received"<<endl;
                }
                cout<<"Timestamp: "<<client.log_entries[i].timestamp<<endl;
                cout<<"Number of frames: "<<client.log_entries[i].number_of_frames<<endl;
                cout<<"Number of hops: "<<client.log_entries[i].number_of_hops<<endl;
                cout<<"Sender ID: "<<client.log_entries[i].sender_id<<endl;
                cout<<"Receiver ID: "<<client.log_entries[i].receiver_id<<endl;
                if(client.log_entries[i].success_status)
                {
                    cout<<"Success: Yes"<<endl;
                }
                else{
                    cout<<"Success: No"<<endl;
                }
                if(client.log_entries[i].activity_type != ActivityType::MESSAGE_FORWARDED
                   && client.log_entries[i].activity_type != ActivityType::MESSAGE_DROPPED)
                {
                    cout<<"Message: \""<<client.log_entries[i].message_content<<"\""<<endl;
                }
                if(i != client.log_entries.size() -1 )
                {
                    cout<<"--------------"<<endl;

                }
            }
        }
    }
}








Network::Network() = default;

void Network::process_commands(vector<Client> &clients, vector<string> &commands, int message_limit,
                               const string &sender_port, const string &receiver_port) {
    // TODO: Execute the commands given as a vector of strings while utilizing the remaining arguments.
    /* Don't use any static variables, assume this method will be called over and over during testing.
     Don't forget to update the necessary member variables after processing each command. For example,
     after the MESSAGE command, the outgoing queue of the sender must have the expected frames ready to send. */


    for(int i =0 ;i < commands.size(); i++)
    {
        string line = commands[i];
        string mySentence = "Command: " + line;
        string shortLines(mySentence.length(), '-');
        cout<<shortLines<<endl;
        cout<<mySentence<<endl;
        cout<<shortLines<<endl;


        std::string command = takeCommand(line);


        if (command == "MESSAGE") {
            messagesProces(line, clients, message_limit, sender_port, receiver_port);
        } else if (command == "SHOW_FRAME_INFO")
        {
            showFInfo(line,clients);

        } else if (command == "SHOW_Q_INFO")
        {
            showQInfo(line,clients);
        }
        else if (command == "SEND")
        {
            sender(clients);

        } else if (command == "RECEIVE") {
            receive(clients);
        } else if (command == "PRINT_LOG") {
            string clientId = takeClientId(line);
            printLog(clients,clientId);
        } else {
            if(i == commands.size() -1)
            {
                std::cout<<"Invalid command.";
            }
            else
            {
                std::cout<<"Invalid command."<<endl;
            }

        }

    }

    /*for (auto &line: commands) {
        string mySentence = "Command: " + line;
        string shortLines(mySentence.length(), '-');
        cout<<shortLines<<endl;
        cout<<mySentence<<endl;
        cout<<shortLines<<endl;


        std::string command = takeCommand(line);


        if (command == "MESSAGE") {
            messagesProces(line, clients, message_limit, sender_port, receiver_port);
        } else if (command == "SHOW_FRAME_INFO")
        {
            showFInfo(line,clients);

        } else if (command == "SHOW_Q_INFO")
        {
            showQInfo(line,clients);
        }
        else if (command == "SEND")
        {
            sender(clients);

        } else if (command == "RECEIVE") {
            receive(clients);
        } else if (command == "PRINT_LOG") {
            string clientId = takeClientId(line);
            printLog(clients,clientId);
        } else {
            std::cout<<"Invalid command.";
        }

    }*/
}

vector<Client> Network::read_clients(const string &filename) {
    vector<Client> clients;

    std::vector<string> input = readLinesFromFile(filename);
    int clientCount = std::stoi(input[0]);

    for(int i = 0; i < clientCount ; i++)
    {
        std::istringstream iss(input[i+1]);
        std::vector<std::string> tokens;
        std::string token;

        while (std::getline(iss, token, ' ')) {
            tokens.push_back(token);
        }

        Client newClient(tokens[0],tokens[1],tokens[2]);
        clients.push_back(newClient);
    }

    return clients;
}

void Network::read_routing_tables(vector<Client> &clients, const string &filename) {
    // TODO: Read the routing tables from the given input file and populate the clients' routing_table member variable.
    std::vector<string> input = readLinesFromFile(filename);

    int clientCounter = 0;

    for(int i = 0; i < input.size(); i++)
    {
        if(input[i] == "-")
        {
            clientCounter++;
        }
        else
        {
            char keyChar = input[i][0];
            std::string key;
            key += keyChar;
            char valueChar = input[i][2];
            std::string val;
            val += valueChar;
            clients[clientCounter].routing_table.emplace(key,val);
        }
    }


}

// Returns a list of token lists for each command
vector<string> Network::read_commands(const string &filename) {
    vector<string> commands;
    // TODO: Read commands from the given input file and return them as a vector of strings.
    commands = readLinesFromFile(filename);
    commands.erase(commands.begin());


    return commands;
}

Network::~Network() {
    // TODO: Free any dynamically allocated memory if necessary.

}
