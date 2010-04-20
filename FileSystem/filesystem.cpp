#include "filesystem.h"

char * dirname = "_*_directory_%_";

template <class Type>
Type get_upbound(Type integer_value,Type segment)
{
	Type t = integer_value % segment;
	return t == 0 ? integer_value : integer_value - t + segment;
}

void set_bit(unsigned int pos,char *pbegin)
{
	unsigned int num_byte = (pos >> 3);
	unsigned int offset = pos & 0x7;
	pbegin[num_byte] |= (0x1 << (7 - offset));
}

void unset_bit(unsigned int pos,char *pbegin)
{
	unsigned int num_byte = (pos >> 3);
	unsigned int offset = pos & 0x7;
	pbegin[num_byte] &= (0xff ^ (0x1 << (7 - offset)));
}

bool has_data(unsigned int pos,char *pbegin)
{
	unsigned int num_byte = (pos >> 3);
	unsigned int offset = pos & 0x7;
	return (pbegin[num_byte] & (0x1 << (7 - offset))) > 0;
}

/*--------------------------fileDescriptor-------------------------------------------------------------*/
fileDescriptor::fileDescriptor()
{
	descriptor = NULL;
}

unsigned int fileDescriptor::get_index(unsigned int pos)
{
	if(pos >= INDEXNUM)
	{
		report_error("file_descriptor:get_index's parameter error!");
		return -1;
	}
	return ((unsigned int *)descriptor)[pos];
}

bool fileDescriptor::set_index(unsigned int pos,unsigned int value)
{
	if(pos >= INDEXNUM)
	{
		report_error("file_descriptor:get_index's parameter error!");
		return false;
	}
	((unsigned int *)descriptor)[pos] = value;
	return true;
}
string fileDescriptor::get_name()
{
	if(descriptor == NULL)
	{
		report_error("file_descriptor:descriptor is NULL!");
		return string("");
	}
	return string(descriptor + INDEXARRAYSIZE);
}

bool fileDescriptor::set_name(const char *name)
{
	if(strlen(name) >= FILENAMELENGTH)
	{
		report_error("file_descriptor:set_name's parameter error!");
		return false;
	}
	strcpy_s(descriptor + INDEXARRAYSIZE,FILENAMELENGTH,name);
	return true;
}

long long fileDescriptor::get_file_size()
{
	return *(long long*)(descriptor + INDEXARRAYSIZE + FILENAMELENGTH);
}

void fileDescriptor::set_file_size(long long size)
{
	*(long long*)(descriptor + INDEXARRAYSIZE + FILENAMELENGTH) = size;
}

__time64_t fileDescriptor::get_data()
{
	return *(__time64_t*)(descriptor + INDEXARRAYSIZE + FILENAMELENGTH + FILESIZE);
}

void fileDescriptor::set_data(__time64_t t)
{
	*(__time64_t*)(descriptor + INDEXARRAYSIZE + FILENAMELENGTH + FILESIZE) = t;
}
/*--------------------------fileDescriptor-------------------------------------------------------------*/

/*--------------------------fileOpenTable--------------------------------------------------------------*/
fileOpenTable::fileOpenTable()
{
	this->ptrinbuf = NULL;
	this->pfds = NULL; 
	this->isbigger = false;
	this->isbufchanged = false;
	this->offset = 0;
	this->block_index = 0;
	this->disk = NULL;
}

fileOpenTable::~fileOpenTable()
{
	this->ptrinbuf = NULL;
	this->pfds = NULL; 
	this->disk = NULL;
}

fileOpenTable::fileOpenTable(const fileOpenTable &fo)
{
	this->isbufchanged = fo.isbufchanged;;	
	this->isbigger = fo.isbigger;
	memcpy(this->buf,fo.buf,BLOCKSIZE_KB * KBSIZE);
    this->ptrinbuf = this->buf + (fo.ptrinbuf - fo.buf);
	this->disk = fo.disk;
	this->pfds = fo.pfds;
	this->offset = fo.offset;
	this->block_index = fo.block_index;
	this->blocks = fo.blocks;
}

fileOpenTable &fileOpenTable::operator =(const fileOpenTable &fo)
{
	if(this == &fo)
		return *this;
	this->isbufchanged = fo.isbufchanged;;	
	this->isbigger = fo.isbigger;
	memcpy(this->buf,fo.buf,BLOCKSIZE_KB * KBSIZE);
    this->ptrinbuf = this->buf + (fo.ptrinbuf - fo.buf);
	this->disk = fo.disk;
	this->pfds = fo.pfds;
	this->offset = fo.offset;
	this->block_index = fo.block_index;
	this->blocks = fo.blocks;
	return *this;
}

int fileOpenTable::get_offset_in_buffer()
{
	if(ptrinbuf == NULL)
	{
		report_error("fileOpenTable::get_offset_in_buffer ptrinbuf is NULL!");
		return -1;
	}
	return ptrinbuf - buf;
}

bool fileOpenTable::read_buf(char *mem,unsigned int count)
{
	if(mem == NULL)
	{
		report_error("fileOpenTable::read_buf mem is NULL!");
		return false;
	}
	if(count == 0)
		count = BLOCKSIZE_KB * KBSIZE - get_offset_in_buffer();
	if(count + get_offset_in_buffer() > BLOCKSIZE_KB * KBSIZE)
	{
		report_error("fileOpenTable::read_buf ptrinbuf will overflow!");
		return false;
	}
	if(this->offset + count > this->pfds->get_file_size())
	{
		report_error("fileOpenTable::read_buf ptrinbuf will encounter EOF!");
		return false;
	}
	memcpy(mem,ptrinbuf,count);
	this->ptrinbuf += count;
	this->offset += count;
	return true;
}

bool fileOpenTable::write_buf(char *mem,unsigned int count)
{
	if(mem == NULL)
	{
		report_error("fileOpenTable::write_buf mem is NULL!");
		return false;
	}
	if(count == 0)
		count = BLOCKSIZE_KB * KBSIZE - get_offset_in_buffer();
	if(count + get_offset_in_buffer() > BLOCKSIZE_KB * KBSIZE)
	{
		report_error("fileOpenTable::write_buf ptrinbuf will overflow!");
		return false;
	}
	if(this->offset + count > this->pfds->get_file_size())
	{
		isbigger = true;
	}
	memcpy(ptrinbuf,mem,count);
	this->ptrinbuf += count;
	this->offset += count;
	this->isbufchanged = true;
	return true;
}

bool fileOpenTable::load_next_block_read()
{
	if(block_index  + 1 == blocks.size())
	{
		report_error("fileOpenTable::load_next_block_read encounter the end of the block index array!");
		return false;
	}
	unload_buf();
	block_index++;
	disk->read_block(blocks[block_index],buf);
	return true;
}

bool fileOpenTable::load_next_block_write()
{
	unload_buf();
	if(block_index + 1 == blocks.size())
	{
		unsigned int eblock;
		if(!disk->find_empty_block(&eblock))
		{
			report_error("fileOpenTable::load_next_block_write disk is full!");
			return false;
		}
		blocks.push_back(eblock);
		set_bit(eblock,disk->pdisk);
		block_index++;
	}
	else
	{
		block_index++;
		disk->read_block(blocks[block_index],this->buf);
	}
	return true;
}

bool fileOpenTable::unload_buf()
{
	if(isbufchanged)
	{
		disk->write_block(blocks[block_index],buf);
		if(isbigger)
		{
			pfds->set_file_size(offset);
		}
	}
	this->isbigger = false;
	this->isbufchanged = false;
	this->ptrinbuf = buf;
	return true;
}

bool fileOpenTable::write_back_block_information()
{
	unsigned int index = 0;
	unsigned int eblock;
	unsigned int blocknum = blocks.size();
	
	char fdes_rollback[FILEDESCRIPTORSIZE];
	memcpy(fdes_rollback,this->pfds,FILEDESCRIPTORSIZE);
	vector<unsigned int> setbit_buf;

	for(unsigned int i = 0;index < blocknum && i < ONELEVELINDEX;i++,index++) //level one
	{
		if(pfds->get_index(i) != 0)
			continue;
		pfds->set_index(i,blocks[index]);
	}
	if(index == blocknum)
		return true;

	char bbuf1[BLOCKSIZE_KB * KBSIZE],bbuf2[BLOCKSIZE_KB * KBSIZE];
	
	memset(bbuf1,0,BLOCKSIZE_KB * KBSIZE);
	for(unsigned int i = 0;index < blocknum && i < BLOCKSIZE_KB * KBSIZE / INDEXSIZE;i++,index++) //level two
	{
		((int *)bbuf1)[i] = blocks[index];
	}
	if(pfds->get_index(10) != 0)
	{
		disk->write_block(pfds->get_index(10),bbuf1);
	}
	else
	{
		if(!disk->find_empty_block(&eblock))
		{
			report_error("fileOpenTable::write_back_block_information disk is full!\nRollback!");
			memcpy(this->pfds,fdes_rollback,FILEDESCRIPTORSIZE);
			for(unsigned int z = 0;z < setbit_buf.size();z++)
			{
				unset_bit(setbit_buf[z],disk->pdisk);
			}
			return false;
		}
		setbit_buf.push_back(eblock);
		set_bit(eblock,disk->pdisk);
		pfds->set_index(10,eblock);
		disk->write_block(eblock,bbuf1);
	}
	if(index == blocknum)
		return true;

	if(pfds->get_index(11) == 0)					//level three
	{
		if(!disk->find_empty_block(&eblock))
		{
			report_error("fileOpenTable::write_back_block_information disk if full!\nRollback!");
			memcpy(this->pfds,fdes_rollback,FILEDESCRIPTORSIZE);
			for(unsigned int z = 0;z < setbit_buf.size();z++)
			{
				unset_bit(setbit_buf[z],disk->pdisk);
			}
			return false;
		}
		setbit_buf.push_back(eblock);
		set_bit(eblock,disk->pdisk);
		disk->clear_block(eblock);
		pfds->set_index(11,eblock);
	}
	disk->read_block(pfds->get_index(11),bbuf2);
	for(unsigned int j = 0;index < blocknum && j < BLOCKSIZE_KB * KBSIZE / INDEXSIZE;j++) 
	{
		memset(bbuf1,0,BLOCKSIZE_KB * KBSIZE);
		for(unsigned int i = 0;index < blocknum && i < BLOCKSIZE_KB * KBSIZE / INDEXSIZE;i++,index++) 
		{
			((int *)bbuf1)[i] = blocks[index];
		}
		if(((int *)bbuf2)[j] != 0)
		{
			disk->write_block(((int *)bbuf2)[j],bbuf1);
		}
		else
		{
			if(!disk->find_empty_block(&eblock))
			{
				report_error("fileOpenTable::write_back_block_information disk if full!\nRollback!");
				memcpy(this->pfds,fdes_rollback,FILEDESCRIPTORSIZE);
				for(unsigned int z = 0;z < setbit_buf.size();z++)
				{
					unset_bit(setbit_buf[z],disk->pdisk);
				}
				return false;
			}
			setbit_buf.push_back(eblock);
			set_bit(eblock,disk->pdisk);
			((int *)bbuf2)[j] = eblock;
			disk->write_block(((int *)bbuf2)[j],bbuf1);
		}
	}
	disk->write_block(pfds->get_index(11),bbuf2);
	if(index == blocknum)
	{
		return true;
	}
	else
	{
		report_error("fileOpenTable::write_back_block_information file > 4G?");
		return false;
	}
}
/*--------------------------fileOpenTable--------------------------------------------------------------*/

/*--------------------------Disk-----------------------------------------------------------------------*/
Disk::Disk()
{
	pdisk = NULL;
	pblocks = NULL;
	pdirectory = NULL;
	fdes = NULL;
	disksize_KB = 0;
	blocknum = 0;
}

Disk::~Disk()
{
	for(map<unsigned int,fileOpenTable>::iterator p = fopt.begin();p != fopt.end();p++)
	{
		this->close_file(p->first);
		p = fopt.begin();
		if(fopt.empty())
			break;
	}
	fdes = NULL;
	pdirectory = NULL;
	pblocks = NULL;
	if(pdisk)
		delete []pdisk;
}

Disk::Disk(const Disk &disk)
{
	if(disk.disksize_KB == 0)
	{
		this->name = disk.name;
		this->blocknum = 0;
		this->disksize_KB = 0;
		this->fopt = disk.fopt;
		pdisk = NULL;
		pblocks = NULL;
		pdirectory = NULL;
		fdes = NULL;
	}
	else
	{
		this->disksize_KB = disk.disksize_KB;
		this->name = disk.name;
		this->pdisk = new char [this->disksize_KB * KBSIZE];
		if(!pdisk)
		{
			report_error("Disk::Disk_CopyConstructor memory error!");
			return;
		}
		memcpy(this->pdisk,disk.pdisk,this->disksize_KB * KBSIZE);
		init_disk_management_facility();
		this->fopt = disk.fopt;
		map<unsigned int,fileOpenTable>::iterator p = this->fopt.begin();
		for(;p != fopt.end();p++)
		{
			p->second.disk = this;
			p->second.pfds = &(this->fdes[p->first]);
		}
	}
}

Disk &Disk::operator =(const Disk &disk)
{
	if(this == &disk)
		return *this;
	if(disk.disksize_KB == 0)
	{
		this->name = disk.name;
		this->blocknum = 0;
		this->disksize_KB = 0;
		this->fopt = disk.fopt;
		pdisk = NULL;
		pblocks = NULL;
		pdirectory = NULL;
		fdes = NULL;
	}
	else
	{
		this->disksize_KB = disk.disksize_KB;
		this->name = disk.name;
		this->pdisk = new char [this->disksize_KB * KBSIZE];
		if(!pdisk)
		{
			report_error("Disk::Disk_CopyConstructor memory error!");
			this->name = disk.name;
			this->blocknum = 0;
			this->disksize_KB = 0;
			this->fopt = disk.fopt;
			pdisk = NULL;
			pblocks = NULL;
			pdirectory = NULL;
			fdes = NULL;
			return *this;
		}
		memcpy(this->pdisk,disk.pdisk,this->disksize_KB * KBSIZE);
		init_disk_management_facility();
		this->fopt = disk.fopt;
		map<unsigned int,fileOpenTable>::iterator p = this->fopt.begin();
		for(;p != fopt.end();p++)
		{
			p->second.disk = this;
			p->second.pfds = &(this->fdes[p->first]);
		}
	}
	return *this;
}

bool Disk::clear_block(unsigned int block)
{
	if(block >= this->blocknum)
	{
		report_error("Disk::clear_block block is too large!");
		return false;
	}
	memset(this->pblocks + block * BLOCKSIZE_KB * KBSIZE,0,BLOCKSIZE_KB * KBSIZE);
	return true;
}

bool Disk::find_empty_block(unsigned int *block)
{
	static unsigned int begin = 0;
	for(unsigned int i = 0;i < this->blocknum;i++)
	{
		if(!has_data((begin + i) % this->blocknum,pdisk))
		{
			if(block)
				*block = (begin + i) % this->blocknum;
			begin = (begin + i + 1) % this->blocknum;
			return true;
		}
	}
	return false;
}

void Disk::init_disk_management_facility(bool isformat)
{
	unsigned int bnum,maxblocknum,head_byte,head_block,bitmap_byte;
	bnum = maxblocknum = this->disksize_KB / BLOCKSIZE_KB;
	do
	{
		bnum--;
		bitmap_byte = get_upbound(bnum,(unsigned int)8) / 8;
		head_byte = 2 * bitmap_byte + FILEDESCRIPTORSIZE * bnum; //bitmap + directroy = 2 * bitmap
		head_block = get_upbound(head_byte,(unsigned int)BLOCKSIZE_KB * KBSIZE) / (BLOCKSIZE_KB * KBSIZE);
	}
	while(bnum + head_block > maxblocknum);

	this->blocknum = bnum;
	this->fdes = new fileDescriptor[bnum];
	this->pdirectory = this->pdisk + bitmap_byte;
	for(unsigned int i = 0;i < bnum;i++)
	{
		this->fdes[i].descriptor = this->pdisk + 2 * bitmap_byte + i * FILEDESCRIPTORSIZE;
	}
	this->pblocks = this->pdisk + head_block * BLOCKSIZE_KB * KBSIZE;

	if(isformat)
	{
		memset(this->pdisk,0,head_block * BLOCKSIZE_KB * KBSIZE);
	}
}

void Disk::format()
{
	init_disk_management_facility(true);
}

bool Disk::save_to_file()
{
	ofstream out;
	out.open((this->name + ".dsk").c_str(),ios::binary);
	if(out.fail())
	{
		report_error("Disk::save_to_file fail!");
		return false;
	}
	out.write(this->pdisk,this->disksize_KB * KBSIZE);
	out.close();
	return true;
}

bool Disk::init_from_file()
{
	struct _stat info;
	_stat((this->name + ".dsk").c_str(), &info);
	unsigned int size = info.st_size;

	ifstream in;
	in.open((this->name + ".dsk").c_str(),ios::binary);
	if(in.fail())
	{
		report_error("Disk::init_from_file cann't open file!");
		return false;
	}
	this->pdisk = new char[size];
	if(this->pdisk == NULL)
	{
		report_error("Disk::init_from_file memory error!");
		return false;
	}
	in.read(this->pdisk,size);
	in.close();

	this->disksize_KB = size / KBSIZE;
	init_disk_management_facility();
	return true;
}

void Disk::write_block(unsigned int i,char *p,unsigned int num)
{
	if(i >= this->blocknum || num > BLOCKSIZE_KB * KBSIZE)
	{
		::report_error("Disk::write_block parameter error!");
		return;
	}
	memcpy(this->pblocks + i * BLOCKSIZE_KB * KBSIZE,p,num);
}

void Disk::read_block(unsigned int i,char *p,unsigned int num)
{
	if(i >= this->blocknum || num > BLOCKSIZE_KB * KBSIZE)
	{
		::report_error("Disk::read_block parameter error!");
		return;
	}
	memcpy(p,this->pblocks + i * BLOCKSIZE_KB * KBSIZE,num);
}

bool Disk::create_file(const char *filename)
{
	if(strlen(filename) >= FILENAMELENGTH)
	{
		::report_error("Disk::create_file filename must less than 256 character!");
		return false;
	}
	unsigned int empty = 0;							//find empty fdes to create file
	for(empty = 0;empty < this->blocknum;empty++)
	{
		if(this->fdes[empty].get_name().empty())
		{
			break;
		}
	}
	if(empty == this->blocknum)
	{
		report_error("Disk::create_file cann't create file because of file descriptor!");
		return false;
	}
	if(find_file_in_directory(filename,NULL))
	{
		report_error((string("Disk::create_file ") + filename + "has exist!").c_str());
		return false;
	}
	unsigned int eblock;
	if(!find_empty_block(&eblock))
	{
		report_error("Disk::create_file disk is full!");
		return false;
	}

	this->fdes[empty].set_name(filename);
	this->fdes[empty].set_file_size(0);
	this->fdes[empty].set_data(_time64(NULL));
	this->fdes[empty].set_index(0,eblock);
	set_bit(eblock,this->pdisk);
	set_bit(empty,this->pdirectory);	//add entry in directory
	return true;
}

bool Disk::write_file(unsigned int file,char *mem,unsigned int count)
{
	if(mem == NULL)
	{
		report_error("Disk::write_file mem == NULL!");
		return false;
	}
	if(!has_open(file))
	{
		report_error("Disk::write_file read a unopen file!");
		return false;
	}
	map<unsigned int,fileOpenTable>::iterator p = fopt.find(file);
	if(p == fopt.end())
	{
		report_error("Disk::write_file file is not illeagal!");
		return false;
	}
	
	/*-----------rollback information----------------*/
	bool isbufchanged = p->second.isbufchanged;	
	bool isbigger = p->second.isbigger;
	long long offset = p->second.offset;
	long long filesize = p->second.pfds->get_file_size();
	unsigned int block_throw = 0;
	bool rollback = false;
	/*-----------rollback information----------------*/

	for(unsigned int writelen = 0;writelen < count;)
	{
		int bufoff = p->second.get_offset_in_buffer();
		if(bufoff == -1)
		{
			report_error("Disk::write_file bufoff == -1!");
			return false;
		}
		if(bufoff + count - writelen < BLOCKSIZE_KB * KBSIZE)
		{
			if(!p->second.write_buf(mem + writelen,count - writelen))
			{
				report_error("rollback!");
				rollback = true;
				break;
			}
			writelen += count;
		}
		else
		{
			if(!p->second.write_buf(mem + writelen))
			{
				rollback = true;
				break;
			}
			writelen += BLOCKSIZE_KB * KBSIZE - bufoff;
			if(!p->second.load_next_block_write())
			{
				report_error("rollback!");
				rollback = true;
				break;
			}
			block_throw++;
		}
	}
	if(rollback)
	{
		for(unsigned int i = 0;i < block_throw;i++)
		{
			unset_bit(*(p->second.blocks.rbegin()),p->second.disk->pdisk);
			p->second.blocks.pop_back();
		}
		unsigned int block = (unsigned int)(offset / (BLOCKSIZE_KB * KBSIZE));
		unsigned int bufoff = offset % (BLOCKSIZE_KB * KBSIZE);
		read_block(p->second.blocks[block],p->second.buf);
		p->second.block_index = block;
		p->second.ptrinbuf = p->second.buf + bufoff;
		p->second.isbufchanged = isbufchanged;
		p->second.isbigger = isbigger;
		p->second.offset = offset;
		p->second.pfds->set_file_size(filesize);
	}
	return true;
}

bool Disk::lseek_file(unsigned int file,long long offset)
{
	if(offset < 0)
	{
		report_error("Disk::lseek_file offset < 0!");
		return false;
	}
	if(!has_open(file))
	{
		report_error("Disk::lseek_file file is not opened!");
		return false;
	}
	
	unsigned int block;
	unsigned int bufoff;
	block = (unsigned int)(offset / (BLOCKSIZE_KB * KBSIZE));
	bufoff = offset %  (BLOCKSIZE_KB * KBSIZE);
	map<unsigned int,fileOpenTable>::iterator p = fopt.find(file);
	if(p == fopt.end())
	{
		report_error("Disk::lseek_file cann't find file in fopt!");
		return false;
	}
	long long filesize = max(p->second.offset,p->second.pfds->get_file_size());
	if(offset > filesize)
	{
		report_error("Disk::lseek_file offset > filesize!");
		return false;
	}
	p->second.unload_buf();
	if(block != p->second.block_index)
	{
		read_block(p->second.blocks[block],p->second.buf);
		p->second.block_index = block;
	}
	p->second.offset = offset;
	p->second.ptrinbuf = p->second.buf + bufoff;
	return true;
}

bool Disk::has_open(unsigned int file)
{
	return fopt.find(file) != fopt.end();
}

bool Disk::open_file(const char *filename,unsigned int *file)
{
	if(!find_file_in_directory(filename,file))
	{
		report_error((string("Disk::open_file cann't find ") + filename + " in directory!").c_str());
		return false;
	}
	if(has_open(*file))
	{
		report_error((string("Disk::open_file ") + filename + " has been opened!").c_str());
		return false;
	}
	init_fileOpenTable(*file);
	return true; 
}

bool Disk::read_file(unsigned int file,char *mem,unsigned int count)
{
	if(mem == NULL)
	{
		report_error("Disk::read_file mem == NULL!");
		return false;
	}
	if(!has_open(file))
	{
		report_error("Disk::read_file read a unopen file!");
		return false;
	}
	for(unsigned int readlen = 0;readlen < count;)
	{
		int bufoff = fopt[file].get_offset_in_buffer();
		if(bufoff == -1)
		{
			report_error("Disk::read_file bufoff == -1!");
			return false;
		}
		if(bufoff + count - readlen <= BLOCKSIZE_KB * KBSIZE)
		{
			if(!fopt[file].read_buf(mem + readlen,count - readlen))
				return false;
			readlen += count;
		}
		else if(bufoff + count > BLOCKSIZE_KB * KBSIZE)
		{
			if(!fopt[file].read_buf(mem + readlen))
				return false;
			readlen += BLOCKSIZE_KB * KBSIZE - bufoff;
			if(!fopt[file].load_next_block_read())
			{
				return false;
			}
		}
	}
	return true;
}

bool Disk::close_file(unsigned int file)
{
	map<unsigned int,fileOpenTable>::iterator p;
	p = this->fopt.find(file);
	if(p == fopt.end())
	{
		report_error("Disk::close_file cann't find file in fopt!");
		return false;
	}
	p->second.unload_buf();
	p->second.write_back_block_information();
	fopt.erase(p);
	
	return true;
}

bool Disk::find_file_in_directory(const char *filename,unsigned int *fds)
{
	for(unsigned int i = 0;i < this->blocknum;i++)
	{
		if(has_data(i,this->pdirectory))
		{
			string s1 = filename;
			string s2 = this->fdes[i].get_name();
			for(string::size_type t = 0;t < s1.length();t++)
			{
				s1[t] = tolower(s1[t]);
			}
			for(string::size_type t = 0;t < s2.length();t++)
			{
				s2[t] = tolower(s2[t]);
			}
			if(s1 == s2)
			{
				if(fds)
				{
					*fds = i;
				}
				return true;
			}
		}
	}
	return false;
}

bool Disk::init_fileOpenTable(unsigned int fds)
{
	fopt[fds] = fileOpenTable();
	map<unsigned int,fileOpenTable>::iterator p = fopt.find(fds);
	p->second.pfds = &fdes[fds];
	p->second.disk = this;
	p->second.ptrinbuf = p->second.buf;
	
	
	read_block(p->second.pfds->get_index(0),p->second.buf);
	
	long long size = p->second.pfds->get_file_size();
	if(size == 0)
	{
		p->second.blocks.push_back(p->second.pfds->get_index(0));
		return true;
	}
	long long s = 0;
	bool finish = false;

	for(int i = 0;i < ONELEVELINDEX;i++)		//level one
	{
		p->second.blocks.push_back(p->second.pfds->get_index(i));
		s += BLOCKSIZE_KB * KBSIZE;
		if(s >= size)
			return true;
	}
	
	char bbuf1[BLOCKSIZE_KB * KBSIZE],bbuf2[BLOCKSIZE_KB * KBSIZE];		//level two
	read_block(p->second.pfds->get_index(10),bbuf1);
	for(int i = 0;i < BLOCKSIZE_KB * KBSIZE / INDEXSIZE;i++)
	{
		p->second.blocks.push_back(((int *)bbuf1)[i]);
		s += BLOCKSIZE_KB * KBSIZE;
		if(s >= size)
			return true;
	}

	read_block(p->second.pfds->get_index(11),bbuf1);		//level three
	for(int i = 0;i < BLOCKSIZE_KB * KBSIZE / INDEXSIZE;i++)
	{
		read_block(((int *)bbuf1)[i],bbuf2);
		for(int j = 0;j < BLOCKSIZE_KB * KBSIZE / INDEXSIZE;j++)
		{
			p->second.blocks.push_back(((int *)bbuf2)[j]);
			s += BLOCKSIZE_KB * KBSIZE;
			if(s >= size)
				return true;
		}
	}

	report_error("Disk::init_fileOpenTable file is bigger than 4G?");
	return false;

}

long long Disk::get_file_size(unsigned int file)
{
	if(!has_open(file))
	{
		report_error("Disk::get_file_size file is not been opened!");
		return 0;
	}
	return this->fdes[file].get_file_size();
}

bool Disk::destroy_file(char *filename)
{
	vector<unsigned int> inf;
	unsigned int file = 0;

	if(!find_file_in_directory(filename,&file))
	{
		report_error((string("Disk::destroy_file ") + filename + "cann't find in directory!").c_str());
		return false;
	}
	if(has_open(file))
	{
		report_error((string("Disk::destroy_file ") + filename + "has been opened!").c_str());
		return false;
	}
	if(!open_file(filename,&file))
	{
		report_error((string("Disk::destroy_file cann't create ") + filename + "'s block information array!").c_str());
		return false;
	}

	inf = fopt[file].blocks;

	if(!close_file(file))
	{
		report_error((string("Disk::destroy_file cann't create ") + filename + "'s block information array!").c_str());
		return false;
	}

	unsigned int len = inf.size();
	for(unsigned int i = 0;i < len;i++)
	{
		unset_bit(inf[i],pdisk);
	}

	char indexbuf[BLOCKSIZE_KB * KBSIZE];			//level 3
	if(fdes[file].get_index(11) != 0)
	{
		read_block(fdes[file].get_index(11),indexbuf);
		for(unsigned int i = 0;i < (BLOCKSIZE_KB * KBSIZE / INDEXSIZE);i++)
		{
			if(((int *)indexbuf)[i] == 0)
				break;
			unset_bit(((int *)indexbuf)[i],pdisk);
		}
		unset_bit(fdes[file].get_index(11),pdisk);
	}
	

	if(fdes[file].get_index(10) != 0)		//level 2
	{
		unset_bit(fdes[file].get_index(10),pdisk);
	}

	memset(fdes[file].descriptor,0,sizeof(FILEDESCRIPTORSIZE));
	
	unset_bit(file,this->pdirectory);	//delete entry in directory
	return true;
}

void Disk::directory()
{
	for(unsigned int i = 0;i < this->blocknum;i++)
	{
		if(has_data(i,this->pdirectory))
		{
			cout<<this->fdes[i].get_name()<<endl;
		}
	}
}
/*--------------------------Disk-----------------------------------------------------------------------*/

/*--------------------------miniFileSystem-------------------------------------------------------------*/
miniFileSystem::miniFileSystem()
{
	curr = -1;
}

bool miniFileSystem::create_disk(char *diskname,unsigned int disksize_KB)
{
	this->dskmounted.push_back(Disk());
	vector<Disk>::reverse_iterator disk = this->dskmounted.rbegin();
	disk->name = diskname;
	disk->disksize_KB = disksize_KB;
	if(disk->disksize_KB < 8)
	{
		report_error("miniFileSystem::create_disk disk size too small!");
		dskmounted.pop_back();
		return false;
	}

	disk->pdisk = new char[disksize_KB * KBSIZE];
	if(disk->pdisk == NULL)
	{
		report_error("miniFileSystem::create_disk memory error!");
		dskmounted.pop_back();
		return false;
	}
	disk->format();
	
	this->dskname.push_back(disk->name);
	this->curr = dskmounted.size() - 1;
	return true;
}


/*--------------------------miniFileSystem-------------------------------------------------------------*/