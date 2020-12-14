#include "usersdb.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <assert.h>

#include <boost/interprocess/sync/scoped_lock.hpp>

static const std::string s_db_file_name              = "userdb.txt";
static const std::string s_shared_memory_name        = "TestServerSharedMemoryName";
static const std::string s_shared_memory_map_name    = "TestServerSharedMap";
static const std::string s_shared_memory_named_mutex = "TestServerSharedNamedMutex";

std::vector<std::string> SplitString(const std::string& str, char delim)
{
    std::vector<std::string> result;
    std::string part_string;
    std::stringstream ss(str);
    while(std::getline(ss, part_string, delim))
        result.push_back(part_string);

    return result;
}

UsersDB::UsersDB(bool first_server)
    : m_first_server(first_server)
{
    if (m_first_server)
    {
        boost::interprocess::shared_memory_object::remove(s_shared_memory_name.c_str());
        boost::interprocess::named_mutex::remove(s_shared_memory_named_mutex.c_str());
    }
    try
    {
        if (m_first_server)
        {

            m_shared_memory = BoostSharedMemory(boost::interprocess::create_only, s_shared_memory_name.c_str(), 20*1024*1024);
            m_mutex_ptr     = SHMutexPtr(new SHMutex(boost::interprocess::create_only, s_shared_memory_named_mutex.c_str()));
        }
        else
        {
            m_shared_memory = BoostSharedMemory(boost::interprocess::open_only, s_shared_memory_name.c_str());
            m_mutex_ptr     = SHMutexPtr(new SHMutex(boost::interprocess::open_only, s_shared_memory_named_mutex.c_str()));
        }

        if (m_first_server)
        {
            m_alloc_ptr = ShMemAllocatorPtr( new ShMemAllocator(m_shared_memory.get_segment_manager()));
            m_users = m_shared_memory.construct<SharedMemoryMap>(s_shared_memory_map_name.c_str())(std::less<Login>(), *m_alloc_ptr);
        }
        else
            m_users = m_shared_memory.find<SharedMemoryMap>(s_shared_memory_map_name.c_str()).first;

        if (m_first_server)
            ReadFile();
    }
    catch (const std::exception& e) {
        std::cout<<" error  " << e.what() <<std::endl;
        if (m_first_server)
        {
            boost::interprocess::shared_memory_object::remove(s_shared_memory_name.c_str());
            boost::interprocess::named_mutex::remove(s_shared_memory_named_mutex.c_str());
        }
        exit(-1);
    }
}

void UsersDB::WriteFile() const
{
    std::ofstream outfile(s_db_file_name);
    for(auto login_password_it = m_users->begin(); login_password_it!=m_users->end();)
    {
        outfile << login_password_it->first << "," << login_password_it->second;
        ++login_password_it;
        if (login_password_it != m_users->end())
            outfile << ";";
    }
}

bool UsersDB::CheckUserPasswords(const Login &username, const Password &password) const
{
    boost::interprocess::scoped_lock<SHMutex> lock(*m_mutex_ptr);
    auto user_password_it = m_users->find(username);
     if (user_password_it == m_users->end())
         return false;

    return user_password_it->second == password;
}

bool UsersDB::AddUser(const Login &username, const Password &password)
{
    boost::interprocess::scoped_lock<SHMutex> lock(*m_mutex_ptr);
    if (UserExist(username))
        return false;

    m_users->emplace(username, password);
    WriteFile();
    return true;
}

bool UsersDB::DeleteUser(const Login &username)
{
    boost::interprocess::scoped_lock<SHMutex> lock(*m_mutex_ptr);
    if (!UserExist(username))
        return false;

    m_users->erase(username);
    WriteFile();
    return true;
}

bool UsersDB::UserExist(const Login &username) const
{
    return m_users->find(username) != m_users->end();
}

void UsersDB::ReadFile()
{
    std::ifstream file(s_db_file_name);

    std::string file_content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

    if (file_content.empty())
    {
        std::cout << "ERROR:No or empty file." << std::endl;
        exit(-1);
    }
    std::vector<std::string> user_password_strs = SplitString(file_content, ';');

    for(const auto& user_password_str : user_password_strs)
    {
        std::vector<std::string> user_password = SplitString(user_password_str, ',');
        if(user_password.size()!=2)
        {
            std::cout << "ERROR:Wrong file format." << std::endl;
            exit(-1);
        }
        m_users->emplace(user_password[0],user_password[1]);
    }
}
