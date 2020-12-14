#ifndef USERSDB_H
#define USERSDB_H

#include<map>
#include<vector>

#define BOOST_DATE_TIME_NO_LIB // do not link to date_time lib
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>

std::vector<std::string> SplitString(const std::string& str, char delim);

using Login    = std::string;
using Password = std::string;
//using Users    = std::map<Login, Password>;

using BoostSharedMemory  = boost::interprocess::managed_shared_memory;
using LoginPasssword     = std::pair<const Login, Password>;
using ShMemAllocator     = boost::interprocess::allocator<LoginPasssword, boost::interprocess::managed_shared_memory::segment_manager>;
using ShMemAllocatorPtr  = std::unique_ptr<ShMemAllocator>;
using SharedMemoryMap    = boost::interprocess::map<Login, Password, std::less<Login>, ShMemAllocator>;
using SHMutex            = boost::interprocess::named_mutex;
using SHMutexPtr         = std::unique_ptr<SHMutex>;
using Users              = boost::interprocess::offset_ptr<SharedMemoryMap> ;

class UsersDB
{
public:
    UsersDB(bool first_server);

    void WriteFile() const;

    bool AddUser(const Login& username, const Password& password);
    bool DeleteUser(const Login& username);

    bool CheckUserPasswords(const Login& username, const Password& password) const;

private:

    void ReadFile();
    bool UserExist(const Login& username) const;

    BoostSharedMemory m_shared_memory;
    ShMemAllocatorPtr m_alloc_ptr;
    Users             m_users;
    bool              m_first_server;
    SHMutexPtr        m_mutex_ptr;
};

#endif // USERSDB_H
