//---------------------------------------------------------------------------
// Purpose:		Parse Gitolite Log files hierarchically to provide statistics
// Author:		Roman Dremov
// Date:		November 2015
// Usage:		glp [-acdhruvw] gitolite.log
//---------------------------------------------------------------------------

// suppress Microsoft deprecated warnings
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <vector>
using namespace std;

//#define GLPVERSION	1		// original code
//#define GLPVERSION	2		// added version option
//#define GLPVERSION	3		// added Option class
#define GLPVERSION		4		// added count, minutes, month, year options

#define MAX_BACK		10		// when searching packet by ID how many to go back
#define INDENT_SPACES	2		// indent spaces per hierarchical level
#define MAX_BUF_SIZE	512		// per line

// reserve vetor size to prevent small allocations
#define INIT_DATA_SIZE	1024
#define INIT_USER_SIZE	32
#define INIT_ADDR_SIZE	INIT_USER_SIZE
#define INIT_CMD_SIZE	2
#define INIT_REPO_SIZE	8

static const char* l_wdays[] = {"Sun", "Mon", "Tus", "Wed", "Thu", "Fri", "Sat"};
static const char* l_months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
enum	ERROR
{
	ERROR_OK = 0,
	ERROR_ARG,
	ERROR_FILE,
};

enum	TYPE
{
	TYPE_NONE = 0,
	TYPE_SSH,
	TYPE_HTTP,
};

inline int		cmp_val(const char* v1, const char* v2)
{
	if( v1 == v2 )
		return 0;
	return strcmp(v1, v2);
}

inline int		cmp_val(int v1, int v2)
{
	if( v1 < v2 )
		return -1;
	else if( v1 > v2 )
		return 1;
	return 0;
}

static bool		str_match(const char* s1, const char* s2)
{
	return 0 == cmp_val(s1, s2);
}

static char*	get_term(char*& str, char delim = '\t')
{
	if( !str )
		return NULL;
	char* term = str;
	while( *str != delim )
	{
		switch( *str )
		{
		case '\n':
			*str = 0;
		case 0:
			str = NULL;
			return term;
		}
		str++;
	}
	while( *str == delim )
	{
		*str = 0;
		str++;
	}
	return term;
}

static char*	get_val(char*& str, const char* arg)
{
	char* strVal = get_term(str);
	char* strName = get_term(strVal, '=');
	if( strName && strVal && str_match(strName, arg) )
		return strVal;
	return NULL;
}

inline tm		LTime(time_t t)
{
	return *localtime(&t);
}

inline void		out_val(const char* val)
{
	if( val )
		printf("%s", val);
}

class	Packet;

typedef int		(Packet::*PFNCmp)(const Packet*) const;
typedef void	(Packet::*PFNOut)() const;

class	Option
{
public:
	Option(PFNCmp pfnCmp, PFNOut pfnOut, Option* pNext)
	{
		m_pfnCmp = pfnCmp;
		m_pfnOut = pfnOut;
		m_pNext = pNext;
	}

	~Option()
	{
		if( m_pNext )
			delete m_pNext;
	}

	int		Compare(const Packet* p1, const Packet* p2) const	{return (p1->*m_pfnCmp)(p2);}
	void	Out(const Packet* p) const							{return (p->*m_pfnOut)();}
	Option*	GetNext() const										{return m_pNext;}

private:
	Option*		m_pNext;
	PFNCmp		m_pfnCmp;
	PFNOut		m_pfnOut;
};

static Option* g_opts = NULL;

class	Packet
{
public:
	Packet()
	{
		memset(this, 0, sizeof(Packet));
	}

	bool	Parse(char* str)
	{
		ParseTime(str);
		ParseTid(str);
		if( !ParseType(str) )
			return false;
		if( m_type > TYPE_NONE )
		{
			ParseUser(str);
			ParseCmd(str);
			ParseAddr(str);
		}
		return true;
	}

	int			CmpAddr(const Packet* p) const		{return cmp_val(m_addr, p->m_addr);}
	void		OutAddr() const						{out_val(m_addr);}

	int			CmpRepo(const Packet* p) const		{return cmp_val(m_repo, p->m_repo);}
	void		OutRepo() const						{out_val(m_repo);}

	int			CmpCmd(const Packet* p) const		{return cmp_val(m_cmd, p->m_cmd);}
	void		OutCmd() const						{out_val(m_cmd);}

	int			CmpUser(const Packet* p) const		{return cmp_val(m_user, p->m_user);}
	void		OutUser() const						{out_val(m_user);}

	int			CmpDelta(const Packet* p) const		{return cmp_val(m_delta,  p->m_delta);}
	void		OutDelta() const					{printf("%d", m_delta);}

	int			CmpDay(const Packet* p) const		{return cmp_val(LTime(m_time).tm_mday, LTime(p->m_time).tm_mday);}
	void		OutDay() const						{printf("%d", LTime(m_time).tm_mday);}

	int			CmpWday(const Packet* p) const		{return cmp_val(LTime(m_time).tm_wday, LTime(p->m_time).tm_wday);}
	void		OutWday() const						{printf("%s", l_wdays[LTime(m_time).tm_wday]);}

	int			CmpHour(const Packet* p) const		{return cmp_val(LTime(m_time).tm_hour, LTime(p->m_time).tm_hour);}
	void		OutHour() const						{printf("%02d", LTime(m_time).tm_hour);}

	int			CmpMin(const Packet* p) const		{return cmp_val(LTime(m_time).tm_min, LTime(p->m_time).tm_min);}
	void		OutMin() const						{printf("%02d", LTime(m_time).tm_min);}

	int			CmpMonth(const Packet* p) const		{return cmp_val(LTime(m_time).tm_mon, LTime(p->m_time).tm_mon);}
	void		OutMonth() const					{printf("%s", l_months[LTime(m_time).tm_mon]);}

	int			CmpYear(const Packet* p) const		{return cmp_val(LTime(m_time).tm_year, LTime(p->m_time).tm_year);}
	void		OutYear() const						{printf("%4d", LTime(m_time).tm_year + 1900);}

protected:
	static int	Compare(const void* p1, const void* p2)
	{
		Option* pOption = g_opts;
		while( pOption )
		{
			int iCmd = pOption->Compare((const Packet*) p1, (const Packet*) p2);
			if( iCmd )
				return iCmd;
			pOption = pOption->GetNext();
		}
		return 0;
	}

	void	ParseTime(char*& str)
	{
		const char* strTime = get_term(str);
		if( !strTime )
			return;
		struct tm tm = {0};
		sscanf(strTime, "%d-%d-%d.%d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
		tm.tm_year -= 1900;
		tm.tm_mon -= 1;
		tm.tm_isdst = -1;
		m_time = mktime(&tm);
	}

	void	ParseTid(char*& str)
	{
		const char* strTid = get_term(str);
		if( strTid )
			m_tid = atoi(strTid);
	}

	bool	ParseType(char*& str)
	{
		const char* strType = get_term(str);
		if( !strType )
			return false;
		if( str_match(strType, "ssh") )
			m_type = TYPE_SSH;
		else if( str_match(strType, "http") )
			m_type = TYPE_HTTP;
		else if( str_match(strType, "END") )
			m_bEnd = true;
		else if( str_match(strType, "update") )
			m_bUpdate = true;
		else if( str_match(strType, "pre_git") )
			m_bPreGit = true;
		else
			return false;
		return true;
	}

	void	ParseUser(char*& str)
	{
		m_user = get_val(str, "ARGV");
	}

	void	ParseCmd(char*& str)
	{
		char* val = get_val(str, "SOC");
		m_cmd = get_term(val, ' ');
		get_term(val, '\'');
		m_repo = get_term(val, '\'');
	}

	void	ParseAddr(char*& str)
	{
		m_addr = get_val(str, "FROM");
	}

private:
	const char*	m_user;
	const char*	m_repo;
	int			m_tid;
	int			m_delta;		// packet duration
	char		m_type;			// TYPE
	bool		m_bEnd;			// packet should have it
	bool		m_bUpdate;		// packet has update field
	bool		m_bPreGit;		// packet has pre_git field
	const char*	m_cmd;
	const char*	m_addr;
	time_t		m_time;
	friend class Data;
};

class	StrMap
{
public:
	StrMap(int nReserve)
	{
		m_v.reserve(nReserve);
	}

	~StrMap()
	{
		for(size_t ii=0; ii<m_v.size(); ii++)
			free(m_v[ii]);
	}

	const char*	Find(const char* str)
	{
		for(size_t ii=0; ii<m_v.size(); ii++)
		{
			if( str_match(str, m_v[ii]) )
				return m_v[ii];
		}
		m_v.push_back(strdup(str));
		return m_v.back();
	}

private:
	vector<char*>	m_v;
};

class	Data
{
public:
	Data()	:	m_users(INIT_USER_SIZE),
				m_addrs(INIT_ADDR_SIZE),
				m_cmds(INIT_CMD_SIZE),
				m_repos(INIT_REPO_SIZE)
	{
		m_packets.reserve(INIT_DATA_SIZE);
	}

	void	Add(const Packet& packet)
	{
		if( packet.m_type > TYPE_NONE )
		{
			m_packets.push_back(packet);
			Packet& packet0 = m_packets.back();
			packet0.m_user = m_users.Find(packet.m_user);
			packet0.m_cmd = m_cmds.Find(packet.m_cmd);
			packet0.m_repo = m_repos.Find(packet.m_repo);
			packet0.m_addr = m_addrs.Find(packet.m_addr);
		}
		else
		{
			int i2 = m_packets.size() - 1;
			int i1 = (i2 > MAX_BACK) ? i2 - MAX_BACK : 0;
			for(int ii=i2; ii>=i1; ii--)
			{
				Packet& packet0 = m_packets[ii];
				if( packet0.m_tid == packet.m_tid )
				{
					int delta = (int)(packet.m_time - packet0.m_time);
					if( packet0.m_delta < delta )
						packet0.m_delta = delta;
					if( packet.m_bEnd )
						packet0.m_bEnd = packet.m_bEnd;
					if( packet.m_bUpdate )
						packet0.m_bUpdate = packet.m_bUpdate;
					if( packet.m_bPreGit )
						packet0.m_bPreGit = packet.m_bPreGit;
					return;
				}
			}
		}
	}

	void	Out(bool bCount)
	{
		if( g_opts )
		{
			qsort(m_packets.data(), m_packets.size(), sizeof(Packet), &Packet::Compare);
			Out(g_opts, 0, m_packets.size(), 0);
		}
		else if( bCount )
		{
			printf("%d\n", m_packets.size());
		}
		else
		{
			for(size_t ii=0; ii<m_packets.size(); ii++)
			{
				const Packet& pa = m_packets[ii];
				printf("%d", ii + 1);
				printf("\t");
				struct tm tm = LTime(pa.m_time);
				printf("%4d-%s-%02d.%02d:%02d:%02d", tm.tm_year + 1900, l_months[tm.tm_mon], tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
				printf("\t");
				printf("%d", pa.m_tid);
				printf("\t");
				out_val(pa.m_user);
				printf("\t");
				out_val(pa.m_addr);
				printf("\t");
				out_val(pa.m_repo);
				printf("\t");
				out_val(pa.m_cmd);
				printf("\n");
			}
		}
	}

protected:
	void	Out(Option* pOption, size_t i0, size_t ic, int spaces)
	{
		for(size_t ii=i0; ii<ic; )
		{
			size_t jj = ii + 1;
			for( ; jj<ic; jj++)
			{
				int iCmd = pOption->Compare(&m_packets[ii], &m_packets[jj]);
				if( iCmd )
					break;
			}
			printf("%6d %*s", jj-ii, spaces, "");
			pOption->Out(&m_packets[ii]);
			printf("\n");
			Option* pNext = pOption->GetNext();
			if( pNext )
				Out(pNext, ii, jj, spaces + INDENT_SPACES);
			ii = jj;
		}
	}

private:
	vector<Packet>	m_packets;
	StrMap			m_users;
	StrMap			m_addrs;
	StrMap			m_cmds;
	StrMap			m_repos;
};

class	Options
{
public:
	Options()
	{
		g_opts = NULL;
	}

	~Options()
	{
		if( g_opts )
			delete g_opts;
	}

	int		Parse(char opt, char* str)
	{
		#define OPT_CASE(_o, _n, _s)	case _o: return Do(str, _s, &Packet::Cmp##_n, &Packet::Out##_n)

		switch( opt )
		{
		OPT_CASE('a', Addr, "IP address");
		OPT_CASE('d', Day, "month day");
		OPT_CASE('e', Delta, "time delta per packet");
		OPT_CASE('h', Hour, "hour");
		OPT_CASE('m', Month, "month");
		OPT_CASE('n', Min, "minute");
		OPT_CASE('r', Repo, "repository name");
		OPT_CASE('u', User, "user name");
		OPT_CASE('w', Wday, "week day");
		OPT_CASE('x', Cmd, "command");
		OPT_CASE('y', Year, "year");
		}
		return 0;
	}

protected:
	int		Do(char* str, const char* desc, PFNCmp pfnCmp, PFNOut pfnOut)
	{
		if( str )
		{
			if( 0 == *str )
			{
				strcpy(str, desc);
				return strlen(str);
			}
		}
		else
			g_opts = new Option(pfnCmp, pfnOut, g_opts);
		return 1;
	}
};

int		main(int argc, char* argv[])
{
	char buf[MAX_BUF_SIZE];
	const char* name = NULL;
	const char* opts = NULL;
	bool bCount = false;

	switch( argc )
	{
	case 2:
		if( str_match(argv[1], "-v") )
		{
			printf("Version 1.%d by Roman Dremov 2015\n", GLPVERSION);
			return 0;
		}
		name = argv[1];
		break;
	case 3:
		if( *argv[1] == '-' )
		{
			if( str_match(argv[1] + 1, "c") )
				bCount = true;
			else
				opts = argv[1] + 1;
			name = argv[2];
		}
		break;
	}

	bool bValid = true;

	if( !name )
		bValid = false;

	Options options;

	if( bValid && opts )
	{
		for(int ii=strlen(opts)-1; ii>=0; ii--)
		{
			if( !options.Parse(opts[ii], NULL) )
			{
				bValid = false;
				break;
			}
		}
	}

	if( !bValid )
	{
		char cc;
		char* str = buf;
		char space[] = " ";
		for(cc='a'; cc<='z'; cc++)
		{
			if( options.Parse(cc, space) )
				*str++ = cc;
		}
		*str = 0;
		printf("USE:\n"
			"\tglp -v\t\t\t\tto get version info\n"
			"\tglp -c gitolite.log\t\tto count entries\n"
			"\tglp gitolite.log\t\tto output all entries\n"
			"\tglp [-%s] gitolite.log\tto output entries hierarchically\n", buf);
		printf("OPTIONS (order defines report sorting hierarchy):\n");
		str = buf;
		*str = 0;
		for(cc='a'; cc<='z'; cc++)
		{
			if( options.Parse(cc, space) )
			{
				sprintf(str, "\t%c - ", cc);
				str += strlen(str);
				str += options.Parse(cc, str);
				strcat(str, "\n");
				str += strlen(str);
			}
		}
		*str = 0;
		printf("%s", buf);
		return ERROR_ARG;
	}

	FILE* pf = fopen(name, "r");

	if( !pf )
	{
		printf("File does not exist: %s\n", name);
		return ERROR_FILE;
	}

	Data data;

	do
	{
		char* str = fgets(buf, sizeof(buf), pf);
		if( str )
		{
			Packet packet;
			if( packet.Parse(str) )
				data.Add(packet);
		}
	} while( !feof(pf) );

	fclose(pf);
	data.Out(bCount);
	return ERROR_OK;
}

