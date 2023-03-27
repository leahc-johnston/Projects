#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <fstream>

using namespace std;
string getoutput(string command){
	int fds[2];
	pipe(fds);
	string return_value;
	pid_t pid = fork();
	if(pid)
	{
		close(fds[1]);
		while(true){
			char buffer[1024];
			ssize_t readlen = read(fds[0], buffer, 1024);
			if(readlen < 1)
				break;
			buffer[readlen] = 0;
			return_value += string(buffer);
		}
		wait(0);
		close(fds[0]);
	}
	else{
		dup2(fds[1], 1);
		system(command.c_str());
		exit(0);
	}
	return return_value;
}
int main()
{
	int pid = fork();
	int stat;
	int i = 0;
	int j = 0;
	if(pid)
		return 0;
	while(1){
		int error = system("last -p now | grep lcjohns > /dev/null");
		//int error_2 = system("last -p now | cut -d \" \" -f1 | grep -v wtmp | sort -u > daemon_info.txt");
		//string num_users = getoutput("last -p now | cut -d \" \" -f1 | grep -v wtmp | sort -u | awk 'NR>1' | wc -l"); this commands did not support longer usernames 
		int error_2 = system("finger | cut -d \" \" -f1 | awk 'NR>1' > daemon_info.txt");
		string num_users = getoutput("finger | cut -d \" \" -f1 | awk 'NR>1' | wc -l");
		int num_users_int = stoi(num_users);
		if(error == 0)
		{
			if(i < 1){
				system("echo Greetings Leah | write lcjohnston");
			}
				
			i++;
		}
		else 
		{
			i = 0;	
		}
		if(error_2 == 0)
		{
			string array[num_users_int];
			ifstream pf;
			string name;
			pf.open("daemon_info.txt");
			for(int p=0; p<num_users_int; p++)
			{
				pf >> name;
				array[p] = name;

			}
			pf.close();
			for(int p=0; p<num_users_int; p++)
			{
				string search = "ls -a -R | grep ";
				string line = array[p];
				search += line;
				int message = system(search.c_str());
				if(message == 0)
				{
					cout << "Found a match!\n";
					string write = "cat .outgoing/";
					write += array[p];
					write += " | write ";
					write += array[p];
					string r = ".outgoing/";
					r += array[p];
					system(write.c_str());
					remove(r.c_str());
				}
				/*else
					cout << "no message for " << array[p] << endl;*/
			}
		}
		sleep(5);
	}
}
