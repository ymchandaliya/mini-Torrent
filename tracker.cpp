#include <bits/stdc++.h>
#include <thread>
#include <pthread.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include<dirent.h>
#include<sys/stat.h>
#include <fstream>
using namespace std;
using std::ifstream;

#define PORT 6000

pthread_t serverThreads[100];

class fileInfo{
    public:
    // string filePath;
    string fileName;
    set<string> seaders;
    set<string> leachers;
    string sha1;
    long long size;

};

unordered_map<string, string> userData;              // uid:password
unordered_map<string, vector<string>> ipActive;      // uname: <ip, port>//of their server
unordered_map<string, pair<string, set<string>>> groupInfo;     // group id: pair<owner,<members of group>>
unordered_map<string, set<string>> groupRequests; // group id: <user ids of requested>
// unordered_map<string, string>files;                  //fileName:filePath
unordered_map<string, fileInfo>fileDetails;          //groupId fileName: fileInfo object



string convertToString(char *a)
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

int createUser(vector<string> in)
{
    if (userData.find(in[1]) != userData.end())
    {
        cout << "Username already exists, please try another username\n";
        return 0;
    }
    else
    {
        userData.insert({in[1], in[2]});
        return 1;
    }
}

int login(vector<string> in)
{
    if (userData.size() == 0)
        return 0;
    if (userData.find(in[1]) == userData.end())
    {
        // cout<<"Invalid username\n";
        return 0;
    }
    else if (userData[in[1]] != in[2])
    {
        // cout<<"Invalid Password\n";
        return 0;
    }
    else if (ipActive.find(in[1]) != ipActive.end())
    {
        // cout<<"already Logged in\n";
        return 0;
    }
    else
    {
        vector<string> v;
        ipActive[in[1]] = v;
        return 1;
    }

    return 0;
}

int createGroup(vector<string> in, string owner)
{
    if (groupInfo.find(in[1]) != groupInfo.end())
    {
        return 0;
    }

    else
    {
        set<string> v;
        v.insert(owner);
        groupInfo[in[1]] = {owner,v};
        return 1;
    }
}

int joinGroup(vector<string> in, string requester)
{

    if (groupInfo.find(in[1]) != groupInfo.end())
    {
        if (groupRequests.find(in[1]) == groupRequests.end()){
            if (find(groupInfo[in[1]].second.begin(), groupInfo[in[1]].second.end(), requester) != groupInfo[in[1]].second.end() || find(groupRequests[in[1]].begin(), groupRequests[in[1]].end(), requester) != groupRequests[in[1]].end() ){
                return 0;
            }
            else{
                set<string> v;
                v.insert(requester);
                groupRequests[in[1]] = v;
                cout<<2;
                return 1;
            }
        }
        else{
            groupRequests[in[1]].insert(requester);
            cout<<1;
            return 1;
        }
    }

    else
    {
        return 0;
    }
}

string listRequests(vector<string> in, string requester){
    cout<<groupRequests.size()<<" greq\n";
    if(groupInfo[in[1]].first != requester || groupRequests.find(in[1]) == groupRequests.end()){
        cout<<1;
        return "0";
    }
    else{
        cout<<2;
        string ans = "";
        int n = groupRequests[in[1]].size();

        for(auto itr : groupRequests[in[1]]){
            ans += itr + " ";
        }

        // for(int i=0;i<n;i++){
        //     ans += groupRequests[in[1]][i] + " ";
        // }
        return ans;
    }
}

int leaveGroup(vector<string> in, string requester){
    if(groupInfo.size()>0 && find(groupInfo[in[1]].second.begin(), groupInfo[in[1]].second.end(), requester) != groupInfo[in[1]].second.end()){
        groupInfo[in[1]].second.erase(find(groupInfo[in[1]].second.begin(), groupInfo[in[1]].second.end(),requester));
        
        //group is empty so deleting the group and joining requests
        if(groupInfo[in[1]].second.size() == 0){
            groupInfo.erase(in[1]);
            groupRequests.erase(in[1]);
            for(auto it:fileDetails){
                it.second.leachers.erase(requester);
                it.second.seaders.erase(requester);
            }            
            return 1;
        }

        //if owner is deketed making new owner
        else if(requester == groupInfo[in[1]].first){
            auto newOwner = groupInfo[in[1]].second.begin();
            groupInfo[in[1]].first = *newOwner;
            for(auto it:fileDetails){
                it.second.leachers.erase(requester);
                it.second.seaders.erase(requester);
            }            
    
            return 1;
        }
    }
    else{
        return 0;
    }
    return 0;
}

string listGroups(vector<string> in){
    if(groupInfo.size()>0){
        string s = "";
        for (auto it = groupInfo.cbegin(); it != groupInfo.cend(); ++it) {
            s += (*it).first + " ";
        }

        return s;

    }
    else{
        return "0";
    }
}

int acceptRequest(vector<string> in, string requester){
    cout<<"gi "<<groupInfo.size()<<" gr"<<groupRequests.size()<<endl;
    //there are some group requests
    if(groupRequests.size()>0){
        //requested group has somr requests
        if(groupRequests[in[1]].size()>0){
            //there is requested group
            if(groupInfo.size()>0 && groupInfo.find(in[1]) != groupInfo.end()){
                //you are owner and this request exists
                if(groupInfo[in[1]].first == requester && find(groupRequests[in[1]].begin(), groupRequests[in[1]].end(), in[2]) != groupRequests[in[1]].end()){
                    //add him to the group
                    groupInfo[in[1]].second.insert(in[2]);
                    //delete from request queue
                    groupRequests[in[1]].erase(find(groupRequests[in[1]].begin(), groupRequests[in[1]].end(), in[2]));
                    return 1;
                }
                else if(groupInfo[in[1]].first != requester){
                    return 4;//you are not owner
                }
                else{
                    return 5;//no request from in[2] exists
                }
            }
            else{
                return 2;//such group doesn't exist
            }
        }
        else{
            return 3;//no such request exists
        }
    }
    return 0;
}

int fileUpload(vector<string> in, string requester){   
    string group = in[2], filePath = in[1];
    string fName = filePath.substr(filePath.find_last_of("/") + 1);
    // files[fName] = filePath;
}

void *serverService(void *param)
{
    bool loggedIn = false;
    string peerId = "";
    int new_socket = *(int *)param;

    while (1)
    {
        int valread;
        cout << "\nListening\n";
        // string str = "connection established";
        char hello[524288] = {0};
        char buffer[524288] = {0};
        // strcpy(hello, str.c_str());
        // cout<<1;
        // send(new_socket, hello, strlen(hello), 0);
        // cout<<2;
        valread = read(new_socket, buffer, 524288);
        //         cout<<buffer;
        string cmd = convertToString(buffer);
        cout << cmd;
        vector<string> cmdVector = splitString(cmd);
        // for(int i=0;i<cmdVector.size();i++){
        //     cout<<cmdVector[i]<<endl;
        // }
        if (cmdVector[0] == "login")
        {
            int res = login(cmdVector);
            // cout<<endl<<res;
            if (res == 1)
            {
                string uid = cmdVector[1];
                string str = "login success";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);
                bzero((char *)&buffer, sizeof(buffer));
                valread = read(new_socket, buffer, 524288);
                cmd = buffer;
                cout << buffer << endl;
                cmdVector.clear();
                cmdVector = splitString(cmd);
                ipActive[uid].push_back(cmdVector[0]);
                ipActive[uid].push_back(cmdVector[1]);
                peerId = uid;
                loggedIn = true;
            }
            else
            {
                cout << "Not logged in\n";
                string str = "login Failed";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);
                // continue;
            }
        }

        else if (cmdVector[0] == "create_user")
        {
            int res = createUser(cmdVector);
            if (res == 1)
            {
                string str = "User created successfully";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);
            }
            else{
                string str = "create user Failed";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);
            }
        }

        else if (cmdVector[0] == "create_group")
        {
            // int res = createGroup(cmdVector);

            if (loggedIn)
            {
                int res = createGroup(cmdVector, peerId);
                cout << res << endl;
                cout<<groupInfo.size();
                if (res == 1)
                {
                    string str = "Group created successfully";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
                else{
                    string str = "create group Failed";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
            }
            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);
            }
        }

        else if (cmdVector[0] == "join_group")
        {
            // int res = createGroup(cmdVector);
            cout<<groupRequests.size()<<" inside join\n";
            if (loggedIn)
            {
                int res = joinGroup(cmdVector, peerId);
                // cout<<res<<endl;
                if (res == 1)
                {
                    string str = "join requested";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
                else{
                    cout<<groupRequests.size()<<" inside else\n";
                        // for (auto it = groupRequests.cbegin(); it != groupRequests.cend(); ++it) {
                        //    std::cout << "{" << (*it).first << "}\n";
                        // }
                    string str = "Invalid join request";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
            }
            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);
            }
        }

        else if(cmdVector[0] == "list_requests"){
            if(loggedIn){
                string res = listRequests(cmdVector, peerId);
                cout<<res<<endl;
                if(res != "0"){
                    string str = "Requests Listed";
                    char hello[524288] = {0};
                    strcpy(hello, res.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
                else{
                    string str = "Invalid Group Id";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
            }
            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);
            }
        }

        else if(cmdVector[0] == "leave_group"){
            if(loggedIn){
                int res = leaveGroup(cmdVector, peerId);
                cout<<res<<endl;
                if(res == 1){
                    string str = "Left the group";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
                else{
                    string str = "Invalid Request";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
            }
            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);
            }
        }

        else if(cmdVector[0] == "list_groups"){
            if(loggedIn){
                string res = listGroups(cmdVector);
                cout<<res<<endl;
                if(res != "0"){
                    string str = "Groups Listed";
                    char hello[524288] = {0};
                    strcpy(hello, res.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
                else{
                    string str = "No groups exists";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
            }
            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);
            }
        }

        else if(cmdVector[0] == "accept_request"){
            if(loggedIn){
                int res = acceptRequest(cmdVector, peerId);
                cout<<res<<endl;
                if(res == 1){
                    string str = "Request Accepted";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
                else if(res == 2){
                    string str = "No such group exists";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
                else if(res == 3){
                    string str = "No such request exists";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
                else if(res == 4){
                    string str = "You are not owner of "+cmdVector[1];
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
                else if(res == 5){
                    string str = cmdVector[2]+" Mase No such request";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
                else{
                    string str = "Invalid request";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);
                }
            }
            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);
            }
        }

        else if(cmdVector[0] == "upload_file"){
            //cout<<"size: "<<std::filesystem::file_size(cmdVector[1])<<endl;
            if(loggedIn){
                string path = cmdVector[1];
                string filename = path.substr(path.find_last_of("/") + 1);
                cout<<filename<<endl;
                //files[filename] = path;
                if(groupInfo.size()>0 && groupInfo.find(cmdVector[2]) != groupInfo.end()){

                    if(groupInfo[cmdVector[2]].second.find(peerId) != groupInfo[cmdVector[2]].second.end()){
                        if(fileDetails.find(cmdVector[2] + " " + filename) == fileDetails.end()){
                            fileDetails[cmdVector[2] + " " + filename].seaders.insert(peerId);
                            struct stat st; 
                            if (stat(cmdVector[1].c_str(), &st) == 0)
                                fileDetails[cmdVector[2] + " " + filename].size = st.st_size;;
                            
                            fileDetails[cmdVector[2] + " " + filename].sha1 = cmdVector[3];
                            // cout<<cmdVector[3]<<endl;
                            string str = "File Uploaded";
                            char hello[524288] = {0};
                            strcpy(hello, str.c_str());
                            send(new_socket, hello, strlen(hello), 0);
                        }
                        else{
                            fileDetails[cmdVector[2] + " " + filename].seaders.insert(peerId);
                            string str = "File Uploaded";
                            char hello[524288] = {0};
                            strcpy(hello, str.c_str());
                            send(new_socket, hello, strlen(hello), 0);                        
                        }
                    }

                    else{
                        string str = "You are not part of this group";
                        char hello[524288] = {0};
                        strcpy(hello, str.c_str());
                        send(new_socket, hello, strlen(hello), 0);                        
                    }                    

                }
                else{
                    string str = "No such group exist";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);                
                }
            }
            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);                
            }

        }

        else if(cmdVector[0] == "download_file"){

            if(loggedIn){
                if(groupInfo.size()>0 && groupInfo[cmdVector[1]].second.find(peerId) != groupInfo[cmdVector[1]].second.end()){
                    // string fPath = files[cmdVector[2]] + " " + ipActive[fileSeaderMap[cmdVector[2]]][0] + " " + ipActive[fileSeaderMap[cmdVector[2]]][1];
                    if(fileDetails.size()>0 && fileDetails.find(cmdVector[1] + " " + cmdVector[2])!= fileDetails.end()){
                        string seadersInfo = to_string(fileDetails[cmdVector[1] + " " + cmdVector[2]].size) + " " + fileDetails[cmdVector[1] + " " + cmdVector[2]].sha1 + " ";
                        set<string> seaders = fileDetails[cmdVector[1] + " " + cmdVector[2]].seaders;
                        for(auto it:seaders){
                            if(ipActive.size()>0 && ipActive.find(it) != ipActive.end()){
                                seadersInfo += ipActive[it][0] + " " + ipActive[it][1] + " ";
                            }
                        }
                        vector<string> temp = splitString(seadersInfo);
                        if(temp.size() < 3){
                            string str = "No seaders available for this file right now";
                            char hello[524288] = {0};
                            strcpy(hello, str.c_str());
                            send(new_socket, hello, strlen(hello), 0);                        
                        }
                        else{
                            seadersInfo = "success " + seadersInfo;
                            char hello[524288] = {0};
                            strcpy(hello, seadersInfo.c_str());
                            cout<<seadersInfo<<endl;
                            send(new_socket, hello, strlen(hello), 0);
                        }
                        
                    }
                    else{
                        string str = "No such file is present in the group";
                        char hello[524288] = {0};
                        strcpy(hello, str.c_str());
                        send(new_socket, hello, strlen(hello), 0);                        
                    }


                }
                else{
                    string str = "You are not member of group";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);                
                }
            }

            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);                
            }

            //string ipPort = ipActive[fileSeaderMap[cmdVector[2]]][0] + " " + ipActive[fileSeaderMap[cmdVector[2]]][1];
            //bzero((char *)&hello, sizeof(hello));
            //cout<<ipPort<<endl;
            //strcpy(hello, ipPort.c_str());
            //send(new_socket, hello, strlen(hello), 0);

        }

        else if(cmdVector[0] == "seader"){
            if(loggedIn){
                string filename = cmdVector[1];
                // string filename = path.substr(path.find_last_of("/") + 1);
                cout<<filename<<endl;
                //files[filename] = path;
                if(groupInfo.size()>0 && groupInfo.find(cmdVector[2]) != groupInfo.end()){

                    if(groupInfo[cmdVector[2]].second.find(peerId) != groupInfo[cmdVector[2]].second.end()){
                        //if(fileDetails.find(cmdVector[2] + " " + filename) == fileDetails.end()){
                            fileDetails[cmdVector[2] + " " + filename].seaders.insert(peerId);
                            //fileDetails[cmdVector[2] + " " + filename].size = std::filesystem::file_size(cmdVector[1]);
                            string str = "File Uploaded";
                            char hello[524288] = {0};
                            strcpy(hello, str.c_str());
                            send(new_socket, hello, strlen(hello), 0);

                            set<string> temp = fileDetails[cmdVector[2] + " " + filename].seaders;
                            for(auto it: temp){
                                cout<<it<<" ";
                            }
                        //}
                        // else{
                        //     string str = "File is already present in the group";
                        //     char hello[524288] = {0};
                        //     strcpy(hello, str.c_str());
                        //     send(new_socket, hello, strlen(hello), 0);                        
                        // }
                    }

                    else{
                        string str = "You are not part of this group";
                        char hello[524288] = {0};
                        strcpy(hello, str.c_str());
                        send(new_socket, hello, strlen(hello), 0);                        
                    }                    

                }
                else{
                    string str = "No such group exist";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);                
                }
            }
            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);                
            }

        }
        else if(cmdVector[0] == "logout"){
            if(loggedIn){
                // for(auto it:fileDetails){
                //     it.second.leachers.erase(peerId);
                //     it.second.seaders.erase(peerId);
                // }
                ipActive.erase(peerId);
                loggedIn = false;
                string str = "You are logged out\n";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);  
            }
            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);                
            }
        }
        else if(cmdVector[0] == "list_files"){
            if(loggedIn){
                string gName = cmdVector[1];
                string res = "";
                for(auto it:fileDetails){
                    string t = it.first;
                    vector<string> gF = splitString(t);
                    if(gF[0] == gName){
                        res += gF[1] + " ";
                    }
                }
                char hello[524288] = {0};
                strcpy(hello, res.c_str());
                send(new_socket, hello, strlen(hello), 0);                

            }
            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);                
            }
        }
        else if(cmdVector[0] == "stop_share"){
            if(loggedIn){
                if(groupInfo.find(cmdVector[1]) != groupInfo.end()){
                    if(groupInfo[cmdVector[1]].second.find(peerId) != groupInfo[cmdVector[1]].second.end()){
                        fileDetails[cmdVector[1] + " " + cmdVector[2]].seaders.erase(peerId);
                        string str = "You stopped Sharing\n";
                        char hello[524288] = {0};
                        strcpy(hello, str.c_str());
                        send(new_socket, hello, strlen(hello), 0); 
                    }
                    else{
                        string str = "You are not part of this group\n";
                        char hello[524288] = {0};
                        strcpy(hello, str.c_str());
                        send(new_socket, hello, strlen(hello), 0);  
        
                    }
                }
                else{
                    string str = "NO Such group exists\n";
                    char hello[524288] = {0};
                    strcpy(hello, str.c_str());
                    send(new_socket, hello, strlen(hello), 0);  
                }
 
            }
            else{
                string str = "Please login First";
                char hello[524288] = {0};
                strcpy(hello, str.c_str());
                send(new_socket, hello, strlen(hello), 0);                
            }
        }
    }
    close(new_socket);

    return NULL;
}

class thread_obj
{
public:
    void operator()()
    {

        int i = 0;
        int server_fd, new_socket, valread;
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);
        char buffer[524288] = {0};
        char hello[524288] = "Hello from server";
        pthread_t tid;

        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Forcefully attaching socket to the port 8080
        if (setsockopt(server_fd, SOL_SOCKET,
                       SO_REUSEADDR | SO_REUSEPORT, &opt,
                       sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        // Forcefully attaching socket to the port 8080
        if (bind(server_fd, (struct sockaddr *)&address,
                 sizeof(address)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(server_fd, 5) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        cout << "\nTracker is UP\n";

        while (1)
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                     (socklen_t *)&addrlen)) < 0)
            {

                perror("accept");
                // char buffer1[INET_ADDRSTRLEN];
                // inet_ntop( AF_INET, &address.sin_port, buffer1, sizeof( buffer1 ));
                // cout<<inet_ntop(address.sin_addr)<<address.sin_port<<endl;
                // cout<<buffer1<<endl;
                exit(EXIT_FAILURE);
            }
            // close(new_socket);

            pthread_create(&serverThreads[i++], NULL, serverService, &new_socket);
            // pthread_join(serverThreads[--i], NULL); 
        }
        close(new_socket);
        shutdown(server_fd, SHUT_RDWR);
    }
};

void quitIt(){

    while(1){
        string s;
        cin>>s;
        if(s == "quit"){
            cout<<"Closing tracker\n";
            exit(1);
        }
    }
}

int main()
{

        thread t1 = thread(quitIt);
        int i = 0;
        int server_fd, new_socket, valread;
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);
        char buffer[524288] = {0};
        char hello[524288] = "Hello from server";
        pthread_t tid;

        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Forcefully attaching socket to the port 8080
        if (setsockopt(server_fd, SOL_SOCKET,
                       SO_REUSEADDR | SO_REUSEPORT, &opt,
                       sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        // Forcefully attaching socket to the port 8080
        if (bind(server_fd, (struct sockaddr *)&address,
                 sizeof(address)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(server_fd, 5) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        cout << "\nTracker is UP\n";

        while (1)
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                     (socklen_t *)&addrlen)) < 0)
            {

                perror("accept");
                // char buffer1[INET_ADDRSTRLEN];
                // inet_ntop( AF_INET, &address.sin_port, buffer1, sizeof( buffer1 ));
                // cout<<inet_ntop(address.sin_addr)<<address.sin_port<<endl;
                // cout<<buffer1<<endl;
                exit(EXIT_FAILURE);
            }
            // close(new_socket);

            pthread_create(&serverThreads[i++], NULL, serverService, &new_socket);
            // pthread_join(serverThreads[--i], NULL); 
        }
        close(new_socket);
        shutdown(server_fd, SHUT_RDWR);

    // vector<string> input;
    // string s;
    // cin>>s;
    // if(s == "login" || s == "create_user"){
    //     input.push_back(s);
    //     cin>>s;
    //     input.push_back(s);
    //     cin>>s;
    //     input.push_back(s);
    // }

   t1.join();
    return 0;
}