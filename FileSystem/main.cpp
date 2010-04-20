#include <iostream>
#include "filesystem.h"
#include "windows.h"
using namespace std;
#define READ
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

	string s = "D:\\SPGBackup.rar";
	struct _stat info;
	_stat(s.c_str(), &info);
	unsigned int size = info.st_size;
	cout<<size<<endl;
#ifdef READ
	unsigned int t = ::GetTickCount();
	
	ifstream in(s.c_str(),ios::binary);
	char *buf = new char[size];
	in.read(buf,size);
	disk.create_file("A.rar");
	disk.create_file("B.rar");
	unsigned int filea[2];
	disk.open_file("A.rar",&filea[0]);
	disk.open_file("B.rar",&filea[1]);
	/*
	for(int j = 0;j < 2;j++)
	{
		for(int i = 0;i < size;)
		{
			if(size - i > 714)
			{
				disk.write_file(filea[j],buf + i, 714);
				i += 714;
			}
			else
			{
				disk.write_file(filea[j],buf + i,size - i);
				break;
			}
			unsigned int c = (rand() % i);
			disk.lseek_file(filea[j],i - c);
			disk.lseek_file(filea[j],i);
		}
		disk.close_file(filea[j]);
	}
	*/
	disk.write_file(filea[0],buf,size);
	disk.write_file(filea[1],buf,size);
	disk.close_file(filea[1]);
	disk.close_file(filea[0]);
	delete []buf;
	in.close();
	cout<<::GetTickCount() - t << endl;
	for(int i = 0;i < disk.blocknum;i++)
	{
		if(has_data(i,disk.pdisk) && i != 0 && i != 1)
			cout<<"ERROR! "<<i<<endl;
	}
	for(int i = 0;i < 10;i++)
	{
		cout<<((int *)disk.fdes[filea[0]].descriptor)[i]<<endl;
	}
	disk.directory();
	disk.destroy_file("A.rar");
	disk.destroy_file("B.rar");
	disk.directory();
	for(int i = 0;i < disk.blocknum;i++)
	{
		if(has_data(i,disk.pdisk) && i != 0 && i != 1)
			cout<<"ERROR! "<<i<<endl;
	}
	
	//disk.save_to_file();
#else
	char *buf = new char[size];
	unsigned int file[2];
	disk.init_from_file();
	disk.open_file("A.rar",&file[0]);
	disk.open_file("B.rar",&file[1]);
	for(int j = 0;j < 2;j++)
	{
		disk.read_file(file[j],buf,size);
		char name[10];
		sprintf(name,"D:\\%c.rar",'A'+ j);
		ofstream out(name,ios::binary);
		out.write(buf,size);
		out.close();
		disk.close_file(file[j]);
	}

#endif
	return 0;
}
