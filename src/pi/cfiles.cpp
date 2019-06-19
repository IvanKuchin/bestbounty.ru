#include "cfiles.h"

CFile::CFile()
{
	content = NULL;
	length = 0;
	name = "";
}

string CFile::GetName()
{
	return name;
}
		
int CFile::GetSize()
{
	return length;
}

char* CFile::GetContent()
{
	return content;
}

void CFile::SetName(string n)
{
	name = n;
}

void CFile::SetSize(int s)
{
	length = s;
}

void CFile::SetContent(char *c)
{
	content = (char *)malloc(GetSize() + 1);
	if(content == NULL)
	{
		CLog	log;
		log.Write(ERROR, "void CFile::SetContent:ERROR: memory allocation. Aborting...");
		
		throw CException("error memory allocation");
	}
	memset(content, 0, GetSize() + 1);
	memcpy(content, c, GetSize());
}

void CFile::Set(string n, char* c, int s)
{
	SetName(n);
	SetSize(s);
	SetContent(c);
}

void CFile::Delete()
{
	if(content != NULL)
	{
		free(content);
		name = "";
		length = 0;
	
		{
			CLog	log;
			
			log.Write(DEBUG, "CFile::" + string(__func__) + "[" + to_string(__LINE__) + "]" + ": delete files from memory");
		}
	
	}
}

CFile::~CFile()
{
	Delete();
}

CFiles::CFiles()
{
}

bool CFiles::Add(string name, char *content, int size)
{
	CFile	*f;
	
	f = new(CFile);
	if(!f)
	{
		CLog	log;

		log.Write(ERROR, "CFiles::" + string(__func__) + "[" + to_string(__LINE__) + "]" + ":ERROR: error allocating memory (file module)");
		throw CException("error allocating memory (file module)");
		
		return false;
	}

	files.push_back(f);
	f->Set(name, content, size);

	return true;
}

char* CFiles::Get(int i)
{
	vector<CFile *>::iterator	iv;
	int				j;
	
	for(iv = files.begin(),j = 0; iv < files.end(); iv++, j++)
	{
		if(i == j)
		{
			return (*iv)->GetContent();
		}
	}
	return NULL;
}

string CFiles::GetName(int i)
{
	vector<CFile *>::iterator	iv;
	int				j;
	
	for(iv = files.begin(), j = 0; iv < files.end(); iv++, j++)
	{
		if(i == j)
		{
			return (*iv)->GetName();
		}
	}
	return "";
}

char* CFiles::Get(string name)
{
	vector<CFile *>::iterator	iv;
	
	for(iv = files.begin(); iv < files.end(); iv++)
	{
		if((*iv)->GetName() == name)
		{
			return (*iv)->GetContent();
		}
	}
	return NULL;
}

int CFiles::GetSize(string name)
{
	vector<CFile *>::iterator	iv;
	
	for(iv = files.begin(); iv < files.end(); iv++)
	{
		if((*iv)->GetName() == name)
		{
			return (*iv)->GetSize();
		}
	}
	return -1;
}

int CFiles::GetSize(int i)
{
	vector<CFile *>::iterator	iv;
	int				j;
	
	for(iv = files.begin(),j = 0; iv < files.end(); iv++, j++)
	{
		if(i == j)
		{
			return (*iv)->GetSize();
		}
	}
	return -1;
}

void CFiles::Delete(string name)
{
	vector<CFile *>::iterator	iv;
	
	for(iv = files.begin(); iv < files.end(); iv++)
	{
		if((*iv)->GetName() == name)
		{
			(*iv)->Delete();
		}
	}
}

int CFiles::Count()
{
	return files.size();
}

CFiles::~CFiles()
{
	vector<CFile *>::iterator	iv;

	for(iv = files.begin(); iv < files.end(); iv++)
	{
		(*iv)->Delete();
	}
}

