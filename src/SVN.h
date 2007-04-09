#pragma once
#include <vector>
#include <map>

#include "apr_general.h"
#include "svn_pools.h"
#include "svn_client.h"
#include "svn_path.h"
#include "svn_wc.h"
#include "svn_utf.h"
#include "svn_config.h"
#include "svn_error_codes.h"
#include "svn_subst.h"
#include "svn_repos.h"
#include "svn_time.h"

#include "SVNPool.h"
#include "UnicodeUtils.h"
#include "Registry.h"
#include "SerializeUtils.h"

typedef std::wstring wide_string;
#ifndef stdstring
#	ifdef UNICODE
#		define stdstring wide_string
#	else
#		define stdstring std::string
#	endif
#endif

#include <string>

using namespace std;

class SVNInfoData
{
public:
	SVNInfoData(){}

	stdstring			url;
	svn_revnum_t		rev;
	svn_node_kind_t		kind;
	stdstring			reposRoot;
	stdstring			reposUUID;
	svn_revnum_t		lastchangedrev;
	__time64_t			lastchangedtime;
	stdstring			author;

	stdstring			lock_path;
	stdstring			lock_token;
	stdstring			lock_owner;
	stdstring			lock_comment;
	bool				lock_davcomment;
	__time64_t			lock_createtime;
	__time64_t			lock_expirationtime;

	bool				hasWCInfo;
	svn_wc_schedule_t	schedule;
	stdstring			copyfromurl;
	svn_revnum_t		copyfromrev;
	__time64_t			texttime;
	__time64_t			proptime;
	stdstring			checksum;
	stdstring			conflict_old;
	stdstring			conflict_new;
	stdstring			conflict_wrk;
	stdstring			prejfile;
};

class SVNLogChangedPaths
{
public:
	SVNLogChangedPaths()
		: action(0)
	{

	}

	wchar_t				action;
	svn_revnum_t		copyfrom_revision;
	stdstring			copyfrom_path;

	bool Save(HANDLE hFile) const
	{
		if (!CSerializeUtils::SaveNumber(hFile, action))
			return false;
		if (!CSerializeUtils::SaveNumber(hFile, copyfrom_revision))
			return false;
		if (!CSerializeUtils::SaveString(hFile, copyfrom_path))
			return false;
		return true;
	}
	bool Load(HANDLE hFile)
	{
		unsigned __int64 value;
		if (!CSerializeUtils::LoadNumber(hFile, value))
			return false;
		action = (wchar_t)value;
		if (!CSerializeUtils::LoadNumber(hFile, value))
			return false;
		copyfrom_revision = (svn_revnum_t)value;
		if (!CSerializeUtils::LoadString(hFile, copyfrom_path))
			return false;
		return true;
	}
};

class SVNLogEntry
{
public:
	SVNLogEntry() 
		: read(false)
		, revision(0)
		, date(0)
	{

	}

	bool				read;
	svn_revnum_t		revision;
	stdstring			author;
	apr_time_t			date;
	stdstring			message;
	map<stdstring, SVNLogChangedPaths>	m_changedPaths;

	bool Save(HANDLE hFile) const
	{
		if (!CSerializeUtils::SaveNumber(hFile, read))
			return false;
		if (!CSerializeUtils::SaveNumber(hFile, revision))
			return false;
		if (!CSerializeUtils::SaveString(hFile, author))
			return false;
		if (!CSerializeUtils::SaveNumber(hFile, date))
			return false;
		if (!CSerializeUtils::SaveString(hFile, message))
			return false;

		if (!CSerializeUtils::SaveNumber(hFile, CSerializeUtils::SerializeType_Map))
			return false;
		if (!CSerializeUtils::SaveNumber(hFile, m_changedPaths.size()))
			return false;
		for (map<stdstring,SVNLogChangedPaths>::const_iterator it = m_changedPaths.begin(); it != m_changedPaths.end(); ++it)
		{
			if (!CSerializeUtils::SaveString(hFile, it->first))
				return false;
			if (!it->second.Save(hFile))
				return false;
		}
		return true;
	}
	bool Load(HANDLE hFile)
	{
		unsigned __int64 value = 0;
		if (!CSerializeUtils::LoadNumber(hFile, value))
			return false;
		read = !!value;
		if (!CSerializeUtils::LoadNumber(hFile, value))
			return false;
		revision = (svn_revnum_t)value;
		if (!CSerializeUtils::LoadString(hFile, author))
			return false;
		if (!CSerializeUtils::LoadNumber(hFile, value))
			return false;
		date = value;
		if (!CSerializeUtils::LoadString(hFile, message))
			return false;

		m_changedPaths.clear();
		if (!CSerializeUtils::LoadNumber(hFile, value))
			return false;
		if (CSerializeUtils::SerializeType_Map == value)
		{
			if (CSerializeUtils::LoadNumber(hFile, value))
			{
				for (unsigned __int64 i=0; i<value; ++i)
				{
					wstring key;
					SVNLogChangedPaths cpaths;
					if (!CSerializeUtils::LoadString(hFile, key))
						return false;
					if (!cpaths.Load(hFile))
						return false;
					m_changedPaths[key] = cpaths;
				}
				return true;
			}
		}
		return false;
	}
};

class SVN
{
public:
	SVN(void);
	~SVN(void);

	void SetAuthInfo(const stdstring& username, const stdstring& password);

	bool Cat(stdstring sUrl, stdstring sFile);

	/**
	 * returns the info for the \a path.
	 * \param path a path or an url
	 * \param pegrev the peg revision to use
	 * \param revision the revision to get the info for
	 * \param recurse if TRUE, then GetNextFileInfo() returns the info also
	 * for all children of \a path.
	 */
	const SVNInfoData * GetFirstFileInfo(stdstring path, svn_revnum_t pegrev, svn_revnum_t revision, bool recurse = false);
	size_t GetFileCount() {return m_arInfo.size();}
	/**
	 * Returns the info of the next file in the file list. If no more files are in the list then NULL is returned.
	 * See GetFirstFileInfo() for details.
	 */
	const SVNInfoData * GetNextFileInfo();

	svn_revnum_t GetHEADRevision(const stdstring& url);

	bool GetLog(const stdstring& url, svn_revnum_t startrev, svn_revnum_t endrev);
	map<svn_revnum_t,SVNLogEntry> m_logs;		///< contains the gathered log information

	bool Diff(const wstring& url1, svn_revnum_t revision1, const wstring& path2, 
		svn_revnum_t revision2, bool ignoreancestry, bool nodiffdeleted, 
		bool ignorecontenttype,  const wstring& options, bool bAppend, 
		const wstring& outputfile, const wstring& errorfile);


private:
	apr_pool_t *				parentpool;		///< the main memory pool
	apr_pool_t *				pool;			///< 'root' memory pool
	svn_client_ctx_t * 			m_pctx;			///< pointer to client context
	svn_error_t *				Err;			///< Global error object struct
	svn_auth_baton_t *			auth_baton;

	vector<SVNInfoData>			m_arInfo;		///< contains all gathered info structs.
	unsigned int				m_pos;			///< the current position of the vector.


private:
	static svn_error_t *		cancel(void *baton);
	static svn_error_t *		infoReceiver(void* baton, const char * path, const svn_info_t* info, apr_pool_t * pool);
	static svn_error_t *		logReceiver(void* baton, apr_hash_t* ch_paths, svn_revnum_t rev, const char* author, const char* date, const char* msg, apr_pool_t* pool);

};


