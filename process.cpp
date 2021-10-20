#include <sys/types.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include <unordered_map>

using namespace std;

#define PORT 2021
#define BUFFER_SIZE 4096

bool active = true;
vector<int> others;
char buffer[BUFFER_SIZE];

double weight = 0;

bool alive = true;

int generate_event(){
	int p = rand();
	if(p < 100){
		p = p%(2*others.size()+1);
		if(p >= others.size()){
			return -1;
		}
		else{
			return others[p];
		}
	}
	return -2;
}

void makeActive(){
	active = true;
}

void makeIdle(){
	active = false;
}

void process_message(char * _m,int sock){
	string m = _m;
	stringstream ss;
	ss << m;
	string cur;
	getline(ss, cur, '$');
	int command = stoi(cur);
	if(command == -2){
		alive = false;
	}
	else if(command == -1){
		getline(ss, cur, '$');
		others.push_back(stoi(cur));
	}
	else{
		getline(ss, cur, '$');
		weight += stod(cur);
		if(!active){
			cout << "----Active------\n";
			cout << " >> Message recieved from Process "<< command - sock <<"\n\t weight = " << weight <<"\n" ;
			makeActive();
		}
	}
}



void print(vector<int> v){
        cout<<" [ ";
        for(int i=0;i<v.size();i++)
                cout<<v[i] <<" , ";
        if(v.size() == 0)
                cout<< " ]\n";
        else
                cout<<"\b\b ]\n";
}


void _read(int sock){
	while(alive){
		read(sock,buffer,BUFFER_SIZE);
		process_message(buffer,sock);
	}
}

int main(int argc, char const *argv[]){
	//create socket
	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	int me;

	// SOCKET CONFIG
	sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_port = htons(PORT);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

	// connect client
	if (connect(sock, (sockaddr *)&server_address, sizeof(server_address)) == -1){
		perror("Cant Connect");
		exit(-1);
	}
	//cout << "connected. \t";

	read (sock , buffer, BUFFER_SIZE);
	string str = buffer;
	stringstream ss(str);
	string s;
	getline(ss,s,'$');
	me = stoi(s);
	getline(ss,s,'$');
	weight = stod(s);
	while(getline(ss,s,'$')){
		others.push_back(stoi(s));
	}
	cout << "PROCESS # " << me-sock <<endl;

	thread t1(_read, sock);

	srand(time(0));

	cout << "----Active------\n";
	cout << "weight = " << weight << "\n";

	while(alive){
		//print(others);
		if(active){
			int e = generate_event();
			if(e != -2){
				if(e != -1){
					cout << " >> Sending message to Process " << e - sock << "\n";
					ostringstream ss;
					weight /= 2;
					ss << e << "$" << weight;
					string m = ss.str();
					write(sock, m.c_str(), m.size() + 1);
				}
			}
			int v = rand();
			if(v < 20){
				cout << "-----Idle-----\n";
				ostringstream ss;
				ss << -10 << "$" << weight;
				string m = ss.str();
				write(sock, m.c_str(), m.size() + 1);
				weight = 0;
				makeIdle();
			}
		}
	}
	string end = "-1";
	write(sock,end.c_str(),end.size()+1);

	t1.join();
	close(sock);

	cout << "Process Terminated\n";

	return 0;
}
