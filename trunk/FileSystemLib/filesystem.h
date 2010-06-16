#pragma once
#include <fstream>
#include <vector>
#include <cstring>
#include <ctime>
#include <string>
#include <map>
#include <sys/stat.h>
#include <algorithm>
using namespace std;

#define BLOCKSIZE_KB 4		//块大小
#define KBSIZE	0x400		//1KB大小
#define INDEXLEVEL 3		//索引级数
#define ONELEVELINDEX 10	//一级索引数目
#define INDEXNUM	(ONELEVELINDEX + INDEXLEVEL - 1)	//文件描述符中索引项数
#define INDEXSIZE (sizeof(unsigned int))		//每个索引项字节数
#define INDEXARRAYSIZE (INDEXNUM * INDEXSIZE)	//文件描述符中索引需要的空间(字节)
#define FILENAMELENGTH 256		//最长文件名长度
#define FILESIZE	(sizeof(long long))		//文件大小描述符所占字节数
#define DATASIZE	(sizeof(__time64_t))	//文件创建日期描述符所占字节数
#define FILEDESCRIPTORSIZE  (INDEXARRAYSIZE + FILENAMELENGTH + FILESIZE + DATASIZE) //文件描述符所占字节数
#define DIRECTORYPOS 0	//目录项所在位置

void set_bit(unsigned int pos, char *pbegin);	//设置pbegin + pos(bit) 处为 1
void unset_bit(unsigned int pos, char *pbegin);	//设置pbegin + pos(bit) 处为 0
bool has_data(unsigned int pos, char *pbegin);	//判断pbegin + pos(bit) 处是否为 1
template <class Type>
Type get_upbound(Type integer_value, Type segment);
enum COLOR{EMPTY = 0,GERY,GREEN,RED,YELLOW};

struct fileDescriptor		//文件描述符
{
    char *descriptor;

    fileDescriptor();

    unsigned int get_index(unsigned int pos);
    bool set_index(unsigned int pos, unsigned int value);

    string get_name();
    bool set_name(const char *name);

    long long get_file_size();
    void set_file_size(long long size);

    __time64_t get_data();
    void set_data(__time64_t t);
};

struct Disk;
struct fileOpenTable;

struct fileOpenTable
{
    bool isbufchanged;
    bool isbigger;

    char buf[BLOCKSIZE_KB * KBSIZE];
    char *ptrinbuf;
    Disk *disk;
    fileDescriptor *pfds;
    long long offset;
    unsigned int block_index;
    vector<unsigned int> blocks;

    fileOpenTable();
    ~fileOpenTable();
    fileOpenTable(const fileOpenTable &fo);
    fileOpenTable &operator =(const fileOpenTable &fo);

    int get_offset_in_buffer();
    bool read_buf(char *mem, unsigned int count = -1); //count == -1 mean read all data after ptrinbuf
    bool write_buf(char *mem, unsigned int count = -1);
    bool load_next_block_read();
    bool load_next_block_write();
    bool unload_buf();
    bool write_back_block_information();
};

struct Disk
{
    string name;
    char *pdisk;
    char *pblocks;
    char *pdirectory;
    fileDescriptor *fdes;
    map<unsigned int, fileOpenTable> fopt;
    unsigned int blocknum;
    unsigned int disksize_KB;

    Disk();
    ~Disk();
    Disk(const Disk &disk);
    Disk &operator =(const Disk &disk);

    bool has_open(unsigned int file);
    bool find_empty_block(unsigned int *block);
    void init_disk_management_facility(bool isformat = false);
    bool init_fileOpenTable(unsigned int fds);
    bool clear_block(unsigned int block);
    bool find_file_in_directory(const char *filename, unsigned int *fds);
    void read_block(unsigned int i, char *p, unsigned int num = BLOCKSIZE_KB * KBSIZE);
    void write_block(unsigned int i, char *p, unsigned int num = BLOCKSIZE_KB * KBSIZE);

    //------------------------------------API-----------------------------------------//
    void format();
    bool init_from_file(const char *pPath = NULL);
    bool save_to_file(const char *pPath = NULL);
    bool open_file(const char *filename, unsigned int *file);
    bool read_file(unsigned int file, char *mem, unsigned int count);
    bool close_file(unsigned int file);
    bool create_file(const char *filename);
    bool lseek_file(unsigned int file, long long offset);
    bool write_file(unsigned int file, char *mem, unsigned int count);
    bool destroy_file(const char *filename);
    vector<string> directory();
    long long get_file_size(unsigned int file);
	//------------------------------------API-----------------------------------------//
};

class CFSShellDlg;
class miniFileSystem
{
public:
    unsigned int curr;
    vector<string> dskname;
    vector<Disk> dskmounted;
public:
    miniFileSystem();

    /*---------API----------------------------------------------------*/
    bool create_disk(char *diskname, unsigned int disksize_KB);
	void set_error_func(void (*func)(const char *s,CFSShellDlg *dlg),CFSShellDlg *dlg);
	bool add_file(const char *path);
	bool save_file(const char *dir_path,const char *fname);
	bool delete_file(const char *fname);
	vector<COLOR> get_bit_map();
	vector<COLOR> get_bit_map(const char *fname);
    /*----------------------------------------------------------------*/
};


