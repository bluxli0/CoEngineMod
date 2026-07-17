#pragma once

#include <Geode/Geode.hpp>
#include <cocos2d.h>
#include <unordered_map>
#include <vector>

namespace CoEngine {

    class ObjectPool {
    private:
        // Maps Object IDs to a vector of pooled inactive GameObjects
        std::unordered_map<int, std::vector<geode::GameObject*>> m_pool;
        
        ObjectPool() = default;

    public:
        ~ObjectPool();

        // Singleton Accessor
        static ObjectPool* get();

        /**
         * Pre-warms the pool by instantiating a set block of objects.
         * Call this during level loading sequences to prevent mid-game stutters.
         */
        void warm(int objectID, size_t count);

        /**
         * Fetches an object from the pool. If none are free, it safely instantiates a new one.
         * The returned object is retained and ready to be configured and added to the LevelEditorLayer.
         */
        geode::GameObject* getObject(int objectID);

        /**
         * Recycles a GameObject back into the pool. 
         * Safely strips it from its parent node, resets volatile state variables, and retains it.
         */
        void recycleObject(geode::GameObject* gameObject);

        /**
         * Purges the entire allocated memory matrix, releasing all retained references.
         * Crucial to run on level exit to prevent severe memory leaks.
         */
        void clear();

        // Metrics
        size_t getPoolSize(int objectID) const;
    };
}
