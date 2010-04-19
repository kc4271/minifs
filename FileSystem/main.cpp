#include <iostream>
#include "filesystem.h"
using namespace std;

int main()
{
	/*
	char buf[1000];
	_ctime64_s(buf,1000,&t); 
	cout<<buf<<endl;
	*/
	
	::miniFileSystem fs;
	fs.create_disk("music",60000);
	Disk &disk = fs.dskmounted[fs.curr];
	/*
	ifstream in("D:\\filelist.txt");
	string s;
	int i = 0,base = 10011047;
	char names[100];
	
	while(in>>s)
	{
		sprintf(names,"%d.wav",base+i);
		i++;
		struct _stat info;
		_stat(s.c_str(), &info);
		unsigned int size = info.st_size;

		ifstream wav(s.c_str(),ios::binary);
		if(wav.fail())
			cout<<"wav fail"<<endl;
		char *buf = new char[size];
		wav.read(buf,size);
		wav.close();
		
		disk.create_file(names);
		unsigned int file;
		disk.open_file(names,&file);
		disk.write_file(file,buf,size);
		disk.close_file(file);
		delete []buf;
	}
	disk.directory();
	disk.save_to_file();
	in.close();
	*/
	
	disk.init_from_file();
	for(unsigned int i = 0;i < disk.blocknum;i++)
	{
		if(has_data(i,disk.pdirectory))
		{
			unsigned int file;
			disk.open_file(disk.fdes[i].get_name().c_str(),&file);
			char *buf = new char[disk.get_file_size(i)];
			disk.read_file(file,buf,(unsigned int)disk.get_file_size(i));
			string s = "d:\\wav\\";
			s += disk.fdes[i].get_name();
			ofstream out(s.c_str(),ios::binary);
			if(out.fail())
			{
				cout<<"faile!"<<endl;
			}
			out.write(buf,disk.get_file_size(file));
			out.flush();
			out.close();
			disk.close_file(file);
			delete []buf;
		}
	}
	
	return 0;
}
