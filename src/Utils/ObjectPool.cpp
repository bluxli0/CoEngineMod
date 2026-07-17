#include "ObjectPool.hpp"
#include <CoEngineCommon.hpp>

using namespace geode::prelude;

namespace CoEngine {

    ObjectPool::~ObjectPool() {
        clear();
    }

    ObjectPool* ObjectPool::get() {
        static ObjectPool instance;
        return &instance;
    }

    void ObjectPool::warm(int objectID, size_t count) {
        auto editor = LevelEditorLayer::get();
        if (!editor) {
            CO_LOG_WARN("ObjectPool: Cannot warm pool without an active LevelEditorLayer context.");
            return;
        }

        auto& objectList = m_pool[objectID];
        size_t currentSize = objectList.size();
        
        if (currentSize >= count) return;
        size_t needed = count - currentSize;

        CO_LOG_INFO("ObjectPool: Pre-warming Object ID %d with %zu instances.", objectID, needed);

        for (size_t i = 0; i < needed; ++i) {
            auto* obj = editor->createObject(objectID, {0, 0}, false);
            if (obj) {
                obj->removeFromParentAndCleanup(false);
                obj->retain(); // Keep alive in our vector
                objectList.push_back(obj);
            }
        }
    }

    GameObject* ObjectPool::getObject(int objectID) {
        auto& objectList = m_pool[objectID];

        if (!objectList.empty()) {
            auto* obj = objectList.back();
            objectList.pop_back();
            obj->autorelease(); // Hand off lifecycle control safely
            return obj;
        }

        auto editor = LevelEditorLayer::get();
        if (!editor) {
            CO_LOG_ERROR("ObjectPool: Critically missing LevelEditorLayer context during object allocation.");
            return nullptr;
        }

        auto* freshObj = editor->createObject(objectID, {0, 0}, false);
        if (freshObj) {
            freshObj->removeFromParentAndCleanup(false);
        }
        return freshObj;
    }

    void ObjectPool::recycleObject(GameObject* gameObject) {
        if (!gameObject) return;

        gameObject->removeFromParentAndCleanup(false);
        
        // Reset base transformations back to baseline
        gameObject->setPosition({0.0f, 0.0f});
        gameObject->setRotation(0.0f);
        gameObject->setScale(1.0f);
        gameObject->setOpacity(255);
        gameObject->setColor({255, 255, 255});
        
        // Clear native interaction flags
        gameObject->m_isEditorSelected = false;
        gameObject->m_hasGroupParent = false;
        gameObject->m_groups.clear();

        gameObject->retain();
        m_pool[gameObject->m_objectID].push_back(gameObject);
    }

    void ObjectPool::clear() {
        CO_LOG_INFO("ObjectPool: Purging active object recipe caches...");
        for (auto& [id, objectList] : m_pool) {
            for (auto* obj : objectList) {
                if (obj) {
                    obj->release(); // Prevent memory leaks on level exit
                }
            }
            objectList.clear();
        }
        m_pool.clear();
    }

    size_t ObjectPool::getPoolSize(int objectID) const {
        auto it = m_pool.find(objectID);
        if (it != m_pool.end()) {
            return it->second.size();
        }
        return 0;
    }
}
