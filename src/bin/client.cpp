#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include "pack109.hpp"

using namespace std;

vector<unsigned char> encrypt(vector<unsigned char> msg){
    for(int i = 0; i < msg.size(); i++){
        msg[i] = msg[i] ^ 42;
    }

    return msg;
}

int main(int argc, char** argv){  //.client --hostname localhost:8001
    vector<unsigned char> buffer(50000);
    string hostname;

    //find hostname flag
    bool hflag = false;
    std::string s = argv[1];
    if(s == "--hostname"){
        hflag = true;
        hostname = argv[2];
    }
    else if(hflag == false){
        printf("Missing --hostname flag");
        exit(1);
    }

    //use hostname to connect to server
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent* server;
    
    int pos = hostname.find(":");
    portno = atoi(hostname.substr(pos+1, std::string::npos).c_str());
    
    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(hostname.substr(0, pos).c_str());
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    
    /* Now connect to the server */
    int n = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if (n < 0) {
        perror("ERROR connecting");
        exit(1);
    }
    
    bool connected = true;
    while(connected){
        string filename;
        string input;

        printf("\nEnter a message for the server (ex: --send files/document.txt): ");
        getline(cin, input);

        vector<string> words{};
        int i = 0;
        size_t pos = 0;
        input.append(" ");
        while ((pos = input.find(" ")) != string::npos) {
            words.push_back(input.substr(0, pos));
            input.erase(0, pos + 1);
        }

        bool sflag = false; bool rflag = false;

        if(words[0] == "--send"){
            sflag = true;
            filename = words[1];
        }
        else if(words[0] == "--request"){
            rflag = true;
            filename = words[1];
        }
        else{
            printf("Incorrect input");
            exit(1);
        }

        if(sflag == true && rflag == false){ //sending a file
            //reading file to memory
            streampos size;
            char* memblock;

            ifstream file (filename, ios::in|ios::binary|ios::ate);

            if (file.is_open()) {
                size = file.tellg();
                memblock = new char [size];
                file.seekg (0, ios::beg);
                file.read (memblock, size);
                file.close();
            }
            else{
                printf("Unable to open file\n");
            }

            //get file name from directory
            size_t pos = 0;
            std::string token = filename.append("/");
            while ((pos = filename.find("/")) != std::string::npos) {
                token = filename.substr(0, pos);
                filename.erase(0, pos + 1);
            }

            struct File send_file;
            send_file.name = token;
            for(int i = 0; i < strlen(memblock); i++){
                send_file.bytes.push_back(memblock[i]);
            }

            vector<unsigned char> serialized_send = pack109::serialize(send_file);

            vector<unsigned char> encrypted_send = encrypt(serialized_send);

            /* Send message to the server */
            printf("Sending file over socket...\n");
            n = send(sockfd, encrypted_send.data(), encrypted_send.size(), MSG_CONFIRM);
            if (n < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
            
            /* Now read server response */
            bzero(buffer.data(), buffer.size());
            n = read(sockfd, buffer.data(), buffer.size());
            if (n < 0) {
                perror("ERROR reading from socket");
                exit(1);
            }
            printf("%s\n", buffer.data());
        }
        else if(sflag == false && rflag == true){ //requesting a file
            //get file name from directory
            size_t pos = 0;
            std::string token = filename.append("/");
            while ((pos = filename.find("/")) != std::string::npos) {
                token = filename.substr(0, pos);
                filename.erase(0, pos + 1);
            }

            struct Request request_file;
            request_file.name = token;

            
            vector<unsigned char> serialized_request = pack109::serialize(request_file);

            vector<unsigned char> encrypted_request = encrypt(serialized_request);

            /* Send encrypted byte vector to the server */
            int n = send(sockfd, encrypted_request.data(), encrypted_request.size(), MSG_CONFIRM);
            if (n < 0) {
                perror("ERROR sending to socket");
                exit(1);
            }

            /* Now read server response */
            bzero(buffer.data(), buffer.size());
            n = recv(sockfd, buffer.data(), buffer.size(), MSG_CONFIRM);
            if (n < 0) {
                perror("ERROR receiving response from socket");
                exit(1);
            }

            vector<unsigned char> request_response;
            for(int i = 0; i < buffer.size(); i++){
                if(buffer[i] == 0x0) break;
                request_response.push_back(buffer[i]);
            }
            
            vector<unsigned char> decrypted_response = encrypt(request_response);

            struct File deserialized_file = pack109::deserialize_file(decrypted_response);

            //save file in folder called received
            string path = "received/" + deserialized_file.name;
            ofstream outfile(path);
            for(const auto &e: deserialized_file.bytes) outfile << e;
            outfile.close();
        
        }
        else if(words.size() > 2){
            // printf("Cannot --send and --request at the same time");
            exit(1);
        }

        string user;
        printf("\nWould you like to end now? (y/n): ");
        getline(cin, user);
        if(user == "y") {
            connected = false;
        }
    }
    return 0;
}