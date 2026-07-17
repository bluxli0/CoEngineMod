#pragma once

#include <Geode/Geode.hpp>
#include <CoEngineCommon.hpp>
#include <vector>

namespace CoEngine {

    // Represents a single grouped action (e.g., an entire generation batch)
    struct EditTransaction {
        std::string transactionName;
        std::vector<geode::GameObject*> spawnedObjects;
    };

    class UndoRedoManager {
    private:
        std::vector<EditTransaction> m_undoStack;
        std::vector<EditTransaction> m_redoStack;
        
        EditTransaction* m_activeTransaction = nullptr;
        const size_t m_maxHistoryDepth = 25; // Prevents memory bloat from tracking thousands of deleted pointers

    public:
        UndoRedoManager() = default;
        ~UndoRedoManager() = default;

        // Singleton access matching Core architecture
        static UndoRedoManager* get();

        bool initialize();

        // Transaction Management APIs
        void beginTransaction(const std::string& name);
        void recordObjectCreation(geode::GameObject* gameObject);
        void endTransaction();

        // Execution Hooks (To be bound to UI hotkeys or Dashboard buttons)
        bool executeUndo(geode::LevelEditorLayer* editor);
        bool executeRedo(geode::LevelEditorLayer* editor);

        // History Utility
        void clearHistory();
        
        // Status checks for UI button state highlighting
        bool canUndo() const { return !m_undoStack.empty(); }
        bool canRedo() const { return !m_redoStack.empty(); }
        std::string getLatestUndoName() const;
    };
}
