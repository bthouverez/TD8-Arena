#ifndef RESOURCE_MANAGER_HEADER
#define RESOURCE_MANAGER_HEADER



class ResourceManager
{
public:
   static bool init();
   static void release();

   static ResourceManager* getInstance(){ return m_instance; }

private:
   ResourceManager(){}

   static ResourceManager * m_instance;
   static bool m_init_done;
};
#endif