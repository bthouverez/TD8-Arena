#include "resource_manager.hpp"



bool ResourceManager::m_init_done = false;
ResourceManager* ResourceManager::m_instance = nullptr;

bool ResourceManager::init()
{
  if (!m_init_done) {
    m_instance = new ResourceManager;
    m_init_done = true;
  }

  return true;
}

void ResourceManager::release()
{
  delete m_instance;
}