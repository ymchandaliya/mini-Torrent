#include<bits/stdc++.h>
#include <netinet/in.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include<thread>
#include<pthread.h>
#include <arpa/inet.h>
#include<fcntl.h>
#include<dirent.h>
#include<sys/stat.h>
#include <openssl/sha.h>

#define PORT 8989
#define TrackerPORT 6000
using namespace std;

string currentUser = "";
pthread_t serverThreads[1000];
unordered_map<string, pair<string, string>> fileToFilePaths;//groupid uname fileName: <bitmap,filepath>
vector<thread> uThreads;// upload  Threads
vector<thread> dThreads;//  download Threads
vector<thread> cdThreads;//  download Threads


string convertToString(char* a)
{
    string s = a;
    return s;
}

vector<string> splitString(string s)
{
    vector<string> tempString;
    int n = s.size();
    string temp;
    for (int i = 0; i < n; i++)
    {
        if (i == n - 1 && s[i] != ' ')
        {
            temp += s[i];
            tempString.push_back(temp);
        }
        else if (s[i] == ' ')
        {
            tempString.push_back(temp);
            temp = "";
        }
        else
        {
            temp += s[i];
        }
    }

    return tempString;
}


void sendFile(vector<string> cmd, int new_socket){
        int fd;
        char hello[524288] = {0};
        string fPath = fileToFilePaths[cmd[1] + " " + currentUser + " " + cmd[2]].second;
        //cout<<fPath<<endl;
        long long fSize=0;
        struct stat st; 
        if (stat(fPath.c_str(), &st) == 0)
            fSize = st.st_size;
        //cout<<fSize<<endl;
        string bitstring = fileToFilePaths[cmd[1] + " " + currentUser + " " + cmd[2]].first;
        bitstring = bitstring + " " + to_string(fSize);
        //cout<<bitstring<<endl;
        if((fd = open(fPath.c_str(), O_RDONLY)) < 0){
            cout<<"Error in opening file\n";                
        }
        //cout<<cmd[1]<<endl;
        //cout<<2<<endl;
        char fileBuf[524288] = {0};
        //cout<<1<<endl;
        //cout<<fd<<endl;
        // ssize_t x = read(fd, fileBuf, 524288);
        // cout<<x<<endl;
        bzero((char*)&hello, sizeof(hello));
        strcpy(hello, bitstring.c_str());
        send(new_socket, hello, sizeof(hello), 0);
        close(new_socket);
        // while(1){
        //     int x = read(fd, fileBuf, 524288);
        //     cout<<"x is: "<<x<<endl;
        //     if(x<=0){
        //         cout<<"File complete\n";
        //         break;
        //     }
        //     write(new_socket, fileBuf, x);
        // }

}

void receiveChunk(string operation, vector<string> command, vector<string> peerads, int df, int i){
        int sock1, clientPeer_fd;
        struct sockaddr_in peer_addr;
        char sendMsg[524288] = { 0 };
        char recFileBuf[524288] = {0};
        char sendBuf[524288] = {0};
        if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
        }
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(stoi(peerads[1]));
        // cout<<peerAddr[2]<<endl;
        // Convert IPv4 and IPv6 addresses from text to binary
        // form
        if (inet_pton(AF_INET, peerads[0].c_str(), &peer_addr.sin_addr)
            <= 0) {
            printf(
                "\nInvalid address/ Address not supported \n");
            // return -1;
        }

        if ((clientPeer_fd
            = connect(sock1, (struct sockaddr*)&peer_addr,
                    sizeof(peer_addr)))
            < 0) {
            printf("\nConnection Failed \n");
            // return -1;
        }
        cout<<"Connection established with peer\n";
        operation = "giveChunk";
        operation = operation + " " + command[1] + " " + command[2] + " " + to_string(i);
        strcpy(sendBuf, operation.c_str());
        send(sock1, sendBuf, strlen(sendBuf), 0);
        int x,k=0;
        long long bwrite = 0;
        char chunkBuf[524288 + 1] = {0};
        while((x = read(sock1, recFileBuf, sizeof(recFileBuf)))>0){
            // cout<<2<<" "; 
            // if(i==0){
            //     cout<<recFileBuf;
            bwrite += x;
            // cout<<x<<" ";
            for(int j=0;j<x;j++){
                chunkBuf[k++] = recFileBuf[j];
            }
            bzero((char*)&recFileBuf, sizeof(&recFileBuf));

        }
        pwrite(df, &chunkBuf, bwrite, i*524288);
        bzero((char*)&chunkBuf, sizeof(&chunkBuf));
        bwrite = 0;
        close(clientPeer_fd);

}

void receiveFile(string operation, vector<string> command, vector<string> peerAddr, int trackerSock){
        int peerSock = 0, sock1, clientPeer_fd;
        struct sockaddr_in peer_addr;
        char sendMsg[524288] = { 0 };
        char recvMsg[524288] = { 0 };
        long long fileSize = stoll(peerAddr[1]);
        int noOfChunks;
        unordered_map<int, set<string>> chunkSeaders;//chunkNo:seaders
        //ip port:bitmap
        unordered_map<string, string> chunkMaps;//ip port:bitmap
        
        int peers = peerAddr.size();
        for(int i=3;i<peers;i+=2){

            if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                printf("\n Socket creation error \n");
            }
            peer_addr.sin_family = AF_INET;
            peer_addr.sin_port = htons(stoi(peerAddr[i+1]));
            // cout<<peerAddr[2]<<endl;
            // Convert IPv4 and IPv6 addresses from text to binary
            // form
            if (inet_pton(AF_INET, peerAddr[i].c_str(), &peer_addr.sin_addr)
                <= 0) {
                printf(
                    "\nInvalid address/ Address not supported \n");
                // return -1;
            }

            if ((clientPeer_fd
                = connect(sock1, (struct sockaddr*)&peer_addr,
                        sizeof(peer_addr)))
                < 0) {
                printf("\nConnection Failed \n");
                // return -1;
            }
            cout<<"Connection established with peer\n";
            strcpy(sendMsg, operation.c_str());
            send(sock1, sendMsg, strlen(sendMsg), 0);
            
            int valread = read(sock1, recvMsg, sizeof(recvMsg));
            try
            {
                if(valread>0){
                    cout<<recvMsg<<endl;
                    string bitMap = convertToString(recvMsg);
                    vector<string> bitSize = splitString(bitMap);// bitMap and file size
                    bitMap = bitSize[0];
                    noOfChunks = bitMap.size();
                    //fileSize = stoll(bitSize[1]);
                    //cout<<fileSize<<endl<<bitMap<<endl<<noOfChunks<<endl;
                    chunkMaps[peerAddr[i] + " " + peerAddr[i+1]] = bitMap;
                    //close(clientPeer_fd);
                }
                /* code */
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }

        }

        for(auto it:chunkMaps){
            string key = it.first;
            string bMap = it.second;
            for(int i=0;i<noOfChunks;i++){
                if(bMap[i] == '1'){
                    chunkSeaders[i].insert(key);
                }
            }    
        }

       char recFileBuf[524288] = {0};
        string filePath = command[3] + command[2];

        FILE *file = fopen((filePath).c_str(), "w");
        ftruncate(fileno(file), fileSize);
        fclose(file);

        int receivedChunks = 0;
        int df = open(filePath.c_str(), O_WRONLY , 0644);
        if(df<0){
            cout<<"Error\n";
        }
        //cout<<df;
        cout<<"File created\n";

        for(int i=0;i<noOfChunks;i++){
            auto it = chunkSeaders[i].begin();
            string ipPort = *it;
            vector<string> peerads = splitString(ipPort);
            //cdThreads.push_back(thread(receiveChunk, operation, command, peerads, df, i));

            char sendBuf[524288] = {0};
            if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                printf("\n Socket creation error \n");
            }
            peer_addr.sin_family = AF_INET;
            peer_addr.sin_port = htons(stoi(peerads[1]));
            // cout<<peerAddr[2]<<endl;
            // Convert IPv4 and IPv6 addresses from text to binary
            // form
            if (inet_pton(AF_INET, peerads[0].c_str(), &peer_addr.sin_addr)
                <= 0) {
                printf(
                    "\nInvalid address/ Address not supported \n");
                // return -1;
            }

            if ((clientPeer_fd
                = connect(sock1, (struct sockaddr*)&peer_addr,
                        sizeof(peer_addr)))
                < 0) {
                printf("\nConnection Failed \n");
                // return -1;
            }
            cout<<"Connection established with peer\n";
            operation = "giveChunk";
            operation = operation + " " + command[1] + " " + command[2] + " " + to_string(i);
            strcpy(sendBuf, operation.c_str());
            send(sock1, sendBuf, strlen(sendMsg), 0);
            int x,k=0;
            long long bwrite = 0;
            char chunkBuf[524288] = {0};
            while((x = read(sock1, recFileBuf, sizeof(recFileBuf)))>0){
                // cout<<2<<" "; 
                // if(i==0){
                //     cout<<recFileBuf;
                bwrite += x;
                // cout<<x<<" ";
                for(int j=0;j<x;j++){
                    chunkBuf[k++] = recFileBuf[j];
                }
                bzero((char*)&recFileBuf, sizeof(&recFileBuf));

            }
            pwrite(df, &chunkBuf, bwrite, i*524288);

            // unsigned char chunkBuf[524288] = {0};
            //int x = pread(fd, chunkBuf, sizeof(chunkBuf), i*524288);
            //cout<<x<<endl;
            unsigned char shaBuf[10] = {0};
            SHA1((unsigned char*)chunkBuf, bwrite, shaBuf);

            string s;                 
            for(int i=0; i<10; i++){
                char buf[3];
                sprintf(buf, "%02x", shaBuf[i]&0xff);
                s += string(buf);
            }

            //cout<<s<<endl;
            string sha1 = peerAddr[2];
            // cout<<sha1<<endl;
            string chunksha;
            for(int k=i*20;k<i*20+20;k++){
                chunksha += sha1[k];
            }
            //cout<<chunksha<<endl;
            bzero((char*)&chunkBuf, sizeof(&chunkBuf));
            bwrite = 0;
            if(s == chunksha){
                receivedChunks++;
                cout<<"sha1 matched\n";
                long long receivedSize = std::filesystem::file_size(filePath);
                //setting file sharable: addingit to file map
                // cout<<1<<endl;
                fileToFilePaths[command[2]+" "+currentUser+" "+command[2]].second = command[1];
                string bitString = "";
                int chunks = ceil((std::filesystem::file_size(command[2]))/524288);
                for(int i=0;i<chunks;i++){
                    bitString += "1";
                }
                fileToFilePaths[command[2]+" "+currentUser+" "+command[2]].first = bitString;
                // cout<<2<<endl;
                //send tracker info that I am also seader
                string msg = "seader " + command[2] + " " + command[1];
                char msgBuf[1024] = {0};
                strcpy(msgBuf, msg.c_str());
                send(trackerSock, msgBuf, sizeof(msgBuf), 0);
                bzero((char*)&msgBuf, sizeof(msgBuf));
                int valread = read(trackerSock, msgBuf, 1024);
                // cout<<msgBuf<<endl;

            }

       }
       if(receivedChunks == noOfChunks){
            long long receivedSize = std::filesystem::file_size(filePath);
            //setting file sharable: addingit to file map
            //cout<<1<<endl;
            fileToFilePaths[command[2]+" "+currentUser+" "+command[2]].second = command[1];
            string bitString = "";
            int chunks = ceil((std::filesystem::file_size(command[2]))/524288);
            for(int i=0;i<chunks;i++){
                bitString += "1";
            }
            fileToFilePaths[command[2]+" "+currentUser+" "+command[2]].first = bitString;
            //cout<<2<<endl;
            //send tracker info that I am also seader
            string msg = "seader " + command[2] + " " + command[1];
            char msgBuf[1024] = {0};
            strcpy(msgBuf, msg.c_str());
            send(trackerSock, msgBuf, sizeof(msgBuf), 0);
            bzero((char*)&msgBuf, sizeof(msgBuf));
            int valread = read(trackerSock, msgBuf, 1024);
            cout<<msgBuf<<endl;        
       }
        // int n = cdThreads.size();
        // for(int i=0;i<n;i++){
        //     if(cdThreads[i].joinable())cdThreads[i].join();
        // }

        
        // char recFileBuf[524288] = {0};
        // string filePath = command[3] + command[2];
        // int df = open(filePath.c_str(), O_CREAT | O_RDWR | O_APPEND, 0644);
        // if(df<0){
        //     cout<<"Error\n";
        // }
        // cout<<df;
        // cout<<"File created\n";
        // int x;
        // while((x = read(sock1, recFileBuf, sizeof(recFileBuf)))>0){
        //     // cout<<2<<" "; 
        //     write(df, &recFileBuf, x);
        //     bzero((char*)&recFileBuf, sizeof(&recFileBuf));
        // }

        // cout<<"File created\n";

        // if(valread>0){
        //     long long receivedSize = std::filesystem::file_size(filePath);
        //     //setting file sharable: addingit to file map
        //     cout<<1<<endl;
        //     fileToFilePaths[command[2]+" "+currentUser+" "+command[2]].second = command[1];
        //     string bitString = "";
        //     int chunks = ceil((std::filesystem::file_size(command[2]))/524288);
        //     for(int i=0;i<chunks;i++){
        //         bitString += "1";
        //     }
        //     fileToFilePaths[command[2]+" "+currentUser+" "+command[2]].first = bitString;
        //     cout<<2<<endl;
        //     //send tracker info that I am also seader
        //     string msg = "seader " + command[2] + " " + command[1];
        //     char msgBuf[1024] = {0};
        //     strcpy(msgBuf, msg.c_str());
        //     send(trackerSock, msgBuf, sizeof(msgBuf), 0);
        //     bzero((char*)&msgBuf, sizeof(msgBuf));
        //     valread = read(trackerSock, msgBuf, 1024);
        //     cout<<msgBuf<<endl;

        // }
       
}

void sendChunk(vector<string> cmd, int peerSocket){
        int fd;
        char hello[524288] = {0};
        string fPath = fileToFilePaths[cmd[1] + " " + currentUser + " " + cmd[2]].second;
        //cout<<fPath<<endl;
        long long fSize = std::filesystem::file_size(fPath);
        //cout<<fSize<<endl;
        string bitstring = fileToFilePaths[cmd[1] + " " + currentUser + " " + cmd[2]].first;
        bitstring = bitstring + " " + to_string(fSize);
        //cout<<bitstring<<endl;
        if((fd = open(fPath.c_str(), O_RDONLY)) < 0){
            cout<<"Error in opening file\n";                
        }
        //cout<<cmd[1]<<endl;
        //cout<<2<<endl;
        char fileBuf[524288] = {0};
        //cout<<1<<endl;
        //cout<<fd<<endl;
        int chunkNo = stoi(cmd[3]);
        // ssize_t x = read(fd, fileBuf, 524288);
        // cout<<x<<endl;
        //bzero((char*)&hello, sizeof(hello));
        //strcpy(hello, bitstring.c_str());
        ///send(new_socket, hello, sizeof(hello), 0);
        //close(new_socket);
        //while(1){
            bzero((char*)&fileBuf, sizeof(&fileBuf));
            int x = pread(fd, fileBuf, 524288, chunkNo*524288);
            //cout<<"x is: "<<x<<endl;
            if(x<=0){
                cout<<"File complete\n";
                //break;
            }
            write(peerSocket, fileBuf, x);
            close(peerSocket);
        //}

}

void* serverService(void* param)
{
        int valread;
        int new_socket = *(int*)param;
        cout<<"\nListening\n";
        string str = "connection established";
        char hello[524288] = {0};
        char buffer[524288] = {0};
        strcpy(hello, str.c_str());
        // send(new_socket, hello, strlen(hello), 0);
        valread = read(new_socket, buffer, 524288);
        //cout<<hello<<valread<<endl;
        //cout<<buffer<<endl;
        str = convertToString(buffer);
        vector<string> cmd = splitString(str);
        if(cmd[0] == "download_file"){
            //cout<<new_socket<<endl;
            uThreads.push_back(thread(sendFile, cmd, new_socket));

        }
        else if(cmd[0] == "giveChunk"){
            uThreads.push_back(thread(sendChunk, cmd, new_socket));
        }
        // printf("Message from client.cpp in server mode: %s\n", buffer);
        //cout<<"Hello message sent\n";
        int n = uThreads.size();
        for(int i=0;i<n;i++){
            if(uThreads[i].joinable())
                uThreads[i].join();
        }
        // n = ucThreads.size();
        // for(int i=0;i<n;i++){
        //     if(ucThreads[i].joinable())
        //         ucThreads[i].join();
        // }
        close(new_socket);
        return param;
}



class thread_obj {
public:
    void operator()()
    {

			int i = 0;
			int server_fd, new_socket, valread;
			struct sockaddr_in address;
			int opt = 1;
			int addrlen = sizeof(address);
			char buffer[524288] = { 0 };
			char hello[524288] = "Hello from server";
            pthread_t tid;

			// Creating socket file descriptor
			if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				perror("socket failed");
				exit(EXIT_FAILURE);
			}

			// Forcefully attaching socket to the port 8080
			if (setsockopt(server_fd, SOL_SOCKET,
						SO_REUSEADDR | SO_REUSEPORT, &opt,
						sizeof(opt))) {
				perror("setsockopt");
				exit(EXIT_FAILURE);
			}
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons(PORT);

			// Forcefully attaching socket to the port 8080
			if (bind(server_fd, (struct sockaddr*)&address,
					sizeof(address))
				< 0) {
				perror("bind failed");
				exit(EXIT_FAILURE);
			}
			if (listen(server_fd, 5) < 0) {
				perror("listen");
				exit(EXIT_FAILURE);
			}
			cout<<"\nserver is UP\n";

		while (1)
		{
			if ((new_socket
				= accept(server_fd, (struct sockaddr*)&address,
						(socklen_t*)&addrlen))
				< 0) {

				perror("accept");
				// exit(EXIT_FAILURE);
			}
			pthread_create(&serverThreads[i++], NULL,  serverService, &new_socket);
		}
		


			// closing the connected socket
			// closing the listening socket
			shutdown(server_fd, SHUT_RDWR);

		}
};

int main()
{

	thread t1 = thread(thread_obj());

        int sock = 0, valread, client_fd;
        struct sockaddr_in serv_addr;
        char hello[524288] = "Hello from client";
        char buffer[524288] = { 0 };
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(TrackerPORT);

        // Convert IPv4 and IPv6 addresses from text to binary
        // form
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
            <= 0) {
            printf(
                "\nInvalid address/ Address not supported \n");
            // return -1;
        }

        if ((client_fd
            = connect(sock, (struct sockaddr*)&serv_addr,
                    sizeof(serv_addr)))
            < 0) {
            printf("\nConnection Failed \n");
            // return -1;
        }
	while(1){
        // valread = read(sock, buffer, 1024);
        // printf("%s\n", buffer);
        //vector<string> input;

 
        string inputCommand;
        string input;
        getline(cin, inputCommand);
        string s;
        // cout<<">>";
        vector<string> command = splitString(inputCommand);
        //cin>>s;
        if(command[0] == "login" || command[0] == "create_user"  || command[0] == "accept_request" || command[0] == "upload_file" || command[0] == "stop_share"){
            if(command.size() != 3){
                cout<<"Invalid command\n";
                continue;
            }
            if(command[0] == "upload_file"){
                string sha1 = "";
                struct stat st; 
                long long fSize=0;
                int fd = open(command[1].c_str(), O_RDONLY);
                if (stat(command[1].c_str(), &st) == 0)
                    fSize = st.st_size;
                int chunks = ceil(fSize/524288.0);
                for(int i=0;i<chunks;i++){
                    unsigned char chunkBuf[524288] = {0};
                    int x = pread(fd, chunkBuf, sizeof(chunkBuf), i*524288);
                    //cout<<x<<endl;
                    unsigned char shaBuf[10] = {0};
                    SHA1(chunkBuf, x, shaBuf);

                    string s;                 
                    for(int i=0; i<10; i++){
                        char buf[3];
                        sprintf(buf, "%02x", shaBuf[i]&0xff);
                        s += string(buf);
                    }

                    //cout<<s<<endl;
                    sha1 += s;
                }
                //cout<<sha1<<endl;
                inputCommand += " " + sha1;
            }            
            int inpLength = inputCommand.size();
            char inpCommand[inpLength + 1];
            strcpy(inpCommand, inputCommand.c_str());
            send(sock, inpCommand, strlen(inpCommand), 0);
            //cout<<inpCommand<<endl;
            bzero((char*)&buffer, sizeof(buffer));
            valread = read(sock, buffer, 524288);
            cout<<buffer<<endl;
            string str = convertToString(buffer);
            if(str == "login success"){
                //cout<<1<<endl;
                string ipPort = "127.0.0.1 " + to_string(PORT);
                bzero((char*)&inpCommand, sizeof(inpCommand));
                strcpy(inpCommand, ipPort.c_str());
                cout<<ipPort<<" "<<inpCommand<<endl;
                send(sock, inpCommand, strlen(inpCommand), 0);
                currentUser = command[1];
            }
            else if(str == "User created successfully"){
                cout<<"User created successfully\n";                
            }
            else if(str == "File Uploaded"){
                string filename = command[1].substr(command[1].find_last_of("/") + 1);
                fileToFilePaths[command[2]+" "+currentUser+" "+filename].second = command[1];
                string bitString = "";
                struct stat st; 
                long long fSize=0;
                if (stat(command[1].c_str(), &st) == 0)
                    fSize = st.st_size;
                int chunks = ceil(fSize/524288.0);
                for(int i=0;i<chunks;i++){
                    bitString += "1";
                }
                fileToFilePaths[command[2]+" "+currentUser+" "+filename].first = bitString;
            }
        }

        else if(command[0] == "create_group" || command[0] == "join_group" || command[0] == "leave_group" || command[0] == "list_requests" || command[0] == "list_files"){
            if(command.size() != 2){
                cout<<"Invalid command\n";
                continue;
            }
            int inpLength = inputCommand.size();
            char inpCommand[inpLength + 1];
            strcpy(inpCommand, inputCommand.c_str());
            send(sock, inpCommand, strlen(inpCommand), 0);
            //cout<<inpCommand<<endl;

            bzero((char*)&buffer, sizeof(buffer));
            valread = read(sock, buffer, 524288);
            string str = convertToString(buffer);
            cout<<buffer<<str<<endl;
            if(str.size() == 0)
                cout<<0<<endl;
            if(str == "Group created successfully"){
                cout<<str<<endl;
            }
            else if(str == "requested join"){
                cout<<str<<endl;
            }
            else if(str == "Requests Listed"){
                vector<string> v = splitString(str);
                int n = v.size();
                cout<<n<<endl;
                for(int i=0;i<n;i++){
                    cout<<v[i]<<endl;
                }
            }
            else if(command[0] == "list_files"){
                vector<string> v = splitString(str);
                int n = v.size();
                cout<<n<<endl;
                for(int i=0;i<n;i++){
                    cout<<v[i]<<endl;
                }
            }

            
        }

        else if(command[0] == "list_groups"){
            if(command.size() != 1){
                cout<<"Invalid command\n";
                continue;
            }

            int inpLength = inputCommand.size();
            char inpCommand[inpLength + 1];
            strcpy(inpCommand, inputCommand.c_str());
            send(sock, inpCommand, strlen(inpCommand), 0);
            cout<<inpCommand<<endl;

            bzero((char*)&buffer, sizeof(buffer));
            valread = read(sock, buffer, 524288);
            string str = convertToString(buffer);
            cout<<buffer<<str<<endl;
            if(str == "Groups Listed"){
                vector<string> v = splitString(str);
                int n = v.size();
                cout<<n<<endl;
                for(int i=0;i<n;i++){
                    cout<<v[i]<<endl;
                }                
            }
        }

        else if(command[0] == "download_file"){
            if(command.size() != 4){
                cout<<"Invalid command\n";
                continue;
            }
            // cout<<input<<endl;
            int inpLength = inputCommand.size();
            char inpCommand[inpLength + 1];
            strcpy(inpCommand, inputCommand.c_str());
            //cout<<input<<endl;
            send(sock, inpCommand, strlen(inpCommand), 0);
            //cout<<inpCommand<<endl;
            bzero((char*)&buffer, sizeof(buffer));
            valread = read(sock, buffer, 524288);
            cout<<buffer<<endl<<1<<endl;
            string str = convertToString(buffer);
            bzero((char*)&buffer, sizeof(buffer));
            vector<string> peerAddr = splitString(str);
            if(peerAddr[0] == "success"){
                // bzero((char*)&buffer, sizeof(buffer));
                string operation = "download_file";
                operation = operation  + " " + command[1] + " " + command[2];
                //cout<<peerAddr[1]<<endl;
                
                dThreads.push_back(thread(receiveFile, operation, command, peerAddr, sock));
            }

        }
        else if(command[0] == "logout"){
            int inpLength = inputCommand.size();
            char inpCommand[inpLength + 1];
            strcpy(inpCommand, inputCommand.c_str());
            //cout<<input<<endl;
            send(sock, inpCommand, strlen(inpCommand), 0);
            bzero((char*)&buffer, sizeof(buffer));
            valread = read(sock, buffer, 524288);
            cout<<buffer<<endl<<1<<endl;

        }
        else if(command[0] == "close_connection"){
            break;
        }
        else{
            cout<<"Invalid command\n";
        }
        // closing the connected socket
        // close(client_fd);


	}
    close(client_fd);
    int n = dThreads.size();
    for(int i=0;i<n;i++){
        if(dThreads[i].joinable())dThreads[i].join();
    }
	t1.join();

	return 0;
}
