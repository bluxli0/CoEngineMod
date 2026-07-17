#include "UndoRedoManager.hpp"

using namespace geode::prelude;

namespace CoEngine {

    UndoRedoManager* UndoRedoManager::get() {
        static UndoRedoManager instance;
        return &instance;
    }

    bool UndoRedoManager::initialize() {
        CO_LOG_INFO("Initializing Snapshot Transaction History (Undo/Redo) Layer...");
        clearHistory();
        return true;
    }

    void UndoRedoManager::clearHistory() {
        m_undoStack.clear();
        m_redoStack.clear();
        if (m_activeTransaction) {
            delete m_activeTransaction;
            m_activeTransaction = nullptr;
        }
        CO_LOG_INFO("Engine history buffer safely flushed.");
    }

    void UndoRedoManager::beginTransaction(const std::string& name) {
        // Safety lock: if a previous transaction wasn't closed, force-close it now
        if (m_activeTransaction) {
            endTransaction();
        }

        m_activeTransaction = new EditTransaction();
        m_activeTransaction->transactionName = name;
        
        // Wipe redo stack because a brand new user action breaks the forward redo chain
        m_redoStack.clear();
    }

    void UndoRedoManager::recordObjectCreation(geode::GameObject* gameObject) {
        if (!m_activeTransaction) {
            return; // Silently discard if no active tracking window is open
        }
        
        if (gameObject) {
            m_activeTransaction->spawnedObjects.push_back(gameObject);
        }
    }

    void UndoRedoManager::endTransaction() {
        if (!m_activeTransaction) return;

        // If the transaction ended up empty, discard it to save memory
        if (m_activeTransaction->spawnedObjects.empty()) {
            delete m_activeTransaction;
            m_activeTransaction = nullptr;
            return;
        }

        m_undoStack.push_back(*m_activeTransaction);
        
        // Enforce history depth constraints to keep memory footprints light
        if (m_undoStack.size() > m_maxHistoryDepth) {
            m_undoStack.erase(m_undoStack.begin());
        }

        CO_LOG_INFO("Transaction '%s' cached with %zu elements.", 
            m_activeTransaction->transactionName.c_str(), 
            m_activeTransaction->spawnedObjects.size());

        delete m_activeTransaction;
        m_activeTransaction = nullptr;
    }

    bool UndoRedoManager::executeUndo(geode::LevelEditorLayer* editor) {
        if (!editor || !canUndo()) {
            CO_LOG_WARN("Undo requested but history stack is empty or editor context is missing.");
            return false;
        }

        // Pull the latest transaction packet
        EditTransaction transaction = m_undoStack.back();
        m_undoStack.pop_back();

        CO_LOG_INFO("Reversing generation batch: %s", transaction.transactionName.c_str());

        // Process backwards to safely decouple interconnected dependencies
        for (auto it = transaction.spawnedObjects.rbegin(); it != transaction.spawnedObjects.rend(); ++it) {
            geode::GameObject* obj = *it;
            if (obj && editor->m_objects->containsObject(obj)) {
                // Safely extract the object from the editor's live hierarchy mapping context
                editor->removeObjectFromLevel(obj);
            }
        }

        // Push to the redo stack so the user can restore their generation if desired
        m_redoStack.push_back(transaction);
        
        // Force the editor interface to visually redraw the changes
        editor->updateEditorLayer();
        return true;
    }

    bool UndoRedoManager::executeRedo(geode::LevelEditorLayer* editor) {
        if (!editor || !canRedo()) {
            CO_LOG_WARN("Redo requested but forward history chain is empty.");
            return false;
        }

        EditTransaction transaction = m_redoStack.back();
        m_redoStack.pop_back();

        CO_LOG_INFO("Restoring generation batch: %s", transaction.transactionName.c_str());

        // Re-insert objects back into the active LevelEditorLayer layout context
        for (geode::GameObject* obj : transaction.spawnedObjects) {
            if (obj) {
                editor->addObjectToLevel(obj);
            }
        }

        m_undoStack.push_back(transaction);
        editor->updateEditorLayer();
        return true;
    }

    std::string UndoRedoManager::getLatestUndoName() const {
        if (m_undoStack.empty()) return "None";
        return m_undoStack.back().transactionName;
    }
}
